"""Build the English player with its native directory seeker and audio decoder."""
import argparse
import json
from pathlib import Path
import shutil
import subprocess

import patch

ROOT = Path(__file__).resolve().parent


def build(source, output, compiler):
    compiler = str(Path(compiler).resolve()) if Path(compiler).is_file() else shutil.which(compiler)
    if not compiler:
        raise ValueError('A 32-bit MinGW C++ compiler is required (--compiler i686-w64-mingw32-g++).')
    machine = subprocess.check_output([compiler, '-dumpmachine'], text=True).strip()
    if not machine.startswith(('i686-', 'i386-')):
        raise ValueError('The Yamaha player needs an x86 compiler, not an x64 compiler.')
    if ('L"' + patch.VERSION + '"') not in (ROOT / 'native/app_version.h').read_text(encoding='utf-8'):
        raise ValueError('The issue reporter version does not match the package version.')
    patch.build(source, output)
    output = Path(output).resolve()
    executable = (output / 'MidRadio-English.exe').read_bytes()
    if executable.count(b'MRG3_') != 10:
        raise ValueError('Unexpected player window-class layout.')
    (output / 'MidRadio-Directory.exe').write_bytes(executable.replace(b'MRG3_', b'MR10_'))
    shutil.copy2(output / 'PlayerCore.dll', output / 'CoreOriginal.dll')
    compile_native(output, compiler)
    launcher = ROOT / 'native' / 'prebuilt' / 'MidRadio.exe'
    launcher_info = json.loads((launcher.parent / 'launcher.json').read_text(encoding='utf-8'))
    if patch.digest(launcher) != launcher_info['sha256']:
        raise ValueError('The supplied C launcher failed its checksum check.')
    shutil.copy2(launcher, output / 'MidRadio.exe')
    identity = {
        'version': patch.VERSION,
        'player_version': '7.2.1.1',
        'target': 'Windows 10 x64 (x86 player under WoW64)',
        'launcher': 'MidRadio-Directory.exe',
        'features': ['English resources', 'embedded directory seeker', 'English skin labels', 'OGG Vorbis', 'M4A via Media Foundation', 'integrated real-time Yamaha PCM WAV export'],
        'files': {},
    }
    for path in output.rglob('*'):
        if not path.is_file() or path.name == 'MidRadio-English.exe':
            continue
        if path.suffix.lower() in ('.exe', '.dll', '.ocx', '.tbl'):
            identity['files'][path.relative_to(output).as_posix()] = patch.digest(path)
    (output / 'directory-build.json').write_text(json.dumps(identity, indent=2) + '\n', encoding='utf-8')
    print('Directory build ready:', output / identity['launcher'])


def compile_native(output, compiler):
    native = ROOT / 'native'
    common = [compiler, '-finput-charset=UTF-8', '-fexec-charset=UTF-8', '-D_WIN32_WINNT=0x0601', '-std=c++17', '-O2', '-static', '-static-libgcc', '-static-libstdc++']
    subprocess.run(common + ['-shared', '-o', str(output / 'PlayerCore.dll')] +
                   [str(native / name) for name in ('core_proxy.cpp', 'directory_ui.cpp', 'skin_english.cpp', 'media_prepare.cpp', 'wav_export.cpp', 'core_proxy.def')] +
                   ['-lcomctl32', '-lwininet', '-lshlwapi', '-lgdi32', '-luser32', '-lshell32', '-lcomdlg32'], check=True)
    subprocess.run(common + ['-municode', '-mwindows', '-o', str(output / 'MidRadioReport.exe'), str(native / 'issue_report.cpp'),
                            '-lshell32', '-lgdi32', '-luser32'], check=True)
    subprocess.run(common + ['-municode', '-o', str(output / 'MidRadioDecode.exe'), str(native / 'codec_decoder.cpp'),
                            '-lmfplat', '-lmfreadwrite', '-lmfuuid', '-lole32'], check=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=Path(r'C:\Program Files (x86)\YAMAHA\MidRadio Player'))
    parser.add_argument('--output', type=Path, default=ROOT / 'MidRadio-Directory')
    parser.add_argument('--compiler', default='i686-w64-mingw32-g++')
    args = parser.parse_args()
    try:
        build(args.source, args.output, args.compiler)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        parser.exit(1, f'Build failed: {error}\n')
