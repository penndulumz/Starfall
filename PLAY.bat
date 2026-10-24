@echo off
setlocal
cd /d "%~dp0"
if not exist "%~dp0dist\PLAY.bat" (
    powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\Build.ps1"
    if errorlevel 1 (
        pause
        exit /b 1
    )
)
call "%~dp0dist\PLAY.bat" %*
exit /b %errorlevel%
