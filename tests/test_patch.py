import struct
import tempfile
from pathlib import Path
import unittest

import patch
from pe_resources import text_fields


def wide(value):
    return value.encode('utf-16le') + b'\0\0'


def strings(values):
    return b''.join(struct.pack('<H', len(value.encode('utf-16le')) // 2) + value.encode('utf-16le') for value in values + [''] * (16 - len(values)))


def dialog(extended):
    style = 64  # DS_SETFONT
    if extended:
        data = struct.pack('<HHIIIHhhhh', 1, 65535, 0, 0, style, 2, 0, 0, 120, 70)
    else:
        data = struct.pack('<IIHhhhh', style, 0, 2, 0, 0, 120, 70)
    data += b'\0\0\0\0' + wide('設定')
    data += (struct.pack('<HHBB', 9, 400, 0, 1) if extended else struct.pack('<H', 9)) + wide('ＭＳ Ｐゴシック')
    for index, title in enumerate(('閉じる', 'キャンセル')):
        data += b'\0' * (-len(data) % 4)
        if extended:
            data += struct.pack('<IIIhhhhI', 0, 0, 0, 0, 0, 40, 12, 100 + index)
        else:
            data += struct.pack('<IIhhhhH', 0, 0, 0, 0, 40, 12, 100 + index)
        data += b'\xff\xff\x80\0' + wide(title) + b'\0\0'
    return data


class PatchTests(unittest.TestCase):
    def setUp(self):
        self.mapping = patch.load_translations()

    def test_string_table_preserves_empty_slots(self):
        data = strings(['', '再生', '', '停止'])
        output, changes = patch.transform(data, 6, self.mapping)
        fields = text_fields(output, 6)
        self.assertEqual([(f['label'], f['text']) for f in fields], [('1', 'Play'), ('3', 'Stop')])
        self.assertEqual(len(changes), 2)

    def test_dialog_alignment_and_font_preservation(self):
        for extended in (False, True):
            with self.subTest(extended=extended):
                output, changes = patch.transform(dialog(extended), 5, self.mapping)
                fields = text_fields(output, 5)
                self.assertEqual([f['text'] for f in fields], ['Settings', 'ＭＳ Ｐゴシック', 'Close', 'Cancel'])
                self.assertEqual(len(changes), 3)

    def test_menu_recursion_and_shortcuts(self):
        data = struct.pack('<HHH', 0, 0, 0x90) + wide('ファイル(&F)')
        data += struct.pack('<HH', 0x80, 10) + wide('ファイルを開く(&O)...\tCtrl+O')
        output, _ = patch.transform(data, 4, self.mapping)
        self.assertEqual([f['text'] for f in text_fields(output, 4)], ['&File', '&Open File...\tCtrl+O'])

    def test_placeholder_mismatch_rejected(self):
        with self.assertRaises(ValueError):
            patch.transform(strings(['Count %ld']), 6, {'Count %ld': 'Count %d'})

    def test_format_message_and_printf(self):
        self.assertEqual(patch.placeholders('%1!s! %2 %ld %% %s'), ['%1!s!', '%2', '%ld', '%s'])

    def test_unchanged_resources_are_identical(self):
        data = strings(['English'])
        self.assertEqual(patch.transform(data, 6, self.mapping), (data, []))

    def test_modified_source_rejected(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / 'MidRadio.exe').write_bytes(b'not the original')
            with self.assertRaisesRegex(ValueError, 'Unsupported or modified'):
                patch.validate_source(root, {'player_version': '7.2.1.1', 'files': [{'file': 'MidRadio.exe', 'source_sha256': '0' * 64}]})

    def test_existing_and_nested_output_rejected(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            with self.assertRaisesRegex(ValueError, 'separate'):
                patch.build(root, root / 'output', True)
            (root / 'existing').mkdir()
            with self.assertRaisesRegex(ValueError, 'already exists'):
                patch.build(root / 'source', root / 'existing', True)


if __name__ == '__main__':
    unittest.main()
