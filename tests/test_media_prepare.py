"""HTTP/cache regressions using the native harness and synthetic audio."""
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import threading
import unittest


@unittest.skipUnless(os.name == 'nt' and os.environ.get('MIDRADIO_MEDIA_TEST'), 'Set MIDRADIO_MEDIA_TEST to the native cache harness')
class MediaPreparationTests(unittest.TestCase):
    def test_encoded_extension_and_interrupted_cache_recovery(self):
        fixtures = Path(__file__).parent / 'fixtures'
        with tempfile.TemporaryDirectory() as folder:
            directory = Path(folder)
            for path in fixtures.iterdir():
                shutil.copy2(path, directory / path.name)
            (directory / 'tone.mmf').write_bytes(b'MMMDsynthetic-byte-preservation-fixture')
            (directory / 'broken.ogg').write_bytes(b'not valid Ogg data')

            class Handler(SimpleHTTPRequestHandler):
                def log_message(self, *args):
                    pass

            server = ThreadingHTTPServer(('127.0.0.1', 0), partial(Handler, directory=folder))
            threading.Thread(target=server.serve_forever, daemon=True).start()
            try:
                harness = Path(os.environ['MIDRADIO_MEDIA_TEST']).resolve()
                decoder = Path(os.environ['MIDRADIO_DECODER']).resolve()
                command = [str(harness), str(decoder.parent), f'http://127.0.0.1:{server.server_port}']
                if not os.environ.get('MIDRADIO_TEST_MEDIA_FOUNDATION'):
                    command.append('--no-m4a')
                result = subprocess.run(command, capture_output=True, text=True, timeout=120)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertIn('cache recovery passed', result.stdout)
            finally:
                server.shutdown()
                server.server_close()
