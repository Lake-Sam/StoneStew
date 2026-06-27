# Stone Stew LLM Packaging

Stone Stew can package a local llama.cpp runtime and a Qwen3 GGUF model with
the Windows build. This is intended for release builds where players should not
need to install Ollama separately.

## Current bundle

- Runtime: llama.cpp Windows x64 CPU release
- Model: `ggml-org/Qwen3-1.7B-GGUF`, `Qwen3-1.7B-Q4_K_M.gguf`
- App-local runtime DLLs: `ThinkGeo.Dependency.MicrosoftVisualCRuntime140`
- Local endpoint: `http://127.0.0.1:8080/v1/chat/completions`

The CPU runtime is the default because it is the most broadly compatible
Windows target. GPU/Vulkan bundles can be added later after the basic packaged
flow is stable.

## Build the local bundle

Run:

```powershell
.\tools\download-stone-stew-llm.ps1
```

This creates `third_party/stone-stew-llm`, downloads llama.cpp, downloads the
GGUF model, and writes a small asset README. The folder is intentionally ignored
by git because it contains large binary release assets.

## Package behavior

`tools/package-stone-stew.ps1` automatically includes the bundle when both of
these files exist:

- `third_party/stone-stew-llm/llama/llama-server.exe`
- `third_party/stone-stew-llm/models/Qwen3-1.7B-Q4_K_M.gguf`

Packaged builds include `Start Stone Stew LLM.ps1`. The normal
`Start Stone Stew.bat` runs that helper before launching `crawl.exe`.
When the LLM bundle is present, startup may pause while llama.cpp loads the
model. On CPU-only machines, the first generated line can still take noticeable
time.

## Runtime behavior

The game asks the bundled llama.cpp endpoint for NPC flavor first. If no local
llama.cpp response is available, it falls back to the development Ollama
endpoint at `http://127.0.0.1:11434/api/generate`. If neither responds, gameplay
continues with the existing handwritten fallback notice.
