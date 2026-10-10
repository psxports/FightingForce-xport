# Fighting Force

Resolve `[XPORT_ROOT]` from `xport-project.json` and read `[XPORT_ROOT]/AGENTS.md`. Keep shared methodology there; open project evidence only when the current task needs it.

## Identity and build

- Native C/VS2022 x86: `src/platform/win/FF.sln` -> `bin/FF.exe`; working directory `bin`; intermediates `_build`
- Original images: `SLUS_004.33`, `GAME.EXE`; default query image: `GAME.EXE`
- Immutable IDA exports: `orig/images`; profiles: `tools/ida`; reviewed status: `status/analysis.sqlite` and `status/translation-ledger.json`
- Disc data stays under `bin/DATA`; Red Book tracks stay under `bin/MUSIC`. Never use historical pre-extracted assets
- Native cold start uses reviewed initialized RAM generated into `src/game_data.c`; it never opens `GAME.EXE`
- Commands: `python -B ../xport/tools/xport.py --project . TOOL ...`

## Project constraints

- Compile the shared `[XPORT_ROOT]/src` runtime directly. Fighting Force is its first consumer, not a source for project-local copies
- Agreed WIP scope: memory card, physical CD/CD music and STR/MDEC. Game logic, sound and engine cutscenes remain real
- Keep ambiguous ledger mappings `GAME.EXE:8001559C`, `80015C6C`, `8002CDEC`, `8005748C` and `80058634` unmapped until each has one reviewed C identity
- Stage ticks reset across transitions; compare by monotonic phase ordinal and preserve stage-specific tables, gates and input-call order
- Native checkpoint ABI is `FightingForce native checkpoint ABI 1`; bump it after any serialized RAM/device/wrapper layout change
- GPU exclusions are limited to the configured audited FT4 padding and raw sprite opcode-65 RGB cases; see `status/todo-completion/raw-sprite-rgb-audit.md`

## Trace and converge

- Reserved GDB ports: audit `2348`, trace `2349`, user `2350`, migration `2400`; default automated role is migration
- Project profile: `tools/duckstation/xport-trace-profile.ini`; project adapter: `tools/ff_stage_adapter.py`
- Save-state slot names are hints only. Bind state hash, context, input sequence and overlay before reuse
- New recording/converge evidence must use the current shared workflow. Pre-2026-09-22 traces, checkpoints, anchors and comparisons are historical only
- Acceptance and optimization evidence lives in `status/toolset-validation` and `status/telemetry/converge`; do not copy measurements or completed-run history into this file
- Before C/H work use `X query 0xADDRESS --image IMAGE --audit-context`; after changes use `X code_refresh`
