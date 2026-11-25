import { Plugin } from "@opencode/plugin"
import { spawn, type ChildProcess } from "node:child_process"

const DEFAULT_EXE = "release\\tts.exe"

function stringOption(ctx: any, name: string, envName: string, fallback: string): string {
    const fromOptions = ctx.options?.[name]
    if (typeof fromOptions === "string" && fromOptions.length > 0) return fromOptions
    const fromEnv = process.env[envName]
    if (fromEnv && fromEnv.length > 0) return fromEnv
    return fallback
}

function numberOption(ctx: any, name: string, envName: string, fallback: number): number {
    const fromOptions = ctx.options?.[name]
    if (typeof fromOptions === "number" && Number.isFinite(fromOptions)) return fromOptions
    const fromEnv = process.env[envName]
    if (fromEnv && Number.isFinite(Number(fromEnv))) return Number(fromEnv)
    return fallback
}

function boolOption(ctx: any, name: string, envName: string, fallback: boolean): boolean {
    if (typeof ctx.options?.[name] === "boolean") return ctx.options[name]
    const fromEnv = process.env[envName]
    if (fromEnv === "1" || fromEnv === "true") return true
    if (fromEnv === "0" || fromEnv === "false") return false
    return fallback
}

function cleanForSpeech(raw: string): string {
    return raw
        .replace(/```[\s\S]*?```/g, " code block ")
        .replace(/`([^`]+)`/g, "$1")
        .replace(/\[([^\]]+)\]\([^)]+\)/g, "$1")
        .replace(/^[\s>#*_~]+/gm, "")
        .replace(/\s+/g, " ")
        .trim()
}

export default Plugin.define({
    id: "opencode.tts",
    setup(ctx) {
        const enabled = boolOption(ctx, "enabled", "OPENCODE_TTS_ENABLED", true)
        if (!enabled) return () => {}

        const exe = stringOption(ctx, "exe", "OPENCODE_TTS_EXE", DEFAULT_EXE)
        const voice = stringOption(ctx, "voice", "OPENCODE_TTS_VOICE", "Zira")
        const rate = numberOption(ctx, "rate", "OPENCODE_TTS_RATE", 1)
        const volume = numberOption(ctx, "volume", "OPENCODE_TTS_VOLUME", 100)
        const maxChars = numberOption(ctx, "maxChars", "OPENCODE_TTS_MAX_CHARS", 1200)
        const mode = stringOption(ctx, "mode", "OPENCODE_TTS_MODE", "interrupt")
        const debug = boolOption(ctx, "debug", "OPENCODE_TTS_DEBUG", false)
        const logPath = exe.replace(/tts\.exe$/i, "tts-invocations.log")

        const controller = new AbortController()
        let active: ChildProcess | null = null
        let lastSpokenID = ""
        let chain: Promise<void> = Promise.resolve()

        function speak(text: string): Promise<void> {
            if (active && !active.killed) {
                active.kill()
            }

            const args = [
                "--voice", voice,
                "--rate", String(rate),
                "--volume", String(volume),
            ]
            if (debug) {
                args.push("--log", logPath)
            }
            args.push(text)

            return new Promise<void>((resolve) => {
                let child: ChildProcess
                try {
                    child = spawn(exe, args, {
                        stdio: "ignore",
                        windowsHide: true,
                    })
                } catch (error) {
                    console.error("[opencode.tts] spawn error:", error)
                    resolve()
                    return
                }

                active = child
                child.on("error", (error) => {
                    console.error("[opencode.tts] failed to start TTS:", error)
                    if (active === child) active = null
                    resolve()
                })
                child.on("close", () => {
                    if (active === child) active = null
                    resolve()
                })
            })
        }

        void (async () => {
            try {
                for await (const event of ctx.event.subscribe({
                    types: ["session.execution.succeeded"],
                    signal: controller.signal,
                })) {
                    const sessionID = (event as any)?.data?.sessionID
                    if (typeof sessionID !== "string") continue

                    let messages: readonly any[]
                    try {
                        messages = await ctx.session.context({ sessionID })
                    } catch (error) {
                        console.error("[opencode.tts] failed to read session:", error)
                        continue
                    }

                    const lastTextMessage = [...messages]
                        .reverse()
                        .map((message) => {
                            if (message?.type !== "assistant") return null
                            const parts = (message.content ?? [])
                                .filter((part: any) => part?.type === "text")
                                .map((part: any) => String(part.text ?? ""))
                            const text = cleanForSpeech(parts.join(" "))
                            return text.length > 0 ? { id: String(message.id), text } : null
                        })
                        .find((entry) => entry !== null)

                    if (!lastTextMessage || lastTextMessage.id === lastSpokenID) continue

                    const text = lastTextMessage.text
                    lastSpokenID = lastTextMessage.id

                    const spoken = text.length > maxChars ? `${text.slice(0, maxChars)}…` : text

                    if (mode === "queue") {
                        chain = chain
                            .then(() => speak(spoken))
                            .catch((error) => console.error("[opencode.tts] queue error:", error))
                    } else {
                        void speak(spoken)
                    }
                }
            } catch (error) {
                if (!controller.signal.aborted) {
                    console.error("[opencode.tts] event stream error:", error)
                }
            }
        })()

        return () => {
            controller.abort()
            if (active && !active.killed) {
                active.kill()
            }
        }
    },
})
