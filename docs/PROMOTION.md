# Promotion drafts

Ready-to-use posts for sharing `opencode-tts`. Tailor slightly per site, and ask for a star only once at the end.

**Repo:** https://github.com/johnkym1/opencode-tts  
**Release:** https://github.com/johnkym1/opencode-tts/releases/tag/v1.0.0  
**Ecosystem PR:** https://github.com/anomalyco/opencode/pull/66270

## Hacker News — Show HN

**Title:** Show HN: OpenCode TTS – local replies read aloud on Windows

**Body:**

> I built a small local plugin that makes OpenCode speak assistant replies aloud on Windows. It uses built-in SAPI voices, with optional natural Narrator voices such as Aria, and needs no API key or cloud service.
>
> It scans backward from the final message and speaks the latest assistant message that actually contains text, so tool-only final turns don't produce silence.
>
> Install is two steps: copy the `release` folder into your OpenCode project and run `release/install.ps1`, then reload. Windows-only for now because it uses SAPI.
>
> Repo: https://github.com/johnkym1/opencode-tts
> Release: https://github.com/johnkym1/opencode-tts/releases/tag/v1.0.0
>
> Feedback on voice selection, install flow, or edge cases is welcome. If you find it useful, a star helps others discover it.

## Reddit — r/OpenCode / r/coding / r/accessibility

**Title:** I made OpenCode speak replies aloud on Windows (free, local, no API key)

**Body:**

> I wanted OpenCode to read its replies out loud, so I made a small local plugin for Windows.
>
> - Uses Windows SAPI voices; optional natural Narrator voices like Aria
> - No cloud service, account, or API key
> - Speaks the latest assistant message that contains text, even if the final message is tool-only
> - MIT licensed, single local plugin + small `tts.exe`
>
> Install: copy `release` into your OpenCode project and run `./release/install.ps1`, then reload OpenCode.
>
> Repo: https://github.com/johnkym1/opencode-tts
>
> Happy to answer questions. If it saves you setup time, a star would help others find it.

## X / Mastodon

**Post 1:**

> Built a free local plugin that makes OpenCode speak replies aloud on Windows. SAPI voices, optional natural Narrator voices, no API key.  
> https://github.com/johnkym1/opencode-tts

**Post 2 (reply/thread):**

> Install: copy `release` into your OpenCode project, run `./release/install.ps1`, reload. Set `voice: "Aria"` in `opencode.jsonc` for a natural voice. Feedback welcome — star it if useful.

## DEV Community / Hashnode

**Title:** Giving OpenCode a Voice: A Local Windows TTS Plugin

**Outline:**

1. Problem: replies are silent; final messages can be tool-only.
2. Solution: local SAPI speaker + OpenCode V2 plugin that scans backward for the last text-bearing assistant message.
3. Install: `release/install.ps1`, config snippet, reload.
4. Voice options: built-in SAPI vs. natural Narrator voices such as Aria.
5. Try it: `opencode run "Reply with exactly: OpenCode TTS is working"`.
6. Links: repo, release, ecosystem PR.
7. Close with a polite request for feedback and a star.

## Short DM / comment

> Hey — I made a free local Windows plugin that speaks OpenCode replies aloud using SAPI/Narrator voices, no API key. Thought it might be useful: https://github.com/johnkym1/opencode-tts . If you try it and like it, a star would help discovery. Feedback welcome.
