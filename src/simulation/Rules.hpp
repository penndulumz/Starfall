#pragma once

namespace starfall::Rules {

// Distances use world units; speeds are world units per second; timers use seconds.
inline constexpr float ArenaWidth = 2600.f;
inline constexpr float ArenaHeight = 1800.f;
inline constexpr int FinalWave = 10;
inline constexpr int MaximumHull = 5;
inline constexpr float FixedStep = 1.f / 120.f;
inline constexpr float MaximumUpdate = .05f;

inline constexpr float PlayerSpeed = 290.f;
inline constexpr float PlayerMargin = 30.f;
inline constexpr float PlayerHitRadius = 16.f;
inline constexpr float DamageProtection = 1.35f;
inline constexpr float DashSpeed = 1000.f;
inline constexpr float DashDuration = .19f;
inline constexpr float DashRecharge = 2.6f;
inline constexpr float DashProtection = .26f;

inline constexpr float ShotInterval = .14f;
inline constexpr float OverdriveShotInterval = .075f;
inline constexpr float ShotSpeed = 880.f;
inline constexpr float ShotLifetime = 1.5f;
inline constexpr float ShotMuzzleOffset = 29.f;
inline constexpr float OverdriveSpread = .045f;
inline constexpr float OverdriveDuration = 10.f;
inline constexpr float PickupAttractDistance = 100.f;
inline constexpr float PickupAttractSpeed = 200.f;
inline constexpr float PickupCollectDistance = 30.f;

inline constexpr float FirstWaveDelay = 1.8f;
inline constexpr float WaveBreak = 3.f;
inline constexpr float WaveAnnouncement = 3.5f;
inline constexpr float SpawnInterval = .75f;
inline constexpr float FinalWaveSpawnInterval = 1.5f;
inline constexpr int FinalWaveHostiles = 5;
inline constexpr float SpawnNearDistance = 570.f;
inline constexpr float SpawnFarDistance = 760.f;
inline constexpr float MinimumSpawnDistance = 360.f;
inline constexpr int BaseWaveHostiles = 4;
inline constexpr int HostilesPerWave = 2;
inline constexpr float SpawnMargin = 55.f;
inline constexpr float OppositeSpawnInset = 100.f;

inline constexpr int ScoutHull = 2;
inline constexpr int LateScoutHull = 3;
inline constexpr int LateScoutFirstWave = 7;
inline constexpr float ScoutRadius = 19.f;
inline constexpr int GunnerFirstWave = 2;
inline constexpr float GunnerSpawnThreshold = .57f;
inline constexpr int GunnerHull = 3;
inline constexpr float GunnerRadius = 23.f;
inline constexpr int TankFirstWave = 4;
inline constexpr float TankSpawnThreshold = .23f;
inline constexpr int TankBaseHull = 6;
inline constexpr int TankHullWaveDivisor = 3;
inline constexpr float TankRadius = 29.f;
inline constexpr int DreadnoughtHull = 110;
inline constexpr float DreadnoughtRadius = 66.f;
inline constexpr float FirstAttackDelayMinimum = 1.f;
inline constexpr float FirstAttackDelayMaximum = 2.5f;

inline constexpr float EnemyBaseSpeed = 70.f;
inline constexpr float EnemySpeedPerWave = 4.f;
inline constexpr float TankBaseSpeed = 55.f;
inline constexpr float TankSpeedPerWave = 2.f;
inline constexpr float GunnerDistance = 300.f;
inline constexpr float DreadnoughtDistance = 330.f;
inline constexpr float ApproachDistance = 100.f;
inline constexpr float StrafeSpeed = 45.f;
inline constexpr float GunnerShotInterval = 3.2f;
inline constexpr float GunnerIntervalPerWave = .12f;
inline constexpr float GunnerMinimumShotInterval = 1.5f;
inline constexpr float DreadnoughtShotInterval = 1.05f;
inline constexpr int DreadnoughtShotCount = 7;
inline constexpr float DreadnoughtSpread = .18f;
inline constexpr float GunnerShotSpeed = 205.f;
inline constexpr float DreadnoughtShotSpeed = 235.f;
inline constexpr float EnemyShotLifetime = 6.f;
inline constexpr float EnemyMuzzleOffset = 8.f;
inline constexpr float ContactPlayerRadius = 18.f;
inline constexpr float ContactKnockback = 85.f;
inline constexpr float FriendlyHitPadding = 3.f;

inline constexpr int RepairKillInterval = 5;
inline constexpr int OverdriveKillInterval = 7;
inline constexpr int CriticalHullThreshold = 3;
inline constexpr float EmergencyRepairChance = .28f;

inline constexpr float ChainDuration = 4.f;
inline constexpr int KillsPerMultiplier = 5;
inline constexpr int MaximumMultiplier = 5;
inline constexpr int ScoutScore = 100;
inline constexpr int GunnerScore = 200;
inline constexpr int TankScore = 350;
inline constexpr int DreadnoughtScore = 5000;
inline constexpr int WaveClearScore = 100;
inline constexpr int VictoryScore = 5000;
inline constexpr int RemainingHullScore = 250;

} // namespace starfall::Rules
