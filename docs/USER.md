# Starfall user guide

This guide covers starting, configuring, upgrading, and recovering the Windows x64 game. The [player guide](PLAYER.md) explains combat and scoring; the [save guide](SAVES.md) explains score backups and compatibility.

## Start the game

Extract the complete **Starfall-Windows-x64.zip** into a folder you can open. Double-click **PLAY.bat** or **Starfall.exe** inside it. The launcher identifies missing required package files. **Starfall-Source.zip** is the separate rebuild package; its source-folder launcher builds a missing `dist` first and therefore needs the developer prerequisites.

Keep the executable, all supplied DLLs, and the complete `resources` folder together. The game loads artwork, lettering, and sound from its own package, so a shortcut can start it from another working folder. Playing the supplied build requires no compiler, account, installation, or online dependency download.

The welcome screen offers **Start Mission**, **Leaderboard**, **How to Play**, and **Quit**. Click a control, or use **Tab/Up/Down** followed by **Enter**. **Shift+Tab** moves backward. **Escape** returns from the manual or leaderboard to the screen that opened it. Closing the window exits the game.

## Display, pause, and sound

- Drag a window corner to resize it. The game preserves its proportions, adding unused space when necessary.
- Press **F11** to switch between fullscreen and a window.
- Press **Escape** or **P**, or click **Pause**, to freeze a run. Returning from another window also leaves the run paused; choose **Resume Mission** when ready.
- Use the sound button at the lower right on any screen. **M** is a shortcut outside pilot-name entry; while entering a name, use the button so letters remain available for the name.
- **Sound Off** means muted. **No Audio** means an audio device could not be opened; the game remains playable.

## Save a completed run

After victory or defeat, type a pilot name and select **Save**. Clicking the name field selects the existing name; typing replaces it and **Enter** saves. You can also navigate to Save with the keyboard. Supported names contain up to twelve English letters, numbers, spaces, hyphens, or underscores. Letters become uppercase, surrounding spaces are removed, and an empty usable name becomes **PILOT**.

Each completed run can be submitted once. The end screen confirms a retained rank or **Outside the Top 10**. The latter means submission succeeded but the score was too low to retain. Different runs can have the same name and score.

Save before choosing **Play Again** or **Main Menu**. Those controls do not save automatically. Abandoning an active run does not save it either. If Save fails, keep the end screen open, correct the problem described below, and retry.

## Where scores live

The normal Windows score file is:

```text
%APPDATA%\Starfall\Starfall\leaderboard.txt
```

Scores stay in your Windows user profile rather than beside the executable. Existing leaderboards remain compatible. The [save guide](SAVES.md) covers backups, damaged rows, and recovery.

For a custom writable folder, open PowerShell in the portable game folder:

```powershell
.\Starfall.exe --data-dir .\my-scores
```

In the full source folder, use `.\dist\Starfall.exe` instead. Relative custom save paths are relative to the working folder you launch from; packaged resource paths remain relative to the executable. Reuse the same explicit save folder when reopening if you want to see those scores.

## Upgrade or move the game

Close the game before replacing package files. Extract the complete new package rather than mixing DLLs or resource files from different versions. Your ordinary leaderboard stays in your Windows user profile, so replacing the portable folder needs no score migration. Back it up first if you want an extra copy.

Moving the game to another computer does not move that computer's user profile. To take scores with you, copy your backed-up leaderboard into the corresponding writable save folder while the game is closed, as described in the save guide. A custom save folder must be retained separately if it sits inside a folder you are replacing.

## Troubleshooting

| What happens | What to do |
| --- | --- |
| Windows reports a missing DLL | Restore the complete package, keeping all four supplied SDL DLLs beside Starfall.exe. |
| Startup names a missing or corrupt image/font | Restore the named file from the complete matching package; keep the original resource subfolders. |
| Sound is muted or unavailable | Check the sound button, Windows volume, and audio device. Play can continue silently when no device opens. Restore the package if a sound-resource diagnostic names a missing or corrupt clip. |
| The run stops when switching windows | This is automatic pause. Return and choose Resume Mission. |
| The display has borders or the game is too large | Resize the window or press F11; unused space preserves the game's proportions. |
| Save reports a write or replacement error | Keep the end screen open, close programs holding the score file, check that the save folder is writable, then retry Save. The failed attempt preserves the previous leaderboard. |
| The leaderboard is empty after using a custom folder | Start with the same `--data-dir` location used for those scores. |
| Rebuilding cannot replace the executable | Close Starfall before rebuilding. Playing an existing portable build needs no rebuild. |

For rebuilding or changing the source, use the developer guide in the full source folder. [Credits](CREDITS.md) identify reference resources and third-party libraries; their notices remain in the package.
