#!/bin/bash
# fix_dlls.sh
# Run this in MSYS2 MINGW64 shell AFTER compile.bat succeeds.
# Deletes old incompatible DLLs then copies correct MSYS2 versions.

DEPLOY=/c/WindowsNJQCA
TEMP=/tmp/njqca_dll_check

echo "=== Step 1: Remove old incompatible DLLs ==="
cd "$DEPLOY"
rm -fv libcurl*.dll libssl*.dll libcrypto*.dll \
       libnghttp*.dll libngtcp*.dll \
       libbrotli*.dll libpsl*.dll \
       libzstd*.dll libssh2*.dll \
       libidn*.dll zlib1.dll libunistring*.dll \
       libgnutls*.dll libp11*.dll libnettle*.dll \
       libhogweed*.dll libgmp*.dll

echo ""
echo "=== Step 2: Run ldd from clean temp folder ==="
mkdir -p "$TEMP"
cp "$DEPLOY/NJQCA.exe" "$TEMP/"
ldd "$TEMP/NJQCA.exe"

echo ""
echo "=== Step 3: Copy correct MSYS2 DLLs ==="
ldd "$TEMP/NJQCA.exe" | awk '$3 ~ /msys64/ {print $3}' | while read f; do
    cp -fv "$f" "$DEPLOY/"
done

echo ""
echo "=== Done. Try running NJQCA.exe now. ==="
rm -rf "$TEMP"
