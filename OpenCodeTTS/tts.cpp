#include <windows.h>
#include <sapi.h>
#include <wrl/client.h>

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cwctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;

#pragma comment(lib, "sapi.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

namespace {

constexpr wchar_t VERSION[] = L"1.0.0";

enum ExitCode : int {
    ExitOk = 0,
    ExitUsage = 1,
    ExitCom = 2,
    ExitNoVoice = 3,
    ExitSpeak = 4,
    ExitInput = 5,
};

struct VoiceEntry {
    std::wstring name;
    ComPtr<ISpObjectToken> token;
};

std::wstring ToLower(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(), [](wchar_t c) {
        return static_cast<wchar_t>(::towlower(c));
    });
    return s;
}

std::wstring Utf8ToWide(const std::string& data) {
    if (data.empty()) return {};

    int length = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        data.data(),
        static_cast<int>(data.size()),
        nullptr,
        0
    );
    if (length <= 0) return {};

    std::wstring result(static_cast<size_t>(length), L'\0');
    MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        data.data(),
        static_cast<int>(data.size()),
        result.data(),
        length
    );
    return result;
}

std::string ReadFileUtf8(const std::wstring& path) {
    std::ifstream input(path.c_str(), std::ios::binary);
    if (!input) return {};

    std::string data(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>()
    );
    if (data.size() >= 3 &&
        static_cast<unsigned char>(data[0]) == 0xEF &&
        static_cast<unsigned char>(data[1]) == 0xBB &&
        static_cast<unsigned char>(data[2]) == 0xBF) {
        data.erase(0, 3);
    }
    return data;
}

void WriteLog(const std::wstring& path, const std::wstring& text) {
    if (path.empty()) return;

    std::wofstream output(path.c_str(), std::ios::app);
    if (!output) return;

    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    output << std::put_time(std::localtime(&time), L"%Y-%m-%dT%H-%M-%S")
           << L" text=" << text << L'\n';
}

void PrintUsage() {
    std::wcout << LR"(
OpenCode TTS helper v1.0.0 (Windows SAPI)

Usage:
  tts.exe [options] "text to speak"
  tts.exe --stdin < input.txt
  tts.exe --file reply.txt
  tts.exe --list-voices
  tts.exe --check

Options:
  --voice <name>    Select voice by exact or partial name (case-insensitive)
  --rate <n>        Speaking rate, -10 (slow) to 10 (fast), default 0
  --volume <n>      Volume, 0 to 100, default 100
  --stdin           Read UTF-8 text from standard input
  --file <path>     Read UTF-8 text from a file
  --log <path>      Append spoken text to a log file
  --list-voices     Print installed SAPI voices
  --check           Verify COM/SAPI and print voice count
  -v, --version     Print version
  -h, --help        Show this help

Examples:
  tts.exe "Build finished"
  tts.exe --voice Zira --rate 2 --volume 80 "All tests passed"
  type reply.txt | tts.exe --stdin
  tts.exe --file reply.txt --log C:\temp\tts.log
)";
}

bool GetVoices(std::vector<VoiceEntry>& voices) {
    ComPtr<ISpObjectTokenCategory> category;
    HRESULT hr = CoCreateInstance(
        CLSID_SpObjectTokenCategory,
        nullptr,
        CLSCTX_ALL,
        IID_PPV_ARGS(&category)
    );
    if (FAILED(hr)) {
        std::wcerr << L"Failed to create voice category: 0x" << std::hex << hr << std::dec << L'\n';
        return false;
    }

    hr = category->SetId(SPCAT_VOICES, FALSE);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to open voice category: 0x" << std::hex << hr << std::dec << L'\n';
        return false;
    }

    ComPtr<IEnumSpObjectTokens> enumerator;
    hr = category->EnumTokens(nullptr, nullptr, &enumerator);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to enumerate voices: 0x" << std::hex << hr << std::dec << L'\n';
        return false;
    }

    ComPtr<ISpObjectToken> token;
    ULONG fetched = 0;
    while (SUCCEEDED(enumerator->Next(1, token.ReleaseAndGetAddressOf(), &fetched)) && fetched == 1) {
        LPWSTR name = nullptr;
        if (SUCCEEDED(token->GetStringValue(nullptr, &name)) && name != nullptr) {
            voices.push_back({ name, token });
            CoTaskMemFree(name);
        }
    }
    return true;
}

bool SelectVoice(const std::vector<VoiceEntry>& voices, const std::wstring& requested, ComPtr<ISpObjectToken>& selected) {
    const std::wstring lowerRequested = ToLower(requested);

    for (const auto& entry : voices) {
        if (ToLower(entry.name) == lowerRequested) {
            selected = entry.token;
            return true;
        }
    }
    for (const auto& entry : voices) {
        if (ToLower(entry.name).find(lowerRequested) != std::wstring::npos) {
            selected = entry.token;
            return true;
        }
    }
    return false;
}

} // namespace

