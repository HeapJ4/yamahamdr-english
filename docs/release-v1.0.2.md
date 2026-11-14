[English](#english) · [Русский](#russian)

# MidRadio English + Directory 1.0.2 — startup hotfix

<a name="english"></a>
## English

**Fixes the missing directory pane and Japanese graphical labels in v1.0/v1.0.1.** The C launcher and English menu resources worked, but extension startup posted a message while listening through a hook that only observes sent messages. That prevented both directory controls and the English skin hook from starting. This was a project regression, not a requirement to install a language pack.

The startup hook now handles queued messages on Yamaha's owning GUI thread. A new native regression creates the real directory controls in a hidden test window using that same posted-message path and verifies **MIDI-WAV EXPORT** exists. Existing layout tests remain. Startup success/failures are recorded with timestamps in `runtime.log`. A small guard also prevents native client height becoming zero during initial frame expansion.

### Install

1. Close the old player yourself. Do not leave an older copy running.
2. Download `yamahamdr-english-v1.0.2-windows-x64.zip` and **extract the entire ZIP into a fresh folder**.
3. Run that folder's **MidRadio.exe**. Keep its PlayerCore.dll, CoreOriginal.dll, MidRadioDecode.exe and Addins together. No Python or installer is needed. Target: Windows 10 x64; the Yamaha engine is x86 under WoW64.
4. Use a full-size mode so the directory pane is visible. Compact modes intentionally hide it.

### MMF / MIDI → WAV

Select an MMF/SMAF or MIDI track in the directory pane → **MIDI-WAV EXPORT** → save destination → let Yamaha play it in real time → **Finish export** → wait for **Saved WAV**. Use the **built-in Yamaha software synthesizer**; hardware MIDI output has no PCM through this path. WAV safety/recovery behavior is retained. This update does not change the synthesis or recording engine.

### Report an issue

Use **Report issue** in the directory seeker's top row. Write a title and description, optionally include basic app/Windows diagnostics, and review the Preview tab. **Continue on GitHub** opens the issue draft for your final review and submission. No GitHub token is embedded and no files/logs upload automatically. For long reports, a confirmed clipboard handoff preserves the full description. Attach screenshots or reviewed logs on GitHub.

If the player cannot start, run **MidRadioReport.exe** directly. Hidden-form tests exercise its startup and 100 Write/Preview switches without opening a browser or touching your clipboard.

### Verification boundary

The sent/posted mismatch was reproduced in a standalone Windows probe. The new startup regression verifies posted initialization, owning-thread creation and the actual export button, with network fetching and binary-import hooking excluded from the hidden fixture. Native layout/resource/codec/export checks run in CI. The final full Yamaha UI has not been manually driven: the user prohibits Computer Use. The earlier View/Edit and mode-transition reports therefore retain their manual-verification limitation.

<a name="russian"></a>
## Русский

**Исправлено отсутствие браузера каталогов и возвращение японских графических надписей в v1.0/v1.0.1.** Лаунчер и перевод меню работали, но расширение отправляло сообщение в очередь, используя обработчик только синхронных сообщений. Из-за этого не запускались ни панель каталогов, ни английские графические подписи. Это ошибка проекта, а не отсутствие языкового пакета.

Теперь WH_GETMESSAGE обрабатывает сообщение очереди в потоке интерфейса Yamaha. Новый нативный тест создаёт настоящие элементы панели в скрытом тестовом окне и проверяет кнопку **MIDI-WAV EXPORT**. Успех и ошибки запуска записываются в `runtime.log`. Дополнительно устранён временный нулевой размер родной клиентской области при первоначальном расширении окна.

### Установка

1. Самостоятельно закройте старый плеер; не оставляйте прежнюю копию работающей.
2. Скачайте `yamahamdr-english-v1.0.2-windows-x64.zip` и **распакуйте весь архив в новую папку**.
3. Запустите **MidRadio.exe** из этой папки. Не разделяйте PlayerCore.dll, CoreOriginal.dll, MidRadioDecode.exe и Addins. Python и установщик не нужны. Целевая система — Windows 10 x64, движок Yamaha — x86 через WoW64.
4. Выберите полноразмерный режим: компактные режимы намеренно скрывают панель каталогов.

### MMF / MIDI → WAV

Выделите MMF/SMAF или MIDI в панели каталогов → **MIDI-WAV EXPORT** → выберите файл → дождитесь воспроизведения в реальном времени → **Finish export** → **Saved WAV**. Нужен **встроенный программный синтезатор Yamaha**; внешний аппаратный MIDI не предоставляет PCM через этот путь. Защита и восстановление WAV сохранены; движок синтеза не изменён.

Несовместимость типов сообщений воспроизведена отдельной Windows-проверкой. Новый тест проверяет реальное создание элементов панели через сообщение очереди, но исключает сетевой запрос и перехват импорта бинарного файла. Полный интерфейс Yamaha вручную не управлялся: пользователь запретил Computer Use. Прежние сообщения о View/Edit и переключении режимов сохраняют ограничение ручной проверки.

### Форма сообщения об ошибке

В верхнем ряду панели каталогов есть **Report issue**: заголовок, описание, Write/Preview, форматирование, Copy report и **Continue on GitHub**. Пользователь проверяет и отправляет черновик уже на GitHub. Необязательная диагностика содержит только версию приложения, Windows и архитектуру. Автоматической загрузки журналов/файлов и встроенного токена нет. Если плеер не запустился, отдельно откройте **MidRadioReport.exe**; для длинного текста доступно подтверждаемое копирование без обрезания. Добавлены проверки кодирования черновика и запуска реальной скрытой формы.
