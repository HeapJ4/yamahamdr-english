[English](#english) · [Русский](#russian)

# MidRadio English

<a name="english"></a>

## English

**Yamaha MidRadio Player, with an English interface.**

[![Tests](https://github.com/HeapJ4/yamahamdr-english/actions/workflows/check.yml/badge.svg)](https://github.com/HeapJ4/yamahamdr-english/actions/workflows/check.yml)
[![License: MIT](https://img.shields.io/badge/patch_code-MIT-blue.svg)](LICENSE)
![Target: Windows 10 x64](https://img.shields.io/badge/target-Windows_10_x64-0078D6)
![Status: Preview](https://img.shields.io/badge/status-preview-orange)

An unofficial English localization of **MidRadio Player 7.2.1.1**. This preview translates **756 text fields across 10 files**, including menus, settings, dialogs, tooltips, and error messages.

## Get the portable version

Open [Releases](https://github.com/HeapJ4/yamahamdr-english/releases) and download **`yamahamdr-english-v0.1.0-preview-windows-x64.zip`**.

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

The repository requires pull requests and passing CI for `main`, protects release tags, and restricts external workflow runs. See [SECURITY.md](SECURITY.md) for the protection model and vulnerability reporting.

## License and attribution

The patching tools are available under the [MIT License](LICENSE). Yamaha's executables, libraries, sound data, artwork, original interface text, and trademarks retain their original ownership and terms. **The MIT license does not relicense Yamaha's software or assets.** See [NOTICE.md](NOTICE.md).

This is an independent community project. It is not affiliated with, endorsed by, or supported by Yamaha.

[↑ Language selection](#midradio-english)

---

<a name="russian"></a>

## Русский

**Yamaha MidRadio Player с английским интерфейсом.**

Неофициальная английская локализация **MidRadio Player 7.2.1.1**. В этой предварительной версии переведены **756 текстовых полей в 10 файлах**: меню, настройки, диалоговые окна, подсказки и сообщения об ошибках. Русский язык здесь используется для документации; интерфейс плеера переведён на английский.

### Скачать портативную версию

Откройте [Releases](https://github.com/HeapJ4/yamahamdr-english/releases) и скачайте **`yamahamdr-english-v0.1.0-preview-windows-x64.zip`**.

1. Закройте все запущенные копии MidRadio Player.
2. Нажмите правой кнопкой мыши на ZIP и выберите **«Извлечь всё»**. Все файлы и папка `Addins` должны оставаться вместе.
3. Откройте распакованную папку и запустите **`MidRadio-English.exe`**.
4. Для открытия музыкального файла выберите **File → Open File**.

Для запуска портативного пакета не нужны Python, установщик, права администратора или замена установленного плеера. Исходная программа может сохранять настройки в реестре Windows: под «портативностью» здесь подразумевается запуск из распакованной папки, а не полное отсутствие следов в системе.

**Основная целевая система:** Windows 10 x64. **Работоспособность подтверждена пользователем:** Windows 11 x64. Сам плеер остаётся **32-разрядным приложением x86**, которое запускается в 64-разрядной Windows через WoW64. Звуковой движок не преобразован в нативный x64. Отдельная проверка на чистой установке Windows 10 пока не проводилась.

### Что переведено

| Компонент | Текстовых полей |
| --- | ---: |
| Основной плеер | 419 |
| Модули синтезатора | 170 |
| Дополнение с аккордами и песенником | 54 |
| Караоке | 49 |
| Элемент ActiveX | 31 |
| Настройка синтезатора SGP2 | 16 |
| Слайд-шоу | 12 |
| Дополнения формата и вывода MIDI | 5 |
| **Всего** | **756** |

Патч изменяет текстовые ресурсы Win32. Идентификаторы ресурсов, структура элементов диалоговых окон, подстановочные параметры, сочетания клавиш, названия шрифтов и байты секций PE вне ресурсов сохраняются.

### Ограничения предварительной версии

- Часть японских надписей встроена в растровые изображения скина: **выбор экрана**, **сведения о композиции** и **режимы повтора**. В этой версии они остаются на японском.
- Названия композиций, тексты песен, сторонние скины и онлайн-контент не переводятся.
- На машине разработки проверены запуск, переведённые меню и окно настроек MIDI. Все диалоги и воспроизведение каждого формата систематически не проверялись.
- Проект не восстанавливает старые онлайн-сервисы и не подтверждает их доступность.
- Портативная сборка запускалась на компьютере, где MidRadio уже был установлен. На чистом компьютере могут обнаружиться дополнительные зависимости.

### Самостоятельная сборка

Потребуются **Windows**, **Python 3.9 или новее** и исходная установка MidRadio Player 7.2.1.1. Дополнительные пакеты pip не нужны.

В папке с исходным кодом выполните:

```powershell
py -3 patch.py --dry-run
py -3 patch.py --output .\MidRadio-English
```

Если плеер установлен в другом месте:

```powershell
py -3 patch.py --source "D:\Apps\MidRadio Player" --output "D:\Apps\MidRadio-English"
```

Перед записью патчер проверяет SHA-256 исходных файлов по [`supported-build.json`](supported-build.json). Затем копирует плеер в новую папку, применяет перевод, проверяет ресурсы и неизменность остальных секций и создаёт явную точку запуска `MidRadio-English.exe`. Существующую выходную папку он не перезаписывает. Исходная установка остаётся нетронутой.

Перед запуском английской копии закройте японскую. Во время разработки запуск файла с исходным именем открывал установленную копию; отдельное имя `MidRadio-English.exe` позволило избежать этого.

### Упаковка и проверки

```powershell
py -3 pack.py
py -3 pack.py --portable .\MidRadio-English
py -3 -m unittest discover -s tests -v
```

Первая команда создаёт архив исходного кода. Вторая дополнительно создаёт портативный ZIP для Windows. Архивы и файлы контрольных сумм SHA-256 сохраняются в `dist/`. Упаковщик проверяет отчёт сборки и включает только файлы плеера, звуковые таблицы, документацию и отчёт.

Синтетические тесты работают в Windows и Linux без файлов Yamaha. Они проверяют строковые таблицы, выравнивание обычных и расширенных диалогов, меню, параметры форматирования, сохранение шрифтов и отклонение неподдерживаемых исходных файлов. Для проверки установленного плеера без записи файлов:

```powershell
py -3 patch.py --dry-run --source "C:\Program Files (x86)\YAMAHA\MidRadio Player"
```

### Структура проекта

| Файл или папка | Назначение |
| --- | --- |
| `patch.py` | Патчер ресурсов Windows с проверкой сборки |
| `pe_resources.py` | Разбор PE32, диалогов, меню и строк |
| `translations.txt` | Переводы в формате `японский\|английский`, UTF-8 |
| `supported-build.json` | Хеши исходных файлов и число переводов |
| `pack.py` | Упаковка ZIP и создание контрольных сумм |
| `tests/` | Регрессионные тесты |
| `docs/` | Описание релиза и результаты исследования ресурсов |

### Участие и безопасность

См. [CONTRIBUTING.md](CONTRIBUTING.md). Полезны сообщения о непереведённых надписях, обрезанном тексте и результатах первого запуска или воспроизведения в Windows 10 x64. Укажите версию плеера и шаги воспроизведения проблемы. Не загружайте музыкальные файлы, учётные данные или личные настройки.

Изменения `main` требуют pull request и успешных проверок CI. Теги версий защищены, а запуск внешних workflow требует разрешения сопровождающего. Модель защиты и порядок сообщения об уязвимостях описаны в [SECURITY.md](SECURITY.md).

### Лицензия и авторство

Инструменты патчера распространяются под [лицензией MIT](LICENSE). Исполняемые файлы, библиотеки, звуковые данные, графика, исходные тексты интерфейса и товарные знаки Yamaha сохраняют исходные права и условия. **MIT не меняет лицензию программного обеспечения и материалов Yamaha.** См. [NOTICE.md](NOTICE.md).

Это независимый общественный проект. Он не связан с Yamaha, не одобрен компанией и не поддерживается ею.

[↑ English](#english) · [↑ Русский](#russian)
