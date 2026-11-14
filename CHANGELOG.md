# Changelog

## 1.0.1

- Rename the directory seeker export button to **MIDI-WAV EXPORT** and widen it to fit. MMF/SMAF export remains supported.
- Update English/Russian instructions with the visible button name and the real-time export steps.

Изменения: кнопка экспорта в браузере каталогов переименована в **MIDI-WAV EXPORT** и расширена. Экспорт MMF/SMAF сохраняется; обновлены инструкции.


## 1.0

- Include the user-supplied C launcher unchanged.
- Install the main-window subclass on its owning GUI thread, isolate initialization messages, reserve native/directory geometry separately, hide compact-mode overlays, and stop timer-driven z-order changes.
- Preserve existing WAV destinations on failure; atomic completion, periodic recovery headers, startup timeout, and bounded audio buffers.
- Catch background worker exceptions and timestamp diagnostics.
- Add automated native layout/recovery regressions. Final real-mode/menu interaction remains manually unverified.

### Integrated exporter

- Add Export WAV and Finish export controls directly to the directory pane.
- Capture Yamaha-rendered MMF/SMAF and GM MIDI audio as real-time PCM WAV.
- Hook the dedicated SMAF, MIDI and Wave output modules, preserving normal playback.
- Finalize WAV headers on Finish/reset/close and guard against mixing the previous track or another audio device.
- Verify real MMF and GM MIDI exports and add native PCM/WAV regression tests to Windows CI.


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
