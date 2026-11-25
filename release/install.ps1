$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$pluginDir = Join-Path $root ".opencode\plugins\tts"
$configPath = Join-Path $root "opencode.jsonc"
$snippetPath = Join-Path $PSScriptRoot "config.snippet.jsonc"
$exePath = Join-Path $PSScriptRoot "tts.exe"

if (-not (Test-Path $exePath)) {
    throw "Missing tts.exe in $PSScriptRoot"
}

New-Item -ItemType Directory -Force -Path $pluginDir | Out-Null
Copy-Item -Path (Join-Path $PSScriptRoot "plugin\index.ts") -Destination (Join-Path $pluginDir "index.ts") -Force

if (-not (Test-Path $configPath)) {
    Copy-Item -Path $snippetPath -Destination $configPath
    Write-Host "Created $configPath"
} else {
    $existing = Get-Content -Path $configPath -Raw
    if ($existing -match "opencode\.tts") {
        Write-Host "OpenCode TTS is already configured in $configPath"
    } else {
        Write-Host "opencode.jsonc already exists. Merge the contents of:"
        Write-Host "  $snippetPath"
        Write-Host "into your plugins array to finish setup."
    }
}

Write-Host "Plugin installed to $pluginDir"
Write-Host "Speaker: $exePath"

opencode api post /api/location/reload | Out-Null
Write-Host "OpenCode reloaded."
