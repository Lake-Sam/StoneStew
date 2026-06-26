# Testing Stone Stew Builds

Stone Stew currently targets Windows local tiles first. The repository is based
on DCSS 0.34.1.

## Build Once

The current machine needs a Windows C++ build toolchain before Codex can compile
the game here. Use the upstream-supported Visual Studio path:

1. Install Visual Studio 2022 or Visual Studio Build Tools with the C++ desktop
   workload.
2. Include the Visual Studio 2019 build tools, Windows Universal C Runtime, and
   Windows 8.1 SDK if the installer offers them.
3. Install Python 3, Perl, and PyYAML.
4. Open `crawl-ref/source/MSVC/crawl-ref.sln`.
5. Build `Release Tiles | x64`.

The expected executable is `crawl-ref/source/crawl.exe`.

## Test Commands

Open the Visual Studio solution:

```powershell
.\tools\stone-stew-dev.ps1 solution
```

After building, validate the data files and force the D:1 town through mapstat:

```powershell
.\tools\stone-stew-dev.ps1 check
```

Launch the local tiles game:

```powershell
.\tools\stone-stew-dev.ps1 launch
```

## Current Manual Test

Start a new normal game. The Stone Stew D:1 town has a temporary high
development weight, so it should usually be selected as the arrival vault.

Walk into a friendly named town NPC to talk. This should print fallback dialogue
and should not spend a turn.

The first NPCs to test are:

- Mara the Coinwise, merchant
- Old Rellan, quest-giver placeholder
- Bethra of the Cot, innkeeper
- Gate Warden, guard
- townsperson

The mapstat check uses `-force-map stone_stew_arrival_gate_town`, which validates
the vault even though `-force-map` is not used for normal interactive new games.
