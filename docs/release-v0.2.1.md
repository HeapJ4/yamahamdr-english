[English](#english) · [Русский](#russian)

# MidRadio English + Directory 0.2.1

<a name="english"></a>

## English

### Fixes

- **Track Details** moves six more logical pixels right. Its wider text area uses uncompressed lettering and no longer draws a duplicate label underneath. The arrow icon and sample-rate fields remain clear.
- Percent-encoded extensions such as `tone%2Eogg` now select the correct decoder.
- Downloads are finalized atomically; incomplete decoded WAV cache files are regenerated.
- Windows CI now tests glyph placement and interrupted-cache recovery, alongside OGG decoding.

### Install

1. Download **`yamahamdr-english-v0.2.1-windows-x64.zip`** from Assets.
2. Close every older copy of MidRadio, then extract the whole ZIP.
3. Keep all files and `Addins` together and run **`MidRadio-Directory.exe`**.

Windows 10 x64 is the target; the directory build has also been used on Windows 11 x64. The Yamaha player remains x86 under WoW64. No Python or installer is required to launch the portable package.

The source ZIP and SHA-256 sidecars are included. English resources, the embedded directory seeker, OGG Vorbis, Windows-supported M4A decoding, and native Yamaha formats are retained. Extra OGG/M4A support applies through the directory seeker. The cache is in `%TEMP%\MidRadio-Directory-cache`.

**MMF/MIDI-to-WAV export is not implemented.** These files contain performance/synthesis data and need an audio rendering path; changing an extension or passing them through the current OGG/M4A helper is not conversion.

---

<a name="russian"></a>

## Русский

### Исправления

- Надпись **Track Details** сдвинута ещё на шесть логических пикселей вправо. Область текста расширена, сжатие букв и дублирующая надпись под ней убраны. Значок стрелки и поля частоты остаются свободными.
- Расширения с процентным кодированием, например `tone%2Eogg`, теперь выбирают правильный декодер.
- Загрузки завершаются атомарно; повреждённый или неполный кеш декодированного WAV создаётся заново.
- В CI Windows добавлены проверки расположения текста и восстановления кеша.

### Установка

1. Скачайте **`yamahamdr-english-v0.2.1-windows-x64.zip`** из Assets.
2. Закройте все старые копии MidRadio и распакуйте весь архив.
3. Не разделяйте файлы и папку `Addins`. Запустите **`MidRadio-Directory.exe`**.

Целевая система — Windows 10 x64; сборка с каталогами также использовалась в Windows 11 x64. Исходный плеер x86 работает через WoW64. Python и установщик для запуска не нужны.

Приложены исходный код и SHA-256. Сохранены английский интерфейс, браузер каталогов, OGG Vorbis, M4A с поддерживаемыми Windows кодеками и форматы Yamaha. Дополнительная обработка OGG/M4A действует через браузер каталогов. Кеш находится в `%TEMP%\MidRadio-Directory-cache`.

**Экспорт MMF/MIDI в WAV пока не реализован.** Для этих файлов требуется синтез звука. Переименование файла или использование текущего декодера OGG/M4A не заменяет рендеринг.
