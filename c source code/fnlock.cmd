@echo off
echo ================= Fn Lock Test =================
echo Step 1: Please ENABLE Fn Lock (Fn key locked).
pause

echo.
echo Now press F1 only...
powershell -Command "$key1 = $Host.UI.RawUI.ReadKey('NoEcho,IncludeKeyDown'); $key1.VirtualKeyCode | Out-File -Encoding ASCII -NoNewline key1.txt"

echo.
echo Step 2: Please DISABLE Fn Lock.
pause

echo.
echo Now press F1 again...
powershell -Command "$key2 = $Host.UI.RawUI.ReadKey('NoEcho,IncludeKeyDown'); $key2.VirtualKeyCode | Out-File -Encoding ASCII -NoNewline key2.txt"

:: Read values from files
set /p key1=<key1.txt
set /p key2=<key2.txt

echo.
echo You pressed F1 with Fn Lock ON:  %key1%
echo You pressed F1 with Fn Lock OFF: %key2%
echo.

:: Interpret the result
if "%key1%"=="112" (
    if not "%key2%"=="112" (
        echo ✅ Detected: Fn Lock was ON in first test, OFF in second. Fn Lock toggle works.
    ) else (
        echo ℹ️ Detected: Fn Lock is likely still ON for both tests.
    )
) else if "%key1%" NEQ "112" (
    if "%key2%"=="112" (
        echo ⚠️ Unusual pattern. Possibly reversed behavior or Fn key required manually.
    ) else (
        echo ❌ Neither F1 detected — Fn key may be intercepting or media key used.
    )
)

echo =================================================
del key1.txt key2.txt >nul 2>&1
pause
