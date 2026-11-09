[English](#english) · [Русский](#russian)

# MidRadio English + Directory 1.0

<a name="english"></a>

## English

**MMF / GM MIDI → WAV export is now inside the player.** English text, the directory seeker, English graphics, OGG/M4A handling, the shifted Track Details label, and cache fixes are retained.

### Install

Download **`yamahamdr-english-v1.0-windows-x64.zip`**, close older copies, extract everything, and run **`MidRadio.exe`**. Keep all DLLs and `Addins` together. No Python or installer is needed. Windows 10 x64 is the target; the live checks were performed on Windows 11 x64 with the original x86 player under WoW64.

### Render MMF or MIDI to WAV

1. Select the track in the directory pane and click **Export WAV**.
2. Choose an output `.wav` file.
3. Let Yamaha render it with the **built-in software synthesizer**.
4. Click **Finish export** when done and wait for **Saved WAV**. Device reset/close can finalize it automatically.

Export runs in real time, using PCM submitted by this player's own Yamaha output modules. It does not record the microphone or other applications. Hardware MIDI output cannot be exported as PCM through this path. Do not close the application before the WAV has finalized. The size limit is 1 GiB.

### Changes in v1.0

- Added integrated MMF/SMAF and GM MIDI → PCM WAV export through Yamaha's software synthesizer.
- Included the owner's existing C `MidRadio.exe` launcher, unchanged and checksum verified.
- Moved main-window subclass installation onto Yamaha's GUI thread; use a registered initialization message rather than a private message number in Yamaha's range.
- Reserve directory space separately from native skin space; remove it in compact modes, restore it in full-size modes, and prevent accumulated window growth. Fixed-coordinate labels hide in compact modes and while native menus are open; timers no longer repeatedly raise panes above native controls.
- Added destination preflight, atomic final WAV replacement, two-second recovery-header checkpoints, 30-second no-audio timeout, bounds, and explicit failure messages. A failed export preserves any pre-existing destination.
- Catch directory/playback worker exceptions, retain network/decoder timeouts, and timestamp diagnostic logs with process/thread IDs.
- Retained English resources/graphics, shifted Track Details, OGG Vorbis and supported M4A handling, and cache-recovery fixes from earlier releases.

### Verification

- Live native MMF export: **31.211 s, 44.1 kHz, stereo, 16-bit PCM**, non-silent.
- Live GM piano MIDI export: **3.697 s, 44.1 kHz, stereo, 16-bit PCM**, non-silent.
- Native export tests check PCM preservation, WAV headers, previous-track/device isolation, reset finalization, and empty-export handling.
- Headless native layout tests run 100 compact/full-size round trips and check that position-only changes never grow the window. Export tests cover recovery and destination preservation.
- The reported View → Edit interaction and every real Yamaha display mode have not been manually rechecked in this final build; Computer Use was prohibited. Automated geometry tests do not prove the exact reported crash is eliminated.
- Windows/Linux CI builds the native code and checks resources, OGG decoding, cache recovery, label placement, and WAV export logic.

Portable/source ZIPs and SHA-256 checksums are attached. Yamaha software keeps its original terms; patch code is MIT-licensed.

---

<a name="russian"></a>

## Русский

**Экспорт MMF / GM MIDI → WAV теперь встроен в плеер.** Сохранены английский интерфейс и графика, браузер каталогов, OGG/M4A, сдвинутая надпись Track Details и исправления кеша.

### Установка

Скачайте **`yamahamdr-english-v1.0-windows-x64.zip`**, закройте старые копии, распакуйте весь архив и запустите **`MidRadio.exe`**. Не разделяйте DLL и папку `Addins`. Python и установщик не нужны. Целевая система — Windows 10 x64; живые проверки выполнены в Windows 11 x64, исходный плеер x86 работает через WoW64.

### Экспорт MMF или MIDI в WAV

1. Выберите композицию в браузере каталогов и нажмите **Export WAV**.
2. Укажите выходной файл `.wav`.
3. Дождитесь рендеринга **встроенным программным синтезатором Yamaha**.
4. Нажмите **Finish export** и дождитесь **Saved WAV**. Сброс или закрытие аудиоустройства также может завершить файл автоматически.

Экспорт идёт в реальном времени и использует PCM самой Yamaha. Микрофон и звук других приложений не записываются. Аппаратный MIDI не выдаёт PCM этим способом. Не закрывайте приложение до завершения WAV. Лимит файла — 1 ГиБ.

### Проверка

- Реальный MMF: **31,211 с, 44,1 кГц, стерео, PCM 16 бит**, звук присутствует.
- GM MIDI с фортепиано: **3,697 с, 44,1 кГц, стерео, PCM 16 бит**, звук присутствует.
- Тесты проверяют сохранение PCM, заголовки WAV, изоляцию от предыдущей композиции и другого устройства, завершение при сбросе и отсутствие ложного успеха при пустом экспорте.
- CI Windows/Linux собирает нативный код и проверяет ресурсы, OGG, кеш, расположение надписей и логику WAV-экспорта.

Приложены портативная сборка, исходники и SHA-256. Материалы Yamaha сохраняют свои условия; код патчера — MIT.

### Изменения и устойчивость v1.0

Добавлен исходный C-лаунчер владельца `MidRadio.exe` без изменения байтов. Установка обработчика окна перенесена в поток интерфейса Yamaha, и используется уникальное зарегистрированное сообщение. Для компактных режимов область каталога и стандартные графические подписи скрываются; при возврате к полноразмерному режиму каталог получает собственную область. Устранено накопительное увеличение высоты окна и постоянное поднятие элементов поверх родных контролов.

WAV сначала записывается в соседний файл `.partial-…wav`. Заголовок восстановления обновляется каждые две секунды аудио; основной файл заменяется только после успешного завершения. Если звук не появился за 30 секунд, экспорт прекращается с объяснением. Ошибка записи сохраняет прежний файл назначения. При аварийном завершении приложения файл восстановления может содержать только часть композиции. Ошибки фоновых задач перехватываются; журнал `runtime.log` содержит время, номер процесса и потока.

Автоматическая проверка выполняет 100 переключений размеров компактного/полного режима. Конкретный переход View → Edit и все настоящие режимы Yamaha в финальной сборке вручную повторно не проверялись: управление компьютером запрещено пользователем. Эти тесты не гарантируют устранение каждого сбоя старого движка.
