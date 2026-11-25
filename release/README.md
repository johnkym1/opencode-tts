# OpenCode TTS — Release 1.0.0

Text-to-speech for OpenCode replies. The C++ helper uses Windows SAPI voices,
so it speaks on this Windows PC. A local OpenCode plugin speaks the final
assistant reply after each turn.

## Try it

1. Run `install.ps1` in this folder.
2. Ask OpenCode anything, for example:
   ```powershell
   opencode run "Reply with exactly: OpenCode TTS release is working"
   ```
3. You should hear the reply spoken with the Microsoft Zira voice.

## Contents

- `tts.exe` — release speaker, built with MSVC `/O2 /MT`
- `plugin/index.ts` — OpenCode plugin source
- `config.snippet.jsonc` — exact config used by this release
- `install.ps1` — installs plugin and config
- `VERSION` — release version

## Speaker options

```powershell
.\tts.exe --check
.\tts.exe --list-voices
.\tts.exe "Hello"
.\tts.exe --voice David --rate 0 --volume 80 "Slightly slower"
.\tts.exe --file reply.txt --log C:\temp\tts.log
type reply.txt | .\tts.exe --stdin
```

## Plugin options

Edit `opencode.jsonc` in the project root:

| Option | Default | Meaning |
|---|---|---|
| `enabled` | `true` | Turn speech on/off |
| `exe` | `release\tts.exe` | Path to speaker |
| `voice` | `Zira` | Exact or partial voice name |
| `rate` | `1` | -10 to 10 |
| `volume` | `100` | 0 to 100 |
| `maxChars` | `1200` | Reply truncation limit |
| `mode` | `interrupt` | `interrupt` or `queue` |
| `debug` | `false` | Write spoken text to `release\tts-invocations.log` |

Environment overrides also work: `OPENCODE_TTS_ENABLED`, `OPENCODE_TTS_EXE`,
`OPENCODE_TTS_VOICE`, `OPENCODE_TTS_RATE`, `OPENCODE_TTS_VOLUME`,
`OPENCODE_TTS_MAX_CHARS`, `OPENCODE_TTS_MODE`, `OPENCODE_TTS_DEBUG`.

## Troubleshooting

- Confirm plugin is active: `opencode plugin list`
- Confirm speaker is healthy: `.\tts.exe --check`
- Reload after config edits: `opencode api post /api/location/reload`
- If no sound, check Windows volume and the default output device.

## Source

Buildable Visual Studio project and development scripts live in
`..\OpenCodeTTS\`.
