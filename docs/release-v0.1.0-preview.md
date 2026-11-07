# MidRadio English v0.1.0-preview

An English interface preview for Yamaha MidRadio Player 7.2.1.1: **756 translated text fields across 10 files**.

## Download

- **Windows portable:** `yamahamdr-english-v0.1.0-preview-windows-x64.zip`
- **Patching source:** `yamahamdr-english-v0.1.0-preview-source.zip`
- Each archive has a matching `.sha256` checksum file.

**Target:** Windows 10 x64. The user confirms this preview works on Windows 11 x64. The player is the original 32-bit application running under WoW64.

## Install / run

1. Close any running MidRadio Player.
2. Download the **Windows portable ZIP** from this release's Assets section.
3. Right-click it, choose **Extract All**, and extract the entire folder to a location you can write to.
4. Keep the executable, DLLs, sound tables, and `Addins` directory together.
5. Run **`MidRadio-English.exe`** from the extracted folder.
6. Open music with **File → Open File**.

You do not need Python or an installer to launch this package. The source archive is for rebuilding the patch; it is not the player. You can remove the portable folder when finished, but the original application may retain preferences in the Windows registry.

## Included

English menus, settings, dialogs, tooltips, and error messages; the existing playback engine and plug-ins; and a build report. Code and data sections outside PE resources are verified unchanged by the patcher.

## Known limitations

Some skin graphics still contain Japanese lettering, including the screen selector label, track details button, and repeat-mode labels. Lyrics, song names, online content, and third-party skins are not translated.

The preview has been observed starting and showing English menus and MIDI settings. Clean-machine Windows 10 dependencies and comprehensive audio playback are not independently tested. A machine without an existing MidRadio installation may expose additional dependencies. Legacy online services may be unavailable.

The patching code is MIT-licensed. Yamaha's original software and assets retain their own ownership and terms; see `NOTICE.md` in the source archive.
