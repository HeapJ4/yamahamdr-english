"""Build an English copy of the supported MidRadio Player installation."""
import argparse
import ctypes
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import struct
import sys

from pe_resources import resources, text_fields

HERE = Path(__file__).resolve().parent
VERSION = '0.2.0'


def load_translations(path=HERE / 'translations.txt'):
    result = {}
    for number, line in enumerate(path.read_text(encoding='utf-8').splitlines(), 1):
        if not line or line.startswith('#'):
            continue
        if '|' not in line:
            raise ValueError(f'Invalid translation on line {number}')
        source, target = (s.replace('\\n', '\n') for s in line.split('|', 1))
        if source in result:
            raise ValueError(f'Duplicate translation on line {number}: {source!r}')
        result[source] = target
    return result


def translate(text, mapping):
    if text in mapping:
        return mapping[text]
    label, separator, shortcut = text.partition('\t')
    return mapping.get(label, label) + separator + shortcut


def placeholders(text):
    return re.findall(r'%(?:\d+(?:![^!]+!)?|[-+ #0]*\d*(?:\.\d+)?(?:hh|ll|[hlLzjt])?[diuoxXfFeEgGaAcspn])', text.replace('%%', ''))


def transform(data, kind, mapping):
    padding = []
    fields = text_fields(data, kind, padding)
    edits, changes, expected = [], [], []
    for field in fields:
        old = field['text']
        structural = kind == 5 and field['label'] in ('font', 'control-class', 'class', 'menu')
        new = old if structural else translate(old, mapping)
        expected.append(new)
        if old == new:
            continue
        if placeholders(old) != placeholders(new):
            raise ValueError(f'Format placeholders changed: {old!r} -> {new!r}')
        encoded = new.encode('utf-16le')
        replacement = struct.pack('<H', len(encoded) // 2) + encoded if field.get('counted') else encoded + b'\0\0'
        edits.append((field['offset'], field['end'], replacement))
        changes.append((old, new))
    if not changes:
        return data, []
    edits += [(start, end, None) for start, end in padding]
    edits.sort(key=lambda edit: (edit[0], 0 if edit[2] is None else 1))
    output, position = bytearray(), 0
    for start, end, replacement in edits:
        if start < position:
            raise ValueError('Overlapping resource edits')
        output += data[position:start]
        output += b'\0' * (-len(output) % 4) if replacement is None else replacement
        position = end
    output += data[position:]
    if [field['text'] for field in text_fields(bytes(output), kind)] != expected:
        raise ValueError('Resource round-trip verification failed')
    return bytes(output), changes


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def section_hashes(path):
    data = path.read_bytes()
    pe = struct.unpack_from('<I', data, 60)[0]
    count = struct.unpack_from('<H', data, pe + 6)[0]
    size = struct.unpack_from('<H', data, pe + 20)[0]
    start = pe + 24 + size
    result = {}
    for i in range(count):
        name, _, _, length, offset = struct.unpack_from('<8sIIII', data, start + i * 40)
        if name.rstrip(b'\0') != b'.rsrc':
            result[name.decode('ascii').rstrip('\0')] = hashlib.sha256(data[offset:offset + length]).hexdigest()
    return result


def write_resources(path, updates):
    if os.name != 'nt':
        raise OSError('Building patched Windows resources requires Windows.')
    api = ctypes.WinDLL('kernel32', use_last_error=True)
    api.BeginUpdateResourceW.argtypes = [ctypes.c_wchar_p, ctypes.c_bool]
    api.BeginUpdateResourceW.restype = ctypes.c_void_p
    api.UpdateResourceW.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_ushort, ctypes.c_void_p, ctypes.c_uint]
    api.UpdateResourceW.restype = ctypes.c_bool
    api.EndUpdateResourceW.argtypes = [ctypes.c_void_p, ctypes.c_bool]
    api.EndUpdateResourceW.restype = ctypes.c_bool

    def identifier(value):
        return ctypes.c_void_p(value) if isinstance(value, int) else ctypes.cast(ctypes.c_wchar_p(value), ctypes.c_void_p)

    handle = api.BeginUpdateResourceW(str(path), False)
    if not handle:
        raise ctypes.WinError(ctypes.get_last_error())
    try:
        for keys, data, _ in updates:
            buffer = ctypes.create_string_buffer(data)
            if not api.UpdateResourceW(handle, identifier(keys[0]), identifier(keys[1]), keys[2], buffer, len(data)):
                raise ctypes.WinError(ctypes.get_last_error())
        active, handle = handle, None
        if not api.EndUpdateResourceW(active, False):
            raise ctypes.WinError(ctypes.get_last_error())
    finally:
        if handle:
            api.EndUpdateResourceW(handle, True)


