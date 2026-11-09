[English](#english) · [Русский](#russian)

# Follow-up diagnosis — 2026-11-08

**Update:** MMF/MIDI WAV export is implemented and live-tested in 1.0; the diagnosis below describes the earlier 0.2.1 state. / **Обновление:** экспорт реализован и проверен в 1.0; ниже описано прежнее состояние 0.2.1.

<a name="english"></a>

## English

The screenshot shows crowded/clipped Track Details lettering. Version 0.2.0 gave the overlay 49 pixels, then reserved 3 pixels of padding, leaving 46 pixels for a label whose original Tahoma font measures 54 pixels on the development system. Integer negative character spacing squeezed the text, and the underlying bitmap hook also drew another English copy.

The fix uses a 61-pixel overlay, 9 pixels of left inset (six more than before), and an uncompressed 8-pixel Arial label measured at 45 pixels. The underlying sprite text is cleared while its icon is preserved. A raster test checks the actual glyph bounds and that the overlay stops before the sample-rate area. Live appearance on the reporter's separate machine still needs confirmation.

Two additional source-level playback issues were reproduced in the native media-preparation harness: a percent-encoded dot could prevent an OGG URL from selecting the decoder, and a truncated generated WAV could be accepted as a valid cache hit. The regression tests now exercise both cases. New downloads also use a partial file followed by atomic finalization.

The screenshots' MMF/GM-MIDI-to-WAV discussion is a feature request. The existing helper decodes Vorbis and Windows-supported compressed audio; it has no MIDI/SMAF synthesizer or export UI. A future exporter needs to render Yamaha's synthesis output, including its instrument settings and timing, to PCM.

The inspected Yamaha binaries have ordinary PE sections (`.text`, `.rdata`, `.data`, `.rsrc`, and in DLLs `.reloc`), readable resources, and no literal Themida marker in the checked files. This is not proof that every component is unprotected; no unpacker was required for the changes in this project.

<a name="russian"></a>

## Русский

На снимке надпись Track Details выглядит обрезанной и сжатой. В версии 0.2.0 ширина окна надписи составляла 49 пикселей; после отступа оставалось 46, тогда как исходный шрифт Tahoma на машине разработки занимает 54 пикселя. Отрицательное межбуквенное расстояние сжимало текст, а обработчик растрового изображения рисовал ещё одну английскую копию под ним.

Исправление: ширина 61 пиксель, левый отступ 9 пикселей (ещё на шесть вправо) и несжатый Arial высотой 8 пикселей, занимающий 45 пикселей по ширине. Текст исходного спрайта очищается, значок сохраняется. Растровый тест проверяет границы букв и отсутствие перекрытия поля частоты. Вид на компьютере автора снимка ещё нужно подтвердить.

В тесте подготовки аудио также воспроизведены две проблемы: процентное кодирование точки мешало распознаванию OGG, а неполный WAV мог приниматься за готовый кеш. Добавлены регрессионные проверки и атомарное завершение новых загрузок.

MMF/GM-MIDI → WAV на снимках — запрос новой функции. Текущий помощник не содержит синтезатора MIDI/SMAF или интерфейса экспорта. Будущий конвертер должен записывать результат синтеза Yamaha в PCM с учётом инструментов, настроек и времени событий.

Проверенные файлы Yamaha имеют обычные секции PE, читаемые ресурсы и не содержат буквального маркера Themida. Это не доказывает отсутствие любой защиты во всех компонентах; для внесённых изменений распаковщик не понадобился.
