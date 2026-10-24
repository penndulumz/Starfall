# Scores, backups, and upgrades

Starfall's leaderboard is local to this computer and Windows user. It keeps the best ten completed-run scores; no account, online ranking, or cloud synchronization is involved.

## Keep scores when upgrading

Close Starfall, then replace the portable game package. The existing score format and current-user save location remain compatible, so no migration is needed. The usual location is:

```text
%APPDATA%\Starfall\Starfall\leaderboard.txt
```

The game asks SDL for this user preferences folder. Saves are separate from the executable, packaged resources, source, and build folders. To back up scores, close the game and copy `leaderboard.txt` somewhere safe. Restoring a backup replaces the local history rather than merging two leaderboards; keep a copy of both files before doing that yourself.

An explicit custom folder is supported:

```powershell
.\dist\Starfall.exe --data-dir .\my-scores
```

A relative custom directory is relative to the working directory you launch from. Keep using the same explicit directory after an upgrade; otherwise you will see the normal user-profile leaderboard instead of that custom one. Resource loading does not change this save-directory choice.

## Submit a completed run

On the victory or defeat end screen, enter a pilot name and activate Save. Names allow up to twelve English letters, digits, spaces, hyphens, or underscores. Letters become uppercase, surrounding spaces are removed, and an empty usable name becomes `PILOT`.

A completed Run can be submitted once. A different Run may legitimately have the same name and score. Equal scores keep their earlier order; a new equal score follows the retained equal entries. A successful submission below the top ten reports **Outside the Top 10**, even though it is not retained in the file.

Save before choosing Main Menu or Play Again. Those actions do not save automatically, and abandoning an active run does not write a score.

## Recover from a save error

Leave the end screen open so the completed Run is still available. The game writes a temporary file and replaces the previous leaderboard only after writing and closing it successfully. A failed write or replacement leaves the previous disk file and retained in-memory entries unchanged, and Save stays available to retry.

| Error | Recovery |
| --- | --- |
| Cannot create or write the save folder | Check the selected directory exists and is writable. A file with the directory's name can also block creation. |
| Cannot replace the score file | Close editors or other programs holding `leaderboard.txt`, check folder permissions, then retry Save. |
| Score file unreadable or unknown format | Check that `leaderboard.txt` is a regular file and restore a known backup if appropriate. Keep the original copy before editing it. |

Malformed rows are ignored while neighboring valid rows remain available. Unknown versions or unreadable files produce a warning. The format is still `STARFALL_SCORES_V1`; source reorganization does not introduce a new profile or reset stored history.

The automated compatibility fixture contains fictional pilots and is used only in isolated build/test directories. It is never installed into the portable game or written into a player's normal user profile.
