# Start here: playing Starfall

You do not need to know programming to play this game. Extract **Starfall-Windows-x64.zip** for the compiled game. The separate **Starfall-Source.zip** contains rebuild inputs and needs the developer prerequisites before playing.

For setup, display/sound options, upgrades, and troubleshooting, use the [user guide](docs/USER.md). For controls, combat, the HUD, and scoring, use the [player guide](docs/PLAYER.md). The short walkthrough below gets you to your first run; build details are in the separate developer guide.

## 1. Open the game folder

Open File Explorer and go to:

```text
C:\Users\Admin\Desktop\spaceshooter-SDL2
```

Double-click **PLAY.bat**. Windows may display its name as just **PLAY** if file extensions are hidden. The welcome screen will open. Keep the staged package's `resources` folder and all supplied DLL files beside the executable; they provide the artwork, font, and runtime support.

You can also open the `dist` folder and double-click **Starfall.exe**. Keep all supplied DLL files and the `resources` folder in that same folder.

## 2. Start a mission

Click **Start Mission**, or press **Enter**.

Use **W, A, S, D** or the arrow keys to move. Point the mouse at an enemy and **hold the left mouse button** to fire. Hold **Space** as an alternative fire control. Press **Shift** to dash away from danger. Your shots use the pink reference sprite; enemy shots are cyan. Avoid enemy shots and enemy ships.

Your five hull segments are at the top left. Pick up cyan plus signs to repair damage. Gold pickups temporarily improve your weapon. The small map at the lower right shows where enemies are, including enemies outside the current view.

## 3. Pause, resize, or change sound

- **Escape** or **P** pauses. You can also click the Pause button.
- Switching to another window pauses automatically.
- **F11** switches between fullscreen and a window.
- **M** toggles sound. Every screen also has a sound button at the lower right.
- **How to Play** opens the flight manual from the welcome or pause screen.

## 4. Save your score

A run ends when your hull reaches zero, or when you clear the tenth wave and defeat the Dreadnought.

On the end screen:

1. Type your pilot name. Typing replaces the initially selected `PILOT` name.
2. Click **Save**, or press **Enter** while Save is selected.
3. Look for the saved rank. Scores below the top ten are marked **Outside the Top 10**.
4. Choose **Leaderboard**, **Play Again**, or **Main Menu**.

Save before leaving the end screen. A run is never saved twice. Scores stay on this computer after you close the game; no account or internet connection is needed.

The overhaul keeps your existing score file in `%APPDATA%\Starfall\Starfall\leaderboard.txt`. Replacing the portable game does not move or erase that file. If Save reports an error, leave the end screen open, close any editor or other program holding the score file, check that the save folder is writable, and retry Save. Main Menu and Play Again do not save automatically. See the [save and upgrade guide](docs/SAVES.md) for backup and custom-directory instructions.

## 5. Rebuild only if you change the code

Rebuilding turns the source files into the runnable game. You can ignore this section if you only want to play.

On this computer, CMake and C++ compilers are already installed. Close the game and double-click **BUILD.bat**. The script finds those tools and runs the project's single CMake build, then stages the game into `dist`. Wait until it says **Ready**, then press a key to close that window. Double-click **PLAY.bat** again.

To run the verification checks as well, open PowerShell in the game folder and enter:

```powershell
.\BUILD.bat -Test
```

On a different Windows computer, the portable game still runs without a compiler. To edit and rebuild there, install [Visual Studio Community](https://visualstudio.microsoft.com/vs/community/) and select **Desktop development with C++**, including its CMake tools. Then run `BUILD.bat`. Microsoft's [installation walkthrough](https://learn.microsoft.com/en-us/cpp/build/vscpp-step-0-installation) shows the workload selection.

The [developer guide](docs/DEVELOPER.md) explains GCC/MSVC selection, direct CMake commands, and separate build/package directories. Keep the source package's complete `third_party` and `resources` folders: they contain verified library sources/runtime inputs and game resources, while `dist` contains generated files for playing. Rebuilding does not read or replace your user leaderboard.

## Troubleshooting

| What you see | What to do |
| --- | --- |
| `SDL2.dll` is missing | Keep the supplied `SDL2.dll` beside `Starfall.exe`; restore the complete `dist` folder from the game ZIP. |
| Another DLL, image, or font is missing | Restore the complete package, including all supplied DLLs and the `resources` folder. The startup message names a missing/corrupt required resource. |
| Game does not fit the screen | Press F11 or drag a window corner. The game keeps its original proportions. |
| Game stops when you switch windows | This is automatic pause; return and click Resume Mission. |
| No sound | Check the in-game sound button and Windows volume. The game remains playable if an audio device cannot open. |
| Footer says Audio Resource Error | Restore the complete package, including `resources/reference/shoot.flac`. Keep the resource folders beside the executable. |
| Score will not save | The end screen displays the error. Check that your user save folder is writable, then click Save again. |
| Rebuild cannot replace the game | Close Starfall before running BUILD.bat. |
| No compiler found on another computer | Use the already built game, or install Visual Studio with the C++ workload described above. |
| CMake or a build program is missing | Use Visual Studio's C++ workload with CMake tools, or install CMake plus Ninja for a GCC build. See the developer guide. |

The [audio guide](docs/AUDIO.md) explains sound controls during name entry and the difference between a missing audio resource and an unavailable output device.

For a custom save location, start from PowerShell using a folder you can write to:

```powershell
.\dist\Starfall.exe --data-dir .\my-scores
```

The complete source stays in this folder. You can move the portable `dist` folder to another Windows x64 computer; user scores are stored separately in each computer's user profile.
