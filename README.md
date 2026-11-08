[English](#english) · [Русский](#russian)

# MidRadio English + Directory

[![Tests](https://github.com/HeapJ4/yamahamdr-english/actions/workflows/check.yml/badge.svg)](https://github.com/HeapJ4/yamahamdr-english/actions/workflows/check.yml)
![Windows 10 x64](https://img.shields.io/badge/Windows-10_x64-0078D6)
[![MIT patch code](https://img.shields.io/badge/patch_code-MIT-blue.svg)](LICENSE)

<a name="english"></a>

## English

**Yamaha MidRadio Player 7.2.1.1 with English text, English graphical labels, and an embedded HTTP directory seeker.** The original Yamaha skin and window border are retained. Version **0.2.1** combines the features in one portable folder.

### Download and run

1. Open [Releases](https://github.com/HeapJ4/yamahamdr-english/releases) and download **`yamahamdr-english-v0.2.1-windows-x64.zip`**.
2. Close other copies of MidRadio, right-click the ZIP, and choose **Extract All**.
3. Keep all files and `Addins` together. Run **`MidRadio-Directory.exe`**.

No Python, installer, or administrator rights are needed to launch the package. Target: **Windows 10 x64**; the user also confirms the directory build on **Windows 11 x64**. The original player is x86, running through WoW64. A clean Windows 10 machine has not been independently tested. The player may retain preferences in the Windows registry.

### Directory seeker

The directory pane is embedded below the original skinned player. It starts at the Samsung SGH-D500 ringtone directory.

- Paste an Apache/nginx HTTP(S) directory URL and press **Open folder**.
- Double-click folders to navigate; use **Up** to go to the parent.
- Filter names, then use **Play selected** for one track or **Play all** for the visible list.
- Right-click a row and choose **Copy full URL** to copy its link.

### Audio and native Yamaha files

| Files opened through the directory seeker | Handling |
| --- | --- |
| WAV, MP3 | Yamaha's original playback engine |
| MMF/SMAF, MRM, MRL, MID/MIDI | Existing Yamaha format add-ins |
| OGG **Vorbis**, mono/stereo | Separate bounded decoder → 16-bit PCM WAV → Yamaha |
| M4A with a Windows-supported codec, including AAC | Windows Media Foundation → PCM WAV → Yamaha |

OGG here means Vorbis, not Opus-in-Ogg. Extra OGG/M4A handling applies to files opened through the **directory seeker**, rather than adding entries to Yamaha's original file-open codec list. Encrypted or unsupported M4A codecs are not supported.

Tracks and decoded PCM are cached in `%TEMP%\MidRadio-Directory-cache`. Close the player before removing that cache. A compressed download is limited to 256 MiB and decoded PCM to 512 MiB. Decoding runs in a separate process with a timeout; the experimental Yamaha Ogg plug-in is excluded. MRL files retain the original player's playlist behavior, including its handling of relative references.

### English interface

The text patch translates **756 fields across 10 files**. Runtime skin rendering replaces **Select Screen**, **Track Details**, and repeat-mode lettering while retaining the artwork and click areas. Track Details has been shifted slightly right. The directory pane uses UTF-8 source text, fixing the stray `Â` separator.

Titles, lyrics, external skins, and online content are not translated. This project does not restore obsolete Yamaha online services.

### Build from source

Use Windows, Python 3.9+, an **x86 MinGW C++ compiler**, and an original MidRadio Player 7.2.1.1 installation:

```powershell
py -3 build_directory.py --compiler "C:\path\to\i686-w64-mingw32-g++.exe" --output .\MidRadio-Directory
py -3 pack.py --portable .\MidRadio-Directory
```

The builder checks original hashes, builds a separate folder, preserves the Yamaha core as `CoreOriginal.dll`, compiles the directory/skin extension and `MidRadioDecode.exe`, and records every shipped binary's SHA-256 in `directory-build.json`. It does not overwrite the installed player. `pack.py` creates source and portable ZIPs plus checksum sidecars in `dist/`.

For the text patch alone:

```powershell
py -3 patch.py --dry-run
py -3 patch.py --output .\MidRadio-English
```

### Tests and contributions

```powershell
py -3 -m unittest discover -s tests -v
```

CI cross-compiles the native x86 components and tests the Python resources on Windows/Linux. Windows CI exercises the separate Vorbis decoder. Local checks also cover M4A/MP3/WAV decoding, HTTP media preparation, native-format byte preservation, malformed OGG rejection, directory parsing, and skin icon preservation. The fixtures are synthetic tones. Optional decoder tests use `MIDRADIO_DECODER`; set `MIDRADIO_TEST_MEDIA_FOUNDATION=1` for local M4A tests.

See [CONTRIBUTING.md](CONTRIBUTING.md), [SECURITY.md](SECURITY.md), and [the resource findings](docs/findings.md). `main` requires PRs, resolved discussions, and passing Windows/Linux checks. Main and version tags are protected; external workflow runs need maintainer approval.

Patch code: [MIT](LICENSE). Yamaha components retain their original ownership and terms: [NOTICE.md](NOTICE.md). The separate decoder includes stb_vorbis under its retained MIT license: [third-party notices](THIRD_PARTY_NOTICES.md). This is an independent project, unaffiliated with Yamaha.

---

<a name="russian"></a>

## Русский

**Yamaha MidRadio Player 7.2.1.1 с английским интерфейсом, английскими графическими надписями и встроенным браузером HTTP-каталогов.** Оригинальный скин и рамка окна Yamaha сохранены. Версия **0.2.1** объединяет эти возможности в одной портативной папке. На русский переведена документация; интерфейс программы — английский.

### Загрузка и запуск

1. Откройте [Releases](https://github.com/HeapJ4/yamahamdr-english/releases) и скачайте **`yamahamdr-english-v0.2.1-windows-x64.zip`**.
2. Закройте другие копии MidRadio. Нажмите на ZIP правой кнопкой мыши и выберите **«Извлечь всё»**.
3. Не разделяйте файлы и папку `Addins`. Запустите **`MidRadio-Directory.exe`**.

Python, установщик и права администратора для запуска не нужны. Основная система — **Windows 10 x64**; пользователь также подтвердил работу сборки с браузером каталогов в **Windows 11 x64**. Исходный плеер остаётся 32-разрядным и работает через WoW64. Чистая установка Windows 10 отдельно не проверялась. Настройки плеера могут сохраняться в реестре Windows.

### Браузер каталогов

Панель встроена под оригинальным плеером. По умолчанию открывается каталог мелодий Samsung SGH-D500.

- Вставьте адрес HTTP(S)-каталога Apache/nginx и нажмите **Open folder**.
- Двойной щелчок открывает папку; **Up** возвращает в родительский каталог.
- Используйте фильтр, **Play selected** для выбранного файла или **Play all** для видимого списка.
- Правая кнопка мыши → **Copy full URL** копирует ссылку.

### Форматы

| Файлы, открываемые через браузер каталогов | Обработка |
| --- | --- |
| WAV, MP3 | Исходный звуковой движок Yamaha |
| MMF/SMAF, MRM, MRL, MID/MIDI | Штатные дополнения Yamaha |
| OGG **Vorbis**, моно/стерео | Отдельный декодер → PCM WAV → Yamaha |
| M4A с поддерживаемым Windows кодеком, включая AAC | Windows Media Foundation → PCM WAV → Yamaha |

Под OGG понимается Vorbis, а не Opus в контейнере Ogg. Дополнительная обработка OGG/M4A действует через **браузер каталогов**, а не через исходный список кодеков окна открытия файлов Yamaha. Зашифрованные M4A и неподдерживаемые кодеки не воспроизводятся.

Загруженные файлы и декодированный PCM сохраняются в `%TEMP%\MidRadio-Directory-cache`. Удаляйте кеш после закрытия плеера. Загрузка ограничена 256 МиБ, декодированный PCM — 512 МиБ. Декодер запускается отдельным процессом с тайм-аутом. Экспериментальный Ogg-плагин Yamaha в сборку не включён. Файлы MRL обрабатываются штатным плеером, включая его правила относительных ссылок.

### Перевод и графика

Переведены **756 полей в 10 файлах**. Графические надписи **Select Screen**, **Track Details** и режимы повтора заменяются во время отрисовки; исходные изображения и области нажатия сохраняются. Track Details немного сдвинута вправо. Ошибка кодировки с лишним символом `Â` исправлена.

Названия композиций, тексты песен, сторонние скины и онлайн-контент не переводятся. Устаревшие онлайн-сервисы Yamaha не восстанавливаются.

### Самостоятельная сборка

Нужны Windows, Python 3.9+, **32-разрядный компилятор MinGW C++** и исходная установка MidRadio Player 7.2.1.1:

```powershell
py -3 build_directory.py --compiler "C:\path\to\i686-w64-mingw32-g++.exe" --output .\MidRadio-Directory
py -3 pack.py --portable .\MidRadio-Directory
```

Сборщик проверяет хеши исходных файлов, создаёт отдельную папку, сохраняет исходное ядро как `CoreOriginal.dll`, компилирует расширение и `MidRadioDecode.exe` и записывает SHA-256 файлов в `directory-build.json`. Установленный плеер не изменяется. ZIP-архивы и контрольные суммы появляются в `dist/`.

Проверки:

```powershell
py -3 -m unittest discover -s tests -v
```

CI собирает нативные компоненты x86 и проверяет ресурсы в Windows/Linux. В Windows проверяется декодирование Vorbis. Локально также проверены M4A/MP3/WAV, подготовка файлов по HTTP, сохранение файлов Yamaha без изменений, отклонение повреждённого OGG, разбор каталогов и сохранение графических значков. Тестовые записи — синтетические тоны. Для тестов декодера задайте `MIDRADIO_DECODER`; для дополнительных проверок Media Foundation — `MIDRADIO_TEST_MEDIA_FOUNDATION=1`.

Участие: [CONTRIBUTING.md](CONTRIBUTING.md). Безопасность: [SECURITY.md](SECURITY.md). Изменения `main` проходят через PR и успешный CI; основная ветка и теги версий защищены, запуск внешних workflow требует разрешения сопровождающего.

Код патчера: [MIT](LICENSE). Материалы Yamaha сохраняют исходные права: [NOTICE.md](NOTICE.md). Лицензия stb_vorbis приведена в [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). Проект независим и не связан с Yamaha.

[↑ English](#english) · [↑ Русский](#russian)
