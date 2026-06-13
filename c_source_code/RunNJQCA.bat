@echo off
echo Script is running from: %~dp0

:: Change to directory where this bat file lives
cd /d "%~dp0"
echo Changed to directory: %cd%

:: Relaunch as Administrator if not already
net session >nul 2>&1
if %errorlevel% neq 0 (
    powershell -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
    exit /b
)

:: Run NJQCA.exe
echo Starting NJQCA.exe...
NJQCA.exe

pause