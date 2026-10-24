@echo off
setlocal
if /I "%~1"=="--help" goto help
for %%F in ("Starfall.exe" "SDL2.dll" "SDL2_image.dll" "SDL2_ttf.dll" "SDL2_mixer.dll" "resources\reference\spaceShip\stitchedFiles\spaceships.png" "resources\reference\spaceShip\stitchedFiles\projectiles.png" "resources\reference\background1.png" "resources\reference\background2.png" "resources\reference\background3.png" "resources\reference\background4.png" "resources\reference\healthBar.png" "resources\reference\health.png" "resources\reference\CommodorePixeled.ttf" "resources\reference\shoot.flac") do (
    if not exist "%~dp0%%~F" (
        echo Starfall cannot start: %%~F is missing.
        echo Extract the complete matching portable ZIP into a new folder and try again.
        echo Keep all supplied DLLs and the resources folder beside Starfall.exe.
        if /I not "%~1"=="--check" pause
        exit /b 1
    )
)
if /I "%~1"=="--check" (
    echo Starfall package files are present. See READ-ME-FIRST.txt to play.
    exit /b 0
)
start "" /d "%~dp0" "%~dp0Starfall.exe" %*
exit /b 0
:help
echo STARFALL - portable Windows x64 game
echo Double-click PLAY.bat or Starfall.exe. No compiler or download is required.
echo PLAY.bat --data-dir "C:\My Scores" chooses a separate writable save folder.
echo PLAY.bat --check checks required filenames without starting the game.
echo Keep the complete package together. See READ-ME-FIRST.txt and docs\USER.md.
exit /b 0
