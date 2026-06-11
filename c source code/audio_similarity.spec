# -*- mode: python ; coding: utf-8 -*-
from PyInstaller.utils.hooks import collect_submodules, collect_data_files

scipy_hidden = collect_submodules('scipy')
scipy_data   = collect_data_files('scipy')

pandas_hidden = collect_submodules('pandas')
pandas_data   = collect_data_files('pandas')

a = Analysis(
    ['audio_similarity.py'],
    pathex=[],
    binaries=[],
    datas=scipy_data + pandas_data,
    hiddenimports=scipy_hidden + pandas_hidden,
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='audio_similarity',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=True,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)