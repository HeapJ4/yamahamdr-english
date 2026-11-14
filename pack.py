"""Package the patch source and, optionally, a verified portable player."""
import argparse
import hashlib
import json
from pathlib import Path
import zipfile

ROOT = Path(__file__).resolve().parent
from patch import VERSION
SOURCE_FILES = [
    'README.md', 'LICENSE', 'NOTICE.md', 'THIRD_PARTY_NOTICES.md', 'CHANGELOG.md', 'CONTRIBUTING.md',
    'patch.py', 'pe_resources.py', 'translations.txt', 'supported-build.json', 'build_directory.py',
    'native/prebuilt/MidRadio.exe', 'native/prebuilt/launcher.json', 'pack.py', '.gitignore', '.gitattributes', 'SECURITY.md', '.github/CODEOWNERS',
    '.github/dependabot.yml', '.github/workflows/check.yml',
]
SOURCE_FILES += sorted(path.relative_to(ROOT).as_posix() for directory in ('native', 'tests', 'docs')
                       for path in (ROOT / directory).rglob('*')
                       if path.is_file() and path.suffix.lower() in ('.py', '.cpp', '.h', '.c', '.def', '.md', '.ogg', '.m4a', '.mp3', '.wav', '.mid'))
PORTABLE_README = """MidRadio English + Directory 1.0.1

Windows 10 x64 target; also used on Windows 11 x64.
Extract everything and run MidRadio.exe. No Python or installer needed.
Keep CoreOriginal.dll, PlayerCore.dll, MidRadioDecode.exe, and Addins together.

Use Open folder for an HTTP directory, double-click a folder to navigate,
Play selected for one track, Play all for the current list, and right-click
for Copy full URL. The default page is the Samsung SGH-D500 ringtone folder.
English menus and graphical labels preserve the original Yamaha skin.

Select an MMF/MIDI track and click MIDI-WAV EXPORT to render it in this same app.
Use Yamaha's built-in software synthesizer, let it play, and click Finish export.
Export is real-time, captures only this player's PCM, and is limited to 1 GiB.
Keep the app open until Saved WAV appears. Hardware MIDI output has no PCM.
Exports use a sibling .partial-...wav with recovery checkpoints. Existing files
are replaced only on successful completion. Compact modes hide the directory pane.

Native Yamaha files, WAV and MP3 use Yamaha's original engine.
OGG Vorbis and Windows-supported M4A audio use the separate bounded decoder.
These extended codecs apply to audio opened through the directory seeker.
Input tracks and converted PCM are cached in %TEMP%/MidRadio-Directory-cache.
You can remove that cache after closing the player.

Source: https://github.com/HeapJ4/yamahamdr-english
See RELEASE-NOTES.md, NOTICE.md, THIRD_PARTY_NOTICES.md, and LICENSE.
"""


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def archive(path, prefix, files, extra=None):
    if path.exists():
        raise ValueError(f'Refusing to overwrite archive: {path}')
    with zipfile.ZipFile(path, 'x', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as output:
        for name, file in sorted(files):
            info = zipfile.ZipInfo(prefix + '/' + name, date_time=(2026, 11, 7, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = 0o100644 << 16
            output.writestr(info, file.read_bytes())
        for name, text in sorted((extra or {}).items()):
            info = zipfile.ZipInfo(prefix + '/' + name, date_time=(2026, 11, 7, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = 0o100644 << 16
            output.writestr(info, text.encode('utf-8'))
    with zipfile.ZipFile(path) as output:
        bad = output.testzip()
        if bad:
            raise ValueError(f'Archive CRC check failed: {bad}')
    checksum = sha256(path)
    path.with_suffix(path.suffix + '.sha256').write_text(f'{checksum}  {path.name}\n', encoding='ascii')
    print(f'{path.name}: {path.stat().st_size:,} bytes | SHA-256 {checksum}')


def package(destination, portable=None):
    destination.mkdir(parents=True, exist_ok=True)
    prefix = 'yamahamdr-english-v' + VERSION
    archive(destination / (prefix + '-source.zip'), prefix, [(name, ROOT / name) for name in SOURCE_FILES])
    if portable is None:
        return
    portable = portable.resolve()
    if (portable / 'BUILD-FAILED.txt').exists():
        raise ValueError('Refusing to package an incomplete player build')
    report = json.loads((portable / 'directory-build.json').read_text(encoding='utf-8'))
    if report['version'] != VERSION or report['launcher'] != 'MidRadio-Directory.exe':
        raise ValueError('This is not the intended English + Directory build')
    required = {'MidRadio.exe', 'MidRadio-Directory.exe', 'MidRadioDecode.exe', 'PlayerCore.dll', 'CoreOriginal.dll'}
    if not required.issubset(report['files']) or 'Addins/OggFormat.dll' in report['files']:
        raise ValueError('Wrong or experimental binary set')
    files = []
    for name, expected in report['files'].items():
        relative = Path(name)
        if relative.is_absolute() or '..' in relative.parts:
            raise ValueError('Unsafe file name in build report')
        file = portable / relative
        if file.is_symlink() or sha256(file) != expected:
            raise ValueError('Portable file failed verification: ' + name)
        files.append((name, file))
    files.append(('directory-build.json', portable / 'directory-build.json'))
    for name in ('NOTICE.md', 'THIRD_PARTY_NOTICES.md', 'LICENSE'):
        files.append((name, ROOT / name))
    files.append(('RELEASE-NOTES.md', ROOT / 'docs' / ('release-v' + VERSION + '.md')))
    archive(destination / (prefix + '-windows-x64.zip'), 'MidRadio-English-Directory', files, {'README.txt': PORTABLE_README})


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--portable', type=Path, help='Verified local English player folder')
    parser.add_argument('--output', type=Path, default=ROOT / 'dist')
    args = parser.parse_args()
    try:
        package(args.output, args.portable)
    except (ValueError, OSError, KeyError) as error:
        parser.exit(1, f'Error: {error}\n')
