# Resource investigation

## Supported input

MidRadio Player file/product version: **7.2.1.1**. The main executable's original SHA-256 is:

```text
839a465ae05f0cb3599df1771660b1de95faabe964a709bdd7af3df100c47c53
```

The original program uses native PE32 binaries. Its interface has Win32 menu (type 4), dialog (type 5), string-table (type 6), and bitmap (type 2) resources. Japanese resources generally use language ID 1041. Some add-ins also contain separate English resources, but those are not a complete main-player localization.

The initial text inventory found 484 distinct non-ASCII values, including two Japanese font names that remain unchanged. Repeated occurrences across multiple binaries account for the 756 translated fields. The implementation preserves existing resource languages and IDs, and replaces payloads through the Windows resource update API. It does not infer runtime behavior from resource text alone.

## Why some labels remain Japanese

Three directly inspected bitmap resources in `MidRadio.exe` contain labels as pixels:

| Resource | Dimensions | Content |
| --- | --- | --- |
| 430 | 112 × 20 | Select Screen |
| 359 | 60 × 52 | Track Details button states |
| 368 | 47 × 378 | Repeat-mode button states |

Replacing string tables cannot change these pixels. This preview leaves the bitmap payloads intact. A future skin pass needs to preserve sprite dimensions, transparency/mask behavior, and all button states.

## Verification boundary

- The initial build translated 756 fields in 10 files and round-tripped the resource parser.
- Each modified file retained identical non-resource PE section bytes.
- The named English executable was observed starting with English menu headings and menu items.
- The MIDI output settings dialog was observed in English.
- The user reports the preview working on Windows 11.
- A clean Windows 10 installation, every dialog, and comprehensive audio-format playback have not been independently verified.

This repository contains a resource patcher, not recovered Yamaha source code. Static observations do not establish how legacy online services or every playback subsystem behave.
