# EU4DLL

This dll makes it possible to display double-byte characters on Europa Universalis IV.

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

> **Note:** If you remove the mod DLLs later, the game will still launch normally -- Wine falls back to its builtin `version.dll`.

## Find bugs

If you find a bug, **create an issue** in this repository.

## Spec

### plugins/plugin.ini

#### SEPARATE_CHARACTER_CODE_POINT

Change the character that connects the region name and the country name. See ISSUE-164.

#### REVERSING_WORDS_BATTLE_OF_AREA

Change the word ordering:

- Battle of / xxx -> xxx / Battle of
- Siege of / xxx -> xxx / Siege of
- Occupation of xxx -> xxx / Occupation of

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

Requires MinGW-w64 cross-compilation toolchain.

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
