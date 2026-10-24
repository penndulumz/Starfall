# Space Shooter

A single-player space shooter with complete start and end flows and a local score history.

## Language

**Run**:
One attempt at the game, beginning when the player starts and ending when the player wins or loses.
_Avoid_: Match, session

**Welcome screen**:
The opening screen where the player starts a run or opens the leaderboard.
_Avoid_: Lobby

**End screen**:
The screen after a completed run showing its result and offering the next action.
_Avoid_: Endgame screen

**Leaderboard**:
The ten highest completed-run scores saved on this computer, each associated with a player name.
_Avoid_: Global ranking, online ranking

**Wave**:
A group of hostiles that must all be destroyed before the next group arrives. A run contains ten waves.
_Avoid_: Level, round

**Hull**:
The player's remaining ability to withstand damage, shown as five segments. An empty hull ends the run in defeat.
_Avoid_: Lives, shield

**Dash**:
A short burst of movement that briefly protects the player from damage and then needs to recharge.
_Avoid_: Teleport, boost pickup

**Overdrive**:
A temporary weapon upgrade that increases firing speed and produces paired shots.
_Avoid_: Permanent upgrade

**Kill chain**:
A sequence of kills close enough together to increase the score multiplier. Taking damage or waiting too long ends the chain.
_Avoid_: Kill streak

**Dreadnought**:
The large final hostile encountered in the tenth wave. Clearing its wave wins the run.
_Avoid_: Mothership
