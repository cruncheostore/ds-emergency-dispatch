@echo off
cd /d %~dp0
echo Closing any running instances...
taskkill /F /IM erds.exe >nul 2>&1
echo Building ERDS...
g++ -o erds.exe main.cpp
if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    pause
    exit /b 1
)
echo Done. Launching...
start "" erds.exe
