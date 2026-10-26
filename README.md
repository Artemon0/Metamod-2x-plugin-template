# CS2 plugin template

This repository is a starting point for a Counter-Strike 2 Metamod plugin. The build remains tailored to the CS2 HL2SDK, Metamod:Source, and funchook.

## Set your plugin details

Run this from PowerShell in the repository:

```powershell
.\setup-template.ps1
```

The script creates `plugin-metadata.json`. It asks for these values:

- `name` — lowercase plugin identifier; used for the CMake target, binary, package directory, and default gamedata/config filenames. Use only lowercase letters, numbers, `_`, and `-`.
- `alias` — Metamod plugin alias; use only letters, numbers, `_`, and `-`.
- `display_name` — human-readable name shown by the plugin.
- `description` — short plugin description.
- `author` — your name or organization.
- `url` — project or support URL; optional.
- `log_tag` — tag used in plugin log messages.
- `license` — your project's license identifier, such as `MIT` or `GPL-3.0-only`.
- `version` — semantic version in `major.minor.patch` format, for example `1.0.0`.

You can also pass values as script parameters. Use `-Force` only when you intend to replace an existing `plugin-metadata.json`:

```powershell
.\setup-template.ps1 -Name "my_plugin" -Alias "my_plugin" -DisplayName "My Plugin" `
    -Description "What the plugin does" -Author "Your Name" -Url "https://example.com" `
    -LogTag "MYPLUGIN" -License "MIT" -Version "1.0.0"
```

## Add plugin files

- Put C and C++ source files in `src`.
- Optional gamedata and config files are copied when named `gamedata/<name>.games.txt` and `cfg/<name>.cfg`.
- Optional translation text files go in `translations/*.txt`.
- Put your license in `LICENSE`; additional third-party license text files can go in `licenses/*.txt`.
- `THIRD_PARTY_NOTICES.md` is copied into the package if present.

The project expects the `hl2sdk-cs2`, `metamod-source`, and `vendor/funchook` directories at the repository root. Initialize the Git submodules and provide funchook before configuring CMake. Builds target 64-bit Windows with MSVC or Linux with Clang.

The distributable package is generated under `build/package` after building the plugin.
