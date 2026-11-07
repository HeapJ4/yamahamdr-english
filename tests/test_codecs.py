"""Runs against a built decoder on Windows; no Yamaha binaries are required."""
import os
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest
import wave

FIXTURES = Path(__file__).parent / 'fixtures'
DECODER = os.environ.get('MIDRADIO_DECODER')


@unittest.skipUnless(DECODER and os.name == 'nt', 'Set MIDRADIO_DECODER to a built Windows decoder')
class CodecTests(unittest.TestCase):
    def decode(self, source, output):
        return subprocess.run([str(Path(DECODER).resolve()), str(source.resolve()), str(output.resolve())], capture_output=True, timeout=30)

    def test_vorbis_decodes_to_non_silent_pcm(self):
        with tempfile.TemporaryDirectory() as temp:
            output = Path(temp) / 'decoded.wav'
            self.assertEqual(self.decode(FIXTURES / 'tone.ogg', output).returncode, 0)
            with wave.open(str(output), 'rb') as audio:
                self.assertEqual((audio.getnchannels(), audio.getsampwidth(), audio.getframerate()), (2, 2, 44100))
                self.assertEqual(audio.getnframes(), 44100)
                pcm = audio.readframes(44100)
            samples = struct.unpack('<' + 'h' * (len(pcm) // 2), pcm)
            rms = (sum(value * value for value in samples) / len(samples)) ** 0.5
            self.assertGreater(rms, 1800)
            self.assertLess(rms, 2400)

    def test_malformed_ogg_rejected_without_output(self):
        with tempfile.TemporaryDirectory() as temp:
            bad, output = Path(temp) / 'bad.ogg', Path(temp) / 'bad.wav'
            bad.write_bytes(b'not valid Ogg data')
            self.assertNotEqual(self.decode(bad, output).returncode, 0)
            self.assertFalse(output.exists())

    @unittest.skipUnless(os.environ.get('MIDRADIO_TEST_MEDIA_FOUNDATION'), 'Optional host codec integration test')
    def test_windows_media_foundation_formats(self):
        with tempfile.TemporaryDirectory() as temp:
            for extension in ('m4a', 'mp3', 'wav'):
                with self.subTest(extension=extension):
                    output = Path(temp) / (extension + '.wav')
                    self.assertEqual(self.decode(FIXTURES / ('tone.' + extension), output).returncode, 0)
                    with wave.open(str(output), 'rb') as audio:
                        self.assertEqual(audio.getsampwidth(), 2)
                        self.assertGreaterEqual(audio.getnframes(), 40000)


if __name__ == '__main__':
    unittest.main()