def validate_source(source, manifest):
    for entry in manifest['files']:
        path = source / entry['file']
        if not path.is_file() or digest(path) != entry['source_sha256']:
            raise ValueError(f'Unsupported or modified file: {entry["file"]}. Use an original MidRadio Player {manifest["player_version"]} installation.')


def build(source, output, dry_run=False):
    source, output = Path(source).resolve(), Path(output).resolve()
    if output == source or source in output.parents or output in source.parents:
        raise ValueError('Choose an output folder separate from the installed player.')
    if output.exists():
        raise ValueError('Output folder already exists. Choose a new folder.')
    manifest = json.loads((HERE / 'supported-build.json').read_text(encoding='utf-8'))
    mapping = load_translations()
    validate_source(source, manifest)
    plans = []
    for entry in manifest['files']:
        path = source / entry['file']
        updates = []
        for keys, data, _ in resources(path):
            if keys[0] not in (4, 5, 6):
                continue
            patched, changes = transform(data, keys[0], mapping)
            if changes:
                updates.append((keys, patched, changes))
        count = sum(len(changes) for _, _, changes in updates)
        if count != entry['fields']:
            raise ValueError(f'Unexpected translation coverage for {entry["file"]}: {count}')
        plans.append((entry, updates))
    total = sum(entry['fields'] for entry, _ in plans)
    if dry_run:
        print(f'Validated: {total} translations across {len(plans)} files. No files written.')
        return None
    if os.name != 'nt':
        raise OSError('Building the English player requires Windows.')
    shutil.copytree(source, output)
    report = {'patch_version': VERSION, 'player_version': manifest['player_version'], 'translated_fields': total, 'files': []}
    try:
        for entry, updates in plans:
            target = output / entry['file']
            before = section_hashes(source / entry['file'])
            write_resources(target, updates)
            if before != section_hashes(target):
                raise ValueError(f'Non-resource section changed: {entry["file"]}')
            actual = {tuple(keys): data for keys, data, _ in resources(target)}
            if any(actual.get(tuple(keys)) != data for keys, data, _ in updates):
                raise ValueError(f'Written resource verification failed: {entry["file"]}')
            report['files'].append({**entry, 'patched_sha256': digest(target), 'non_resource_sections_unchanged': True})
            print(f'{entry["file"]}: {entry["fields"]} translated fields')
        shutil.copy2(output / 'MidRadio.exe', output / 'MidRadio-English.exe')
        (output / 'english-build.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
        print(f'Built {total} translations. Launch: {output / "MidRadio-English.exe"}')
        return report
    except Exception:
        # Keep the incomplete copy for diagnosis; never alter or delete the source.
        (output / 'BUILD-FAILED.txt').write_text('This build is incomplete. Do not run it. Rebuild into a new folder.\n', encoding='utf-8')
        raise


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--version', action='version', version=VERSION)
    parser.add_argument('--source', type=Path, default=Path(r'C:\Program Files (x86)\YAMAHA\MidRadio Player'))
    parser.add_argument('--output', type=Path, default=Path.cwd() / 'MidRadio-English')
    parser.add_argument('--dry-run', action='store_true', help='Verify hashes and translation coverage without writing files')
    args = parser.parse_args()
    try:
        build(args.source, args.output, args.dry_run)
    except (OSError, ValueError, struct.error, KeyError) as error:
        parser.exit(1, f'Error: {error}\n')


if __name__ == '__main__':
    main()
