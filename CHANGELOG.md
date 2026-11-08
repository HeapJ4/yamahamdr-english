# Changelog

## 0.2.1

- Move Track Details another six logical pixels right, widen its text area, and remove compressed/double-rendered lettering.
- Keep the arrow icon and sample-rate fields unobstructed.
- Recognize percent-encoded OGG/M4A extensions during audio preparation.
- Publish downloads atomically and regenerate incomplete decoded WAV caches.
- Add native layout and cache-recovery regression tests to Windows CI.


## 0.2.0

- Combine English resources with the embedded native directory seeker.
- Translate screen/details/repeat skin lettering and shift Track Details right.
- Fix UTF-8 separators.
- Add isolated OGG Vorbis and Windows Media Foundation M4A decoding.
- Preserve native Yamaha formats and test HTTP media preparation.
- Ship the correct directory launcher, bilingual instructions, checksums, and source.


## 0.1.0-preview

- Translate 756 Win32 text fields across 10 MidRadio Player 7.2.1.1 binaries.
- Provide a portable folder package targeting Windows 10 x64.
- Include an explicit `MidRadio-English.exe` launch target.
- Publish Python patching tools, translation data, original build hashes, and deterministic ZIP packaging.
- Validate placeholder preservation, dialog alignment, written resources, and unchanged non-resource sections.
- Document remaining Japanese bitmap labels and the limits of runtime verification.
