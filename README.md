# CS2 Metamod plugin template

This repository is a small, buildable starting point for Counter-Strike 2 Metamod:Source plugins. It takes the reusable project structure and build ideas from CS2AC-CE, but does not copy its anti-cheat logic, signatures, detection modules, or third-party hooks.

## Set up plugin metadata

Run this in PowerShell from the repository root:

```powershell
.\setup-template.ps1
```

The script writes `plugin-metadata.json`. Provide your own values for:

- `name`: lowercase identifier used for the CMake target, binary, package directory, and generated config filename. Allowed characters: lowercase letters, numbers, `_`, `-`.
- `alias`: Metamod alias. Allowed characters: letters, numbers, `_`, `-`.
- `display_name`: name shown in Metamod.
- `description`: short plugin description.
- `author`: your name or organization.
- `url`: optional project or support URL. Pass it with `-Url` when running the script.
- `log_tag`: short prefix used in server console output.
- `license`: your project's license identifier.
- `version`: version in `major.minor.patch` format.

For a non-interactive setup, pass the values as parameters:

```powershell
.\setup-template.ps1 -Name "my_plugin" -Alias "my_plugin" -DisplayName "My Plugin" `
    -Description "What the plugin does" -Author "Your Name" -Url "https://example.com" `
    -LogTag "MYPLUGIN" -License "MIT" -Version "1.0.0"
```

The script will not replace an existing metadata file unless `-Force` is specified.

## Build

Initialize the SDK submodules, then configure and build:

```powershell
git submodule update --init --recursive
cmake -S . -B build -A x64
cmake --build build --config Release
```

Requirements are CMake, 64-bit MSVC on Windows or Clang on Linux, the CS2 HL2SDK, and Metamod:Source. The HL2SDK protobuf messages are generated as part of the build. Funchook is not required by the starter plugin.

The packaged plugin is written to `build/package`. It contains the binary and a Metamod VDF file; the template config is copied as `game/csgo/cfg/<name>.cfg`. Optional files are copied when present:

- `gamedata/<name>.games.txt`
- `translations/*.txt`
- `licenses/*.txt`
- `THIRD_PARTY_NOTICES.md`

Edit `cfg/plugin.cfg` for the starter convar. The starter command is `template_status`; rename its command and the `template_enabled` convar in `src/template.cpp` and `cfg/plugin.cfg` to use your own prefix.

## Starter features

The plugin entry point in `src/template.cpp` provides Metamod load, pause, unpause, unload, and running-state callbacks. It also installs removable Source 2 hooks for map frames and player lifecycle callbacks:

- server frame
- client entering the server
- client becoming active and fully connected
- client disconnecting
- client commands and client settings changes

`Template::PlayerInfo` stores a player's slot, name, Steam ID, and connection state. Use `Template::FindPlayer` to access a connected player's record. `template_enabled` and `template_status` show how to add a convar and server command.

Put additional C++ feature modules under `src/features/`; CMake includes their `.cpp` and `.c` files automatically. Keep reusable systems in separate headers and source files there. This lets a small command plugin stay small while larger game modes can add player state, round systems, persistence, economy, entities, and gameplay modules as needed.

Only `src/template.cpp` and `src/features/` are compiled. The existing `src/player/` and `src/utils/` files are remnants of the earlier CS2AC-based draft and are intentionally excluded until they are adapted to the generic interfaces.

The CS2AC-CE reference uses version-sensitive gamedata and event hooks for its anti-cheat features. Those are intentionally not enabled in this generic starter; add only the signatures and hooks your plugin actually needs, and keep their cleanup paired with plugin pause/unload.
