@echo off
setlocal
set SRC=NJQCA.cpp
set OUT=NJQCA.exe
set "GPP="

:: ── Find g++ ──────────────────────────────────────────────────────
where g++ >nul 2>&1
if %errorlevel% equ 0 ( set "GPP=g++" & goto :build )
for %%L in (
    "C:\msys64\mingw64\bin\g++.exe"
    "C:\msys64\ucrt64\bin\g++.exe"
    "C:\msys2\mingw64\bin\g++.exe"
    "D:\msys64\mingw64\bin\g++.exe"
) do ( if exist %%L ( set "GPP=%%~L" & goto :build ) )
echo ERROR: g++ not found. & pause & exit /b 1

:build
echo.
echo Building %OUT% (dynamic linking) ...
echo Using: %GPP%
echo.

"%GPP%" %SRC% -o %OUT% ^
  -I./lib ^
  -std=c++17 -O2 ^
  -lcurl ^
  -lsqlite3 ^
  -lws2_32 -lwinhttp -lcrypt32 ^
  -lole32 -loleaut32 -lwbemuuid -lversion ^
  -static-libgcc -static-libstdc++

if %errorlevel% neq 0 (
    echo BUILD FAILED. & pause & exit /b 1
)

echo.
echo BUILD SUCCESS: %OUT%
echo Now run fix_dlls.sh in MSYS2 MINGW64 shell to fix DLLs.
echo.
pause
endlocal
