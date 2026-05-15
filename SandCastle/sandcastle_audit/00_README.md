# SandCastle audit — fix prompts

Each numbered file is a self-contained prompt describing one engine bug to fix.
Lower number = higher priority. Files were generated from a deep audit of the
madman + SandCastle codebase looking for the cause of sporadic crashes at high
entity counts (15k+ horde units).

**Engine location:** `C:/dev/SandCastle/SandCastle/`
**Game location:** `C:/dev/madman/`

## Priority tiers

- **01–06** — Tier 1: directly correlated with the load-related crash symptom.
  Likeliest culprits. Fix first.
- **07–12** — Tier 2: real bugs with smaller crash windows or that require a
  trigger (asset reload, runtime layer swap, etc.).
- **13–17** — Tier 3: latent / lower impact / small bugs.

## How to use one of these as a prompt

Paste the contents of the chosen file into a fresh Claude Code session running
inside `C:/dev/SandCastle/SandCastle/`. Each file is structured to be
self-contained: it cites verified file:line locations, explains the bug,
suggests a fix, and tells the agent what to verify before editing.

## Game-side issues NOT covered here

These were found in `C:/dev/madman/src/` and are handled separately:
- `SpawnerSys` priority race — FIXED in `madman/src/InitSystems.cpp` (push order)
- `DevSys::OnImGui` mutates ECS from the render thread
- `HordeSys::MoveUpdt` NaN check has wrong operator precedence (line 450)
- Various `static auto x = sys(Other);` caches that outlive Systems::Kill

## Audit methodology

Findings were produced by 6 parallel deep-dive agents (renderer threading,
worker/signal primitives, ECS, asset lifetime, UI/physics/input, madman game
systems). The top critical claims were then verified by direct source reads
before being written up. Each file notes whether the cited lines were verified
personally or are agent-reported (mark them as "verify first" in that case).
