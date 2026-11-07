[English](#english) · [Русский](#russian)

# MidRadio English v0.1.0-preview

<a name="english"></a>

## English

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

---

<a name="russian"></a>

## Русский

Предварительная версия английского интерфейса Yamaha MidRadio Player 7.2.1.1: **756 переведённых текстовых полей в 10 файлах**.

### Загрузка

- **Портативная версия для Windows:** `yamahamdr-english-v0.1.0-preview-windows-x64.zip`
- **Исходный код патчера:** `yamahamdr-english-v0.1.0-preview-source.zip`
- Для каждого архива приложен файл контрольной суммы `.sha256`.

**Целевая система:** Windows 10 x64. Пользователь подтвердил работу в Windows 11 x64. Плеер остаётся исходным 32-разрядным приложением, работающим через WoW64. Русский перевод относится к документации; интерфейс плеера — английский.

### Установка и запуск

1. Закройте запущенный MidRadio Player.
2. В разделе **Assets** этого релиза скачайте **портативный ZIP для Windows**.
3. Нажмите на архив правой кнопкой мыши, выберите **«Извлечь всё»** и распакуйте его в папку с правом записи.
4. Не разделяйте исполняемый файл, DLL, звуковые таблицы и папку `Addins`.
5. Запустите **`MidRadio-English.exe`** из распакованной папки.
6. Открывайте музыку через **File → Open File**.

Python и установщик не нужны. Архив исходного кода предназначен для пересборки патча; самого плеера в нём нет. Для удаления портативной копии можно удалить её папку, но настройки исходного приложения могут остаться в реестре Windows.

### Состав и ограничения

Переведены меню, настройки, диалоги, подсказки и сообщения об ошибках. Включены существующий звуковой движок, дополнения и отчёт сборки. Патчер проверяет, что секции PE вне ресурсов остаются неизменными.

Некоторые надписи скина остаются японскими: выбор экрана, сведения о композиции и режимы повтора. Они встроены в растровую графику. Тексты песен, названия композиций, онлайн-контент и сторонние скины не переводятся.

Проверены запуск, английские меню и окно настроек MIDI. Зависимости при первом запуске на чистой Windows 10 и полноценная проверка всех форматов воспроизведения пока не исследованы. На компьютере без установленного MidRadio могут потребоваться дополнительные компоненты. Старые онлайн-сервисы могут быть недоступны.

Код патчера распространяется под MIT. Программы и материалы Yamaha сохраняют свои исходные права и условия; см. `NOTICE.md` в архиве исходного кода.

[↑ English](#english) · [↑ Русский](#russian)
