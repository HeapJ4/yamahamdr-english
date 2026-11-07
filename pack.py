"""Package the patch source and, optionally, a verified portable player."""
import argparse
import hashlib
import json
from pathlib import Path
import zipfile

ROOT = Path(__file__).resolve().parent
VERSION = '0.1.0-preview'
SOURCE_FILES = [
    'README.md', 'LICENSE', 'NOTICE.md', 'CHANGELOG.md', 'CONTRIBUTING.md',
    'patch.py', 'pe_resources.py', 'translations.txt', 'supported-build.json',
    'pack.py', '.gitignore', 'SECURITY.md', '.github/CODEOWNERS',
    '.github/dependabot.yml', '.github/workflows/check.yml',
    'docs/findings.md', 'docs/release-v0.1.0-preview.md', 'tests/test_patch.py',
]
PORTABLE_README = '''MidRadio English v0.1.0-preview

Target: Windows 10 x64. User-confirmed working on Windows 11 x64.
The original player is a 32-bit application running under WoW64.

1. Close other copies of MidRadio Player.
2. Extract this entire ZIP. Keep the Addins folder and all DLL/TBL files.
3. Launch MidRadio-English.exe from the extracted folder.
4. Use File > Open File to open a local music file.

No Python or installer is needed to launch this folder package.
The original application may store preferences in the Windows registry.
Some skin labels remain Japanese because they are embedded in bitmap images.
Clean Windows 10 first-run dependencies have not been independently tested.

Project and source: https://github.com/HeapJ4/yamahamdr-english
Yamaha's original software/assets retain their original ownership and terms.
See NOTICE.md and RELEASE-NOTES.md.
'''


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def archive(path, prefix, files, extra=None):
    if path.exists():
        raise ValueError(f'Refusing to overwrite archive: {path}')
    with zipfile.ZipFile(path, 'x', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as output:
        for name, file in sorted(files):
            info = zipfile.ZipInfo(prefix + '/' + name, date_time=(2026, 10, 23, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = 0o100644 << 16
            output.writestr(info, file.read_bytes())
        for name, text in sorted((extra or {}).items()):
            info = zipfile.ZipInfo(prefix + '/' + name, date_time=(2026, 10, 23, 0, 0, 0))
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
    report = json.loads((portable / 'english-build.json').read_text(encoding='utf-8'))
    expected = json.loads((ROOT / 'supported-build.json').read_text(encoding='utf-8'))
    if report['translated_fields'] != expected['translated_fields'] or len(report['files']) != len(expected['files']):
        raise ValueError('Unexpected player build coverage')
    expected_files = {entry['file']: entry for entry in expected['files']}
    for entry in report['files']:
        name = entry['file'].replace('\\', '/')
        if name not in expected_files or entry['source_sha256'] != expected_files[name]['source_sha256']:
            raise ValueError(f'Unexpected source build: {name}')
        if sha256(portable / name) != entry['patched_sha256'] or not entry['non_resource_sections_unchanged']:
            raise ValueError(f'Portable file failed verification: {name}')
    if sha256(portable / 'MidRadio-English.exe') != sha256(portable / 'MidRadio.exe'):
        raise ValueError('The explicit English launcher does not match the patched executable')
    files = []
    for path in portable.rglob('*'):
        relative = path.relative_to(portable)
        if not path.is_file() or path.is_symlink():
            continue
        if relative.parent.as_posix() not in ('.', 'Addins'):
            continue
        if path.suffix.lower() in ('.exe', '.dll', '.ocx', '.tbl') or relative.as_posix() == 'english-build.json':
            files.append((relative.as_posix(), path))
    for name in ('NOTICE.md',):
        files.append((name, ROOT / name))
    files.append(('RELEASE-NOTES.md', ROOT / 'docs/release-v0.1.0-preview.md'))
    archive(destination / (prefix + '-windows-x64.zip'), 'MidRadio-English', files, {'README.txt': PORTABLE_README})


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--portable', type=Path, help='Verified local English player folder')
    parser.add_argument('--output', type=Path, default=ROOT / 'dist')
    args = parser.parse_args()
    try:
        package(args.output, args.portable)
    except (ValueError, OSError, KeyError) as error:
        parser.exit(1, f'Error: {error}\n')
