@echo off
setlocal
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -property installationPath`) do set "VSPATH=%%i"
call "%VSPATH%\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 exit /b 1

if not exist "..\release" mkdir "..\release"

cl /nologo /EHsc /std:c++17 /utf-8 /DUNICODE /D_UNICODE /O2 /MT tts.cpp /Fe:"..\release\tts.exe" /link sapi.lib ole32.lib oleaut32.lib
if errorlevel 1 exit /b 1

echo Built ..\release\tts.exe
