"""Run with Python 3 and Clang. The host shim supplies UEFI types and services."""
import os
from pathlib import Path
import re
import subprocess
import tempfile

here = Path(__file__).resolve().parent
root = here.parents[1]
menu = (root / 'Main/menu.c').read_text()
config = (root / 'Main/config.c').read_text()
header = (root / 'Main/menu.h').read_text()


def function(source, name):
    pattern = rf'^(?:static\n)?\w+[ *]+{name} \(.*?^\}}'
    return re.search(pattern, source, re.M | re.S)[0] + '\n'


definitions = header[header.index('typedef struct {', header.index('// Scrolling definitions')):
                     header.index('#define INPUT_KEY')]
constants = '\n'.join(re.findall(r'^#define (?:TILE_[XY]SPACING|MIN_TOOL_ROW_GAP)\s+\(\d+\)', menu, re.M))
functions = ['AddPixels', 'MinimumToolGap', 'PlaceToolsAbove', 'ComputeRow0PosY', 'ComputeToolRowPosY',
             'RowStartX', 'InitScroll', 'AdjustScrollState', 'UpdateScroll', 'GetStateInfo',
             'IdentifyRows', 'FindMainMenuItem', 'GenerateWaitList', 'WaitForInput']
start = config.rindex('        else if (', 0, config.index('L"row_icons_move"'))
end = config.rindex('        else if (', 0, config.index('L"scan_delay"', start))
dispatch = 'static void parse(CHAR16 **TokenList, UINTN TokenCount) { if (0) {}\n' + config[start:end] + '}\n'
production = (definitions + constants + '\n' + function(config, 'KeepReading')
              + function(config, 'ReadTokenLine') + function(config, 'ParseRowOffset') + dispatch
              + ''.join(function(menu, name) for name in functions)
              + ''.join(function((root / 'libeg/image.c').read_text(), name)
                        for name in ['egCreateImage', 'egCropImage']))
with tempfile.TemporaryDirectory() as tmp:
    src = Path(tmp) / 'audit.c'
    src.write_text((here / 'shim.h').read_text() + production + (here / 'cases.c').read_text())
    cc = os.environ.get('CC', 'clang')
    subprocess.run([cc, '-DWORD_BITS=64', '-fshort-wchar', '--analyze', '-Xanalyzer', '-analyzer-output=text', str(src)], check=True)
    for bits in (32, 64):
        exe = Path(tmp) / 'audit'
        subprocess.run([cc, '-std=c11', '-fshort-wchar', '-Wall', '-Wextra', '-Werror', '-Wno-sign-compare',
                        '-fsanitize=address,undefined', '-fno-sanitize-recover=all', '-g', '-O1',
                        f'-DWORD_BITS={bits}', str(src), '-o', str(exe)], check=True)
        subprocess.run([str(exe)], check=True)
