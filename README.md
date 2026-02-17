# EU4DLL

[中文说明](README_zh-CN.md)

This dll makes it possible to display double-byte characters on Europa Universalis IV.

## Features about this fork

- **Auto UTF-8 conversion**: Localization files in UTF-8 are automatically converted to the game's internal encoding at load time. No need for pre-processing tools — just drop UTF-8 `.yml` files in and they work alongside traditional pre-encoded files.
- **Steam Rich Presence fix**: CJK text in Steam Rich Presence (e.g. country name, ruler title shown on your Steam profile) is properly displayed instead of garbled symbols.
- **Checksum override**: Force the game to display a specific checksum (e.g. `491d` for vanilla 1.37.5), allowing modded games to appear as unmodified for multiplayer compatibility.
- **Achievement unlock**: Bypass the game's checksum validation so achievements can be earned even with mods enabled.
- **Linux support**: Works under Proton/Wine via `version.dll` override.

## Notice

- This project is **unofficial**.
- Supports EU4 v1.37.x only.
- Windows and Linux (via Proton/Wine) are supported. Steam only.
- macOS is not supported.

## How to use

### Windows

1. Download the latest release zip and extract it.
2. Copy `version.dll` and the `plugins/` folder into the EU4 game directory.
3. Subscribe to multibyte mods from Steam Workshop, or add mods to the Paradox user directory.
4. Enable mods in the game launcher.
5. Play.

### Linux (Steam Play / Proton)

1. Follow the same steps as Windows to place the files.
2. In Steam, right-click EU4 > Properties > Launch Options, and add:

   ```bash
   WINEDLLOVERRIDES="version=n,b" %command%
   ```

   This tells Wine/Proton to load the native `version.dll` from the game directory instead of its builtin one.
3. Play.

> **Note:** If you remove the mod DLLs later, the game will still launch normally — Wine falls back to its builtin `version.dll`.

## Find bugs

If you find a bug, **create an issue** in this repository.

## Spec

### plugins/plugin.ini

#### SEPARATE_CHARACTER_CODE_POINT

Change the character that connects the region name and the country name. See [ISSUE-164](https://github.com/matanki-saito/EU4dll/issues/164).

#### REVERSING_WORDS_BATTLE_OF_AREA

Change the word ordering:

- Battle of / xxx -> xxx / Battle of
- Siege of / xxx -> xxx / Siege of
- Occupation of xxx -> xxx / Occupation of

#### AUTO_UTF8_CONVERSION

When set to `yes` (default), UTF-8 encoded `.yml` and `.txt` files from mods are automatically converted to the game's internal escape encoding at load time. Vanilla game files are not affected.

#### STEAM_RICH_PRESENCE

When set to `yes` (default), fixes garbled CJK text in Steam Rich Presence display (country name, ruler title, etc. shown on your Steam profile).

#### CHECKSUM_OVERRIDE

Set to a 4-character hex string (e.g. `491d`) to force the game to display that checksum instead of the computed one. This works by intercepting `checksum_manifest.txt` and patching the checksum string in memory. Leave empty to disable.

#### ACHIEVEMENT_UNLOCK

When set to `yes`, patches the game's internal checksum validation so that achievements can be earned regardless of whether mods are active. Default is `no`.

### Name order

Attaching Inverted Question Mark (¿) to dynasty, the first name and last name are reversed.

```paradox
1534.6.23 = {
  heir = {
    name = "Nobunaga"  # Nobunaga
    monarch_name = "Nobunaga"  # Nobunaga
    dynasty = "¿Oda"  # Oda
    birth_date = 1534.6.23
    death_date = 1582.6.21
    claim = 90
    adm = 5
    dip = 5
    mil = 6
  }
}
# Nobunaga Oda -> Oda Nobunaga
```

When you use this feature, please exclude Inverted Question Mark (¿) from all fonts.

## Build

Requires a MinGW-w64 toolchain (GCC with C++20 support).

### Windows (native)

Use MSYS2 or standalone MinGW-w64:

```bash
cmake -B build -S src -G "MinGW Makefiles"
cmake --build build
cmake --build build --target package
```

### Linux / macOS (cross-compilation)

```bash
cmake -B build -S src -DCMAKE_TOOLCHAIN_FILE=src/toolchain/mingw-w64-x86_64.cmake
cmake --build build
cmake --build build --target package
```

Output is in `build/package/`.

## Licence

MIT Licence

## Thanks

This dll was forked from the following project. Thank you so much.

[EU4CHS](https://bitbucket.org/kelashi/eu4chs/src/master/)

[EU4JPS](https://github.com/matanki-saito/EU4dll/tree/develop)
