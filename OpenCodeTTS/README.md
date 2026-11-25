# OpenCode TTS — source and development

Release package is in `..\release\`. Start there if you just want to try it.

## Source files

- `tts.cpp` — C++ helper source (v1.0.0)
- `tts.exe` — development build
- `build.bat` — development build
- `build-release.bat` — builds `..\release\tts.exe`
- `OpenCodeTTS.sln` / `OpenCodeTTS.vcxproj` — Visual Studio project
- `..\.opencode\plugins\tts\index.ts` — installed plugin
- `..\opencode.jsonc` — plugin configuration

## Speaker features

- Windows SAPI voices (David and Zira on this machine)
- Text from arguments, `--stdin`, or `--file`
- Voice selection by exact or partial name
- Rate/volume control
- `--list-voices`, `--check`, `--version`
- Optional `--log` file

## Build

```powershell
cmd /c build.bat
cmd /c build-release.bat
```

Or open `OpenCodeTTS.sln` in Visual Studio and build `Release|x64`.

## Plugin

The plugin listens for `session.execution.succeeded`, reads the last assistant
text message, cleans Markdown/code blocks, and speaks it through the release
speaker.

Options in `..\opencode.jsonc`:

| Option | Default | Meaning |
|---|---|---|
| `enabled` | `true` | Turn speech on/off |
| `exe` | `release\tts.exe` | Speaker path |
| `voice` | `Zira` | Voice name |
| `rate` | `1` | -10 to 10 |
| `volume` | `100` | 0 to 100 |
| `maxChars` | `1200` | Reply truncation |
| `mode` | `interrupt` | `interrupt` or `queue` |
| `debug` | `false` | Log spoken text |

Reload after edits:

```powershell
opencode api post /api/location/reload
```

## Notes

- Speech is local/offline.
- It speaks on this Windows PC, not on a remote Raspberry Pi.
- To speak on another machine, point `exe` at a wrapper that sends text to that
  machine's TTS service.
