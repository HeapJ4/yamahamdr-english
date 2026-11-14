[English](#english) · [Русский](#russian)

# MidRadio English + Directory 1.0.1

<a name="english"></a>
## English

The directory seeker now has a clearly named **MIDI-WAV EXPORT** button. It also exports **MMF/SMAF** tracks. The button is wider so the label fits; playback, the Yamaha synthesizer and export safeguards are unchanged.

Download `yamahamdr-english-v1.0.1-windows-x64.zip`, close older copies, extract the entire ZIP and run `MidRadio.exe`. Keep the DLLs and `Addins` together. Windows 10 x64 is the target; the underlying Yamaha player is x86 under WoW64.

1. Use a full-size Yamaha mode so the directory seeker is visible (compact modes hide it).
2. Open the HTTP directory containing your `.mid`, `.midi` or `.mmf` file.
3. Single-click the file in the list, then press **MIDI-WAV EXPORT**.
4. Choose the output `.wav` filename and save.
5. Use Yamaha's **built-in software synthesizer** and let the track play in real time.
6. Press **Finish export** when finished and wait for **Saved WAV**. Native device reset/close can finish automatically.

This captures this player's synthesized PCM, not your microphone or other programs. Hardware MIDI output does not provide PCM through this path. Existing destination files are replaced only after successful finalization; `.partial-…wav` files provide recovery checkpoints. The 1 GiB output limit and 30-second no-audio timeout remain.

The reported native menu/mode issues from v1.0 have not been manually rechecked. This update changes the button label/layout and documentation, not the rendering engine.

<a name="russian"></a>
## Русский

В браузере каталогов появилась явно подписанная кнопка **MIDI-WAV EXPORT**. Она также экспортирует **MMF/SMAF**. Кнопка расширена под новый текст; синтезатор, воспроизведение и защита экспорта не изменены.

Скачайте `yamahamdr-english-v1.0.1-windows-x64.zip`, закройте прежнюю копию, распакуйте весь архив и запустите `MidRadio.exe`. Не разделяйте DLL и `Addins`. Целевая система — Windows 10 x64; исходный движок Yamaha x86 работает через WoW64.

1. Выберите полноразмерный режим Yamaha: компактные режимы скрывают браузер каталогов.
2. Откройте HTTP-каталог с файлом `.mid`, `.midi` или `.mmf`.
3. Выделите файл одним щелчком и нажмите **MIDI-WAV EXPORT**.
4. Выберите имя выходного `.wav` и сохраните.
5. Используйте **встроенный программный синтезатор Yamaha** и дождитесь воспроизведения в реальном времени.
6. Нажмите **Finish export** и дождитесь **Saved WAV**. Сброс или закрытие устройства вывода может завершить экспорт автоматически.

Записывается PCM самого плеера, а не микрофон или другие приложения. Внешний аппаратный MIDI через этот путь не экспортируется. Прежний файл назначения заменяется только после успешного завершения; `.partial-…wav` содержит контрольные точки восстановления. Сохраняются лимит 1 ГиБ и таймаут отсутствия звука 30 секунд.

Ранее отмеченные проблемы родных меню и режимов Yamaha вручную повторно не проверялись. Это обновление меняет подпись/ширину кнопки и документацию, а не движок рендеринга.
