[English](#english) · [Русский](#russian)

# Integrated Yamaha WAV rendering

<a name="english"></a>

## English

The directory pane's MIDI-WAV EXPORT action prepares the selected file, arms the recorder, and opens the track through the original player's loader. The recorder hooks `waveOutOpen`, `waveOutWrite`, `waveOutReset`, `waveOutClose`, and `waveOutRestart` in this process's Yamaha WaveOut, MidiOut, and Smaf modules. SMAF uses its own output route, so hooking only WaveOut is insufficient.

PCM is snapshotted before submission, then recorded only after the real waveOutWrite succeeds. The recorder waits for a fresh output start/reset/restart so it does not capture the previous track's queued tail. It selects one audio device, preserves the actual PCM format, and writes/finalizes a standard RIFF/WAVE header. Finish export, output reset, and device close finalize recording. A no-audio attempt is reported as failure rather than producing a pretend conversion.

This is real-time rendering with the built-in Yamaha software synthesizer. It is not offline accelerated conversion, external-hardware MIDI capture, microphone recording, or system-loopback audio. The limit is 1 GiB; supported PCM output is 8–32 bits, 1–8 channels, 8–192 kHz. Ordinary tested output is 44.1 kHz stereo 16-bit.

### Live verification

| Input | WAV duration | Rate / channels / precision | Signal RMS |
| --- | ---: | --- | ---: |
| Native MMF sample | 31.211 seconds | 44100 Hz / 2 / 16-bit | 12716.53 |
| Synthetic GM piano MIDI | 3.697 seconds | 44100 Hz / 2 / 16-bit | 389.71 |

Both files were created through the actual player UI, observed with Saved WAV status, then parsed as WAV and checked for non-silent PCM. The MIDI fixture is included in the source; the MMF recording is not distributed in the repository. Manual Finish export was exercised for MMF; natural output finalization was observed for MIDI.

<a name="russian"></a>

## Русский

Кнопка MIDI-WAV EXPORT подготавливает выбранный файл, включает запись и открывает композицию исходным загрузчиком Yamaha. Перехватываются функции waveOut в модулях WaveOut, MidiOut и Smaf только внутри процесса плеера. У SMAF отдельный путь вывода, поэтому одного WaveOut недостаточно.

PCM копируется до отправки устройству и записывается только после успешной отправки. Запись ожидает нового запуска/сброса, чтобы не захватить хвост прежней композиции, и выбирает одно устройство. Формат PCM сохраняется. Finish export, сброс и закрытие устройства завершают заголовок RIFF/WAVE. Пустой экспорт выдаёт ошибку, а не ложное сообщение об успешной конвертации.

Это рендеринг в реальном времени встроенным синтезатором Yamaha, а не ускоренный офлайн-конвертер, захват внешнего MIDI, микрофона или системного звука. Лимит — 1 ГиБ. Поддерживается PCM 8–32 бита, 1–8 каналов и 8–192 кГц; проверенный обычный вывод — стерео 44,1 кГц / 16 бит.

В реальном интерфейсе получены непустые WAV: MMF — 31,211 с, GM MIDI — 3,697 с. Оба файла проверены по заголовку и ненулевому сигналу. Для MMF проверена кнопка ручного завершения; для MIDI — автоматическое завершение вывода. Синтетический MIDI приложен к исходникам, запись MMF в репозитории не распространяется.