int wmain(int argc, wchar_t** argv) {
    bool listVoices = false;
    bool checkOnly = false;
    bool fromStdin = false;
    std::wstring voiceName;
    std::wstring filePath;
    std::wstring logPath;
    std::wstring text;
    long rate = 0;
    long volume = 100;

    for (int i = 1; i < argc; ++i) {
        const std::wstring arg = argv[i];

        if (arg == L"-h" || arg == L"--help") {
            PrintUsage();
            return ExitOk;
        }
        if (arg == L"-v" || arg == L"--version") {
            std::wcout << VERSION << L'\n';
            return ExitOk;
        }
        if (arg == L"--list-voices") {
            listVoices = true;
        } else if (arg == L"--check") {
            checkOnly = true;
        } else if (arg == L"--stdin") {
            fromStdin = true;
        } else if (arg == L"--voice" && i + 1 < argc) {
            voiceName = argv[++i];
        } else if (arg == L"--rate" && i + 1 < argc) {
            rate = _wtoi(argv[++i]);
        } else if (arg == L"--volume" && i + 1 < argc) {
            volume = _wtoi(argv[++i]);
        } else if (arg == L"--file" && i + 1 < argc) {
            filePath = argv[++i];
        } else if (arg == L"--log" && i + 1 < argc) {
            logPath = argv[++i];
        } else if (arg.rfind(L"--", 0) == 0) {
            std::wcerr << L"Unknown option: " << arg << L'\n';
            PrintUsage();
            return ExitUsage;
        } else {
            if (!text.empty()) text += L' ';
            text += arg;
        }
    }

    if (!filePath.empty()) {
        const std::wstring fromFile = Utf8ToWide(ReadFileUtf8(filePath));
        if (!text.empty() && !fromFile.empty()) text += L' ';
        text += fromFile;
    }

    if (fromStdin) {
        const std::string input(
            (std::istreambuf_iterator<char>(std::cin)),
            std::istreambuf_iterator<char>()
        );
        const std::wstring converted = Utf8ToWide(input);
        if (!text.empty() && !converted.empty()) text += L' ';
        text += converted;
    }

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        std::wcerr << L"COM initialization failed: 0x" << std::hex << hr << std::dec << L'\n';
        return ExitCom;
    }

    int result = ExitOk;

    // Keep every COM pointer inside this scope so it is released before CoUninitialize.
    {
        std::vector<VoiceEntry> voices;
        if (!GetVoices(voices)) {
            result = ExitCom;
        } else if (listVoices) {
            for (const auto& entry : voices) {
                std::wcout << entry.name << L'\n';
            }
        } else if (checkOnly) {
            std::wcout << L"OK voices=" << voices.size() << L'\n';
            if (voices.empty()) result = ExitNoVoice;
        } else if (text.empty()) {
            PrintUsage();
            result = ExitUsage;
        } else {
            rate = (std::max)(-10L, (std::min)(10L, rate));
            volume = (std::max)(0L, (std::min)(100L, volume));

            ComPtr<ISpVoice> voice;
            hr = CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&voice));
            if (FAILED(hr)) {
                std::wcerr << L"Failed to create SAPI voice: 0x" << std::hex << hr << std::dec << L'\n';
                result = ExitCom;
            } else {
                if (!voiceName.empty()) {
                    ComPtr<ISpObjectToken> selected;
                    if (!SelectVoice(voices, voiceName, selected) || !selected) {
                        std::wcerr << L"No voice matched: " << voiceName << L'\n';
                        std::wcerr << L"Run tts.exe --list-voices to see available voices.\n";
                        result = ExitNoVoice;
                    } else {
                        hr = voice->SetVoice(selected.Get());
                        if (FAILED(hr)) {
                            std::wcerr << L"Failed to select voice: 0x" << std::hex << hr << std::dec << L'\n';
                            result = ExitNoVoice;
                        }
                    }
                }

                if (result == ExitOk) {
                    voice->SetRate(rate);
                    voice->SetVolume(static_cast<WORD>(volume));

                    hr = voice->Speak(text.c_str(), SPF_DEFAULT | SPF_PURGEBEFORESPEAK, nullptr);
                    if (FAILED(hr)) {
                        std::wcerr << L"Speech failed: 0x" << std::hex << hr << std::dec << L'\n';
                        result = ExitSpeak;
                    } else {
                        WriteLog(logPath, text);
                    }
                }
            }
        }
    }

    CoUninitialize();
    return result;
}
