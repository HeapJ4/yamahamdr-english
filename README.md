# MidRadio English

**Yamaha MidRadio Player, with an English interface.**

[![Tests](https://github.com/HeapJ4/yamahamdr-english/actions/workflows/check.yml/badge.svg)](https://github.com/HeapJ4/yamahamdr-english/actions/workflows/check.yml)
[![License: MIT](https://img.shields.io/badge/patch_code-MIT-blue.svg)](LICENSE)
![Target: Windows 10 x64](https://img.shields.io/badge/target-Windows_10_x64-0078D6)
![Status: Preview](https://img.shields.io/badge/status-preview-orange)

An unofficial English localization of **MidRadio Player 7.2.1.1**. This preview translates **756 text fields across 10 files**, including menus, settings, dialogs, tooltips, and error messages.

## Get the portable version

Open [Releases](https://github.com/HeapJ4/yamahamdr-english/releases) and download **`yamahamdr-english-v0.1.0-preview-windows-x64.zip`** when available.

1. Close any running copy of MidRadio Player.
2. Right-click the ZIP and choose **Extract All**. Keep every file and the `Addins` folder together.
3. Open the extracted folder and launch **`MidRadio-English.exe`**.
4. Use **File → Open File** to open a local music file.

No Python, installer, administrator access, or replacement of the installed player is needed to launch the portable package. The original application may still store its settings in the Windows registry: “portable” here means an extracted application folder, not guaranteed zero traces.

**Primary target:** Windows 10 x64. **User-confirmed working:** Windows 11 x64. The original player remains an **x86 application** and runs on x64 Windows through WoW64; this project does not convert its audio engine to native x64. A clean Windows 10 installation has not yet been independently tested.

## What is translated?

| Component | Translated text fields |
| --- | ---: |
| Main player | 419 |
| Synthesizer modules | 170 |
| Chord/songbook add-in | 54 |
| Karaoke add-in | 49 |
| ActiveX control | 31 |
| SGP2 synthesizer setup | 16 |
| Slideshow | 12 |
| MIDI format and output add-ins | 5 |
| **Total** | **756** |

The patch changes Win32 text resources. It preserves resource identifiers, dialog control structure, format placeholders, keyboard-shortcut suffixes, font names, and non-resource PE section bytes.

### Preview limitations

- Some Japanese text is drawn into skin bitmaps, including **Select Screen**, **Track Details**, and the **repeat-mode labels**. Those graphics remain Japanese in this release.
- Music titles, lyrics, third-party skins, and online content are not translated.
- UI startup, translated menus, and the MIDI settings dialog were observed on the development machine. Audio playback and every dialog have not been systematically tested.
- The project does not repair the player's legacy online services or establish that they still work.
- The portable build was tried on a machine with MidRadio already installed. First-run dependency behavior on a clean PC still needs testing.

## Build it yourself

Use **Windows** and **Python 3.9 or newer**, plus an original MidRadio Player 7.2.1.1 installation. No pip dependencies are required.

From the extracted source folder:

```powershell
py -3 patch.py --dry-run
py -3 patch.py --output .\MidRadio-English
```

For a different installation location:

```powershell
py -3 patch.py --source "D:\Apps\MidRadio Player" --output "D:\Apps\MidRadio-English"
```

The patcher checks the source SHA-256 hashes in [`supported-build.json`](supported-build.json) before writing anything. It copies the player to a new directory, applies translations there, verifies the changed resources and unchanged code/data sections, and creates `MidRadio-English.exe` as the explicit launch target. It refuses existing output folders and never edits the original installation.

Close the Japanese player before launching the English copy. During development, launching an executable with the original name opened the installed copy; using the explicitly named English executable avoided that.

### Package a build

```powershell
py -3 pack.py
py -3 pack.py --portable .\MidRadio-English
```

The first command creates a source ZIP. The second also creates the Windows portable ZIP. Both write SHA-256 checksum sidecars to `dist/`. The portable packager verifies its input report and includes only player binaries, sound tables, documentation, and a build report.

### Development checks

```powershell
py -3 -m unittest discover -s tests -v
```

The synthetic tests run on Windows and Linux without Yamaha binaries. Tests cover string tables, standard and extended dialog alignment, menus, placeholders, font preservation, and source-build rejection. An optional local build check is:

```powershell
py -3 patch.py --dry-run --source "C:\Program Files (x86)\YAMAHA\MidRadio Player"
```

## Repository layout

```text
patch.py              Validated Windows resource patcher
pe_resources.py       PE32 resource and dialog/menu/string parsers
translations.txt      Japanese|English translations (UTF-8)
supported-build.json  Supported original hashes and translation counts
pack.py               Source/portable ZIP packaging and checksums
tests/                Synthetic regression tests
docs/                 Release notes and reverse-engineering findings
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). Helpful reports include untranslated labels, clipped controls, and playback or first-run results on Windows 10 x64. Include the player version and reproduction steps. Please do not upload music files, credentials, or personal settings.

## License and attribution

The patching tools are available under the [MIT License](LICENSE). Yamaha's executables, libraries, sound data, artwork, original interface text, and trademarks retain their original ownership and terms. **The MIT license does not relicense Yamaha's software or assets.** See [NOTICE.md](NOTICE.md).

This is an independent community project. It is not affiliated with, endorsed by, or supported by Yamaha.
