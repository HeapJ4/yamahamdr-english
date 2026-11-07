[English](#english) · [Русский](#russian)

# MidRadio English + Directory 0.2.0

<a name="english"></a>

## English

This is the combined build: English interface, embedded directory seeker, original Yamaha skin, English graphical labels, and OGG Vorbis support. Track Details is shifted three pixels right and the stray Â separator is fixed.

### Download / install

1. Download **yamahamdr-english-v0.2.0-windows-x64.zip** from Assets.
2. Close other copies, extract the **entire ZIP**, and keep Addins and all DLLs together.
3. Run **MidRadio-Directory.exe**. No Python, installer, or administrator rights are needed.
4. Use Open folder, Play selected, Play all, and the right-click Copy full URL command.

Target: Windows 10 x64; the user confirms the directory build on Windows 11 x64. The player is x86 under WoW64. Clean Windows 10 setup remains untested.

WAV/MP3 and native MMF/SMAF, MRM, MRL and MIDI use Yamaha's engine. OGG Vorbis and Windows-supported M4A (including AAC) are decoded by the separate helper before native WAV playback. Extra OGG/M4A support is through the directory seeker; Opus-in-Ogg and encrypted M4A are excluded. Audio is cached in %TEMP%/MidRadio-Directory-cache, removable after closing the app. Settings may remain in the registry.

The source ZIP includes the patcher, C++ extension, separate decoder, retained stb_vorbis license, synthetic fixtures, tests, and packaging tools. Both ZIPs have SHA-256 sidecars. The original Yamaha software retains its own terms.

Validation: local resource tests, synthetic audio decoding (OGG/M4A/MP3/WAV), HTTP media preparation, malformed OGG rejection, directory parser tests, and skin icon-preservation tests passed. CI additionally builds the x86 components and tests the Vorbis decoder on Windows. The earlier text-only preview was withdrawn; this release replaces it.

---

<a name="russian"></a>

## Русский

Объединённая сборка: английский интерфейс, встроенный браузер каталогов, оригинальный скин Yamaha, английские графические надписи и OGG Vorbis. Надпись Track Details сдвинута на три пикселя вправо, лишний символ Â исправлен.

### Загрузка / установка

1. Скачайте **yamahamdr-english-v0.2.0-windows-x64.zip** из Assets.
2. Закройте другие копии и распакуйте **весь архив**. Не разделяйте DLL и папку Addins.
3. Запустите **MidRadio-Directory.exe**. Python, установщик и права администратора не нужны.
4. Используйте Open folder, Play selected, Play all и правую кнопку мыши → Copy full URL.

Целевая система — Windows 10 x64; пользователь подтвердил работу сборки с каталогами в Windows 11 x64. Плеер x86 работает через WoW64. Чистая установка Windows 10 отдельно не проверялась.

WAV/MP3 и родные форматы MMF/SMAF, MRM, MRL и MIDI воспроизводятся движком Yamaha. OGG Vorbis и поддерживаемые Windows файлы M4A (включая AAC) обрабатываются отдельным декодером и передаются как PCM WAV. Дополнительная поддержка действует через браузер каталогов; Opus-in-Ogg и зашифрованные M4A не поддерживаются. Кеш хранится в %TEMP%/MidRadio-Directory-cache; удаляйте его после закрытия программы. Настройки могут сохраняться в реестре.

Архив исходного кода содержит патчер, расширение C++, декодер, лицензию stb_vorbis, синтетические записи, тесты и упаковщик. К архивам приложены SHA-256. Материалы Yamaha сохраняют исходные условия.

Локальные проверки ресурсов, аудиодекодирования, подготовки файлов по HTTP, отклонения повреждённого OGG, разбора каталогов и сохранения значков прошли успешно. CI также собирает компоненты x86 и проверяет Vorbis в Windows. Предыдущий релиз только с переводом снят с публикации и заменён этой сборкой.
