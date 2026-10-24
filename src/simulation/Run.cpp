#include "Run.hpp"

namespace starfall {

namespace {
void decay(float& timer, float seconds) {
    timer = std::max(0.f, timer - seconds);
}

template <typename Values, typename Predicate>
void eraseDead(Values& values, Predicate predicate) {
    values.erase(std::remove_if(values.begin(), values.end(), predicate), values.end());
}
} // namespace

bool segmentHitsCircle(Vec from, Vec to, Vec center, float radius) {
    const Vec delta = to - from;
    const float squaredDistance = dot(delta, delta);
    // Project the circle center onto the segment, including either endpoint.
    // A stationary projectile tests overlap at its current position.
    const float fraction = squaredDistance > 0
        ? std::clamp(dot(center - from, delta) / squaredDistance, 0.f, 1.f) : 0;
    const Vec offset = from + delta * fraction - center;
    return dot(offset, offset) <= radius * radius;
}

float Run::between(float low, float high) {
    return std::uniform_real_distribution<float>(low, high)(random_);
}

void Run::start(std::uint32_t seed) {
    state_ = RunState{};
    random_.seed(seed);
    beginWave(1);
}

void Run::beginWave(int number) {
    auto& state = state_;
    state.wave = number;
    state.queuedEnemies = number == FinalWave ? Rules::FinalWaveHostiles : Rules::BaseWaveHostiles + number * Rules::HostilesPerWave;
    state.waveDelay = number == 1 ? Rules::FirstWaveDelay : Rules::WaveBreak;
    state.spawnTimer = 0;
    state.announcement = Rules::WaveAnnouncement;
    state.events.push_back({EventType::Wave, state.player});
}

void Run::spawnEnemy() {
    auto& state = state_;
    Enemy enemy;
    enemy.position = bounded(state.player + direction(between(0, Pi * 2)) *
        between(Rules::SpawnNearDistance, Rules::SpawnFarDistance), Rules::SpawnMargin);
    // Clamping near an arena edge can shorten the spawn distance: use the far side.
    if (length(enemy.position - state.player) < Rules::MinimumSpawnDistance) {
        enemy.position = {state.player.x < ArenaWidth / 2 ? ArenaWidth - Rules::OppositeSpawnInset : Rules::OppositeSpawnInset,
                          state.player.y < ArenaHeight / 2 ? ArenaHeight - Rules::OppositeSpawnInset : Rules::OppositeSpawnInset};
    }
    const float roll = between(0, 1);
    if (state.wave == FinalWave && state.queuedEnemies == Rules::FinalWaveHostiles) {
        enemy.type = EnemyType::Boss;
        enemy.health = Rules::DreadnoughtHull;
        enemy.radius = Rules::DreadnoughtRadius;
    } else if (state.wave >= Rules::TankFirstWave && roll < Rules::TankSpawnThreshold) {
        enemy.type = EnemyType::Tank;
        enemy.health = Rules::TankBaseHull + state.wave / Rules::TankHullWaveDivisor;
        enemy.radius = Rules::TankRadius;
    } else if (state.wave >= Rules::GunnerFirstWave && roll < Rules::GunnerSpawnThreshold) {
        enemy.type = EnemyType::Gunner;
        enemy.health = Rules::GunnerHull;
        enemy.radius = Rules::GunnerRadius;
    } else {
        enemy.health = state.wave >= Rules::LateScoutFirstWave ? Rules::LateScoutHull : Rules::ScoutHull;
    }
    enemy.maxHealth = enemy.health;
    enemy.cooldown = between(Rules::FirstAttackDelayMinimum, Rules::FirstAttackDelayMaximum);
    enemy.phase = between(0, Pi * 2);
    state.enemies.push_back(enemy);
    --state.queuedEnemies;
}

void Run::burst(Vec position, int count, int tone) {
    for (int index = 0; index < count; ++index) {
        const float duration = between(.25f, .8f);
        state_.particles.push_back({position, direction(between(0, Pi * 2)) * between(35, 260),
                                   duration, duration, between(1.5f, 4), tone});
    }
}

void Run::damagePlayer() {
    auto& state = state_;
    if (state.invulnerable > 0 || state.result != Result::Active) return;
    --state.health;
    state.invulnerable = Rules::DamageProtection;
    state.chain = 0;
    state.chainTime = 0;
    state.shake = 10;
    burst(state.player, 18, 2);
    state.events.push_back({EventType::Hit, state.player});
    if (state.health <= 0) {
        state.health = 0;
        state.result = Result::Defeat;
        burst(state.player, 65, 2);
        state.events.push_back({EventType::Lose, state.player});
    }
}

void Run::destroy(Enemy& enemy) {
    auto& state = state_;
    ++state.kills;
    state.chain = state.chainTime > 0 ? state.chain + 1 : 1;
    state.chainTime = Rules::ChainDuration;
    const int base = enemy.type == EnemyType::Boss ? Rules::DreadnoughtScore :
        enemy.type == EnemyType::Tank ? Rules::TankScore :
        enemy.type == EnemyType::Gunner ? Rules::GunnerScore : Rules::ScoutScore;
    state.score += base * state.multiplier();
    burst(enemy.position, enemy.type == EnemyType::Boss ? 100 : 28, 1);
    state.shake = enemy.type == EnemyType::Boss ? 16.f : 4.f;
    state.events.push_back({EventType::Explosion, enemy.position});
    if (state.kills % Rules::RepairKillInterval == 0 ||
        (state.health < Rules::CriticalHullThreshold && between(0, 1) < Rules::EmergencyRepairChance)) {
        state.pickups.push_back({enemy.position, PickupType::Repair});
    } else if (state.kills % Rules::OverdriveKillInterval == 0) {
        state.pickups.push_back({enemy.position, PickupType::Overdrive});
    }
}

void Run::update(float seconds, const Input& input) {
    state_.events.clear();
    if (state_.result != Result::Active || seconds <= 0) return;
    seconds = std::min(seconds, Rules::MaximumUpdate);
    state_.elapsed += seconds;

    // Ordering preserves damage protection, pickup consumption, and wave transitions.
    updateTimers(seconds);
    updatePlayer(seconds, input);
    updateSpawning(seconds);
    updateEnemies(seconds);
    updateProjectiles(seconds);
    updatePickups(seconds);
    updateParticles(seconds);
    removeExpired();
    finishWave();
}

void Run::updateTimers(float seconds) {
    auto& state = state_;
    decay(state.invulnerable, seconds);
    decay(state.shotCooldown, seconds);
    decay(state.dashCooldown, seconds);
    decay(state.dashTime, seconds);
    decay(state.overdrive, seconds);
    decay(state.chainTime, seconds);
    decay(state.announcement, seconds);
    state.shake = std::max(0.f, state.shake - 32 * seconds);
    if (state.chainTime == 0) state.chain = 0;
}

void Run::updatePlayer(float seconds, const Input& input) {
    auto& state = state_;
    if (length(input.aim - state.player) > 1) state.facing = unit(input.aim - state.player);
    Vec movement = input.movement;
    if (length(movement) > 1) movement = unit(movement);
    if (input.dash && state.dashCooldown <= 0) {
        state.dashDirection = length(movement) > .1f ? unit(movement) : state.facing;
        state.dashTime = Rules::DashDuration;
        state.dashCooldown = Rules::DashRecharge;
        state.invulnerable = std::max(state.invulnerable, Rules::DashProtection);
        burst(state.player, 16, 0);
        state.events.push_back({EventType::Dash, state.player});
    }
    const Vec velocity = state.dashTime > 0 ? state.dashDirection * Rules::DashSpeed : movement * Rules::PlayerSpeed;
    state.player = bounded(state.player + velocity * seconds);
    if (input.fire && state.shotCooldown <= 0) {
        state.shotCooldown = state.overdrive > 0 ? Rules::OverdriveShotInterval : Rules::ShotInterval;
        const float angle = std::atan2(state.facing.y, state.facing.x);
        const int count = state.overdrive > 0 ? 2 : 1;
        for (int index = 0; index < count; ++index) {
            const float spread = count == 2 ? (index == 0 ? -Rules::OverdriveSpread : Rules::OverdriveSpread) : 0;
            const Vec aim = direction(angle + spread);
            const Vec nose = state.player + aim * Rules::ShotMuzzleOffset;
            state.bullets.push_back({nose, nose, aim * Rules::ShotSpeed, Rules::ShotLifetime, false});
        }
        state.events.push_back({EventType::Shot, state.player});
    }
}

void Run::updateSpawning(float seconds) {
    auto& state = state_;
    if (state.waveDelay > 0) {
        decay(state.waveDelay, seconds);
    } else if (state.queuedEnemies > 0) {
        state.spawnTimer -= seconds;
        if (state.spawnTimer <= 0) {
            spawnEnemy();
            state.spawnTimer = state.wave == FinalWave ? Rules::FinalWaveSpawnInterval : Rules::SpawnInterval;
        }
    }
}

void Run::updateEnemies(float seconds) {
    auto& state = state_;
    for (auto& enemy : state.enemies) {
        if (enemy.health <= 0) continue;
        const Vec toward = state.player - enemy.position;
        const float distance = length(toward);
        const Vec forward = unit(toward);
        enemy.phase += seconds;
        const float speed = Rules::EnemyBaseSpeed + state.wave * Rules::EnemySpeedPerWave;
        Vec velocity = forward * speed;
        if (enemy.type == EnemyType::Tank) velocity = forward * (Rules::TankBaseSpeed + state.wave * Rules::TankSpeedPerWave);
        if (enemy.type == EnemyType::Gunner || enemy.type == EnemyType::Boss) {
            const bool boss = enemy.type == EnemyType::Boss;
            const float desiredDistance = boss ? Rules::DreadnoughtDistance : Rules::GunnerDistance;
            const float approach = std::clamp((distance - desiredDistance) / Rules::ApproachDistance, -1.f, 1.f);
            velocity = forward * (speed * approach) + Vec{-forward.y, forward.x} * Rules::StrafeSpeed;
            enemy.cooldown -= seconds;
            if (enemy.cooldown <= 0) {
                enemy.cooldown = boss ? Rules::DreadnoughtShotInterval :
                    std::max(Rules::GunnerMinimumShotInterval, Rules::GunnerShotInterval - state.wave * Rules::GunnerIntervalPerWave);
                const float angle = std::atan2(toward.y, toward.x);
                const int count = boss ? Rules::DreadnoughtShotCount : 1;
                for (int index = 0; index < count; ++index) {
                    const float spread = boss ? (index - Rules::DreadnoughtShotCount / 2) * Rules::DreadnoughtSpread : 0;
                    const Vec aim = direction(angle + spread);
                    const Vec nose = enemy.position + aim * (enemy.radius + Rules::EnemyMuzzleOffset);
                    state.bullets.push_back({nose, nose, aim * (boss ? Rules::DreadnoughtShotSpeed : Rules::GunnerShotSpeed),
                                             Rules::EnemyShotLifetime, true});
                }
            }
        }
        enemy.position = bounded(enemy.position + velocity * seconds, enemy.radius);
        if (distance < enemy.radius + Rules::ContactPlayerRadius) {
            damagePlayer();
            enemy.position = bounded(enemy.position - forward * Rules::ContactKnockback, enemy.radius);
        }
    }
}

void Run::updateProjectiles(float seconds) {
    auto& state = state_;
    for (auto& bullet : state.bullets) {
        if (bullet.life <= 0) continue;
        bullet.previous = bullet.position;
        bullet.position += bullet.velocity * seconds;
        bullet.life -= seconds;
        if (bullet.hostile) {
            if (segmentHitsCircle(bullet.previous, bullet.position, state.player, Rules::PlayerHitRadius)) {
                bullet.life = 0;
                damagePlayer();
            }
        } else {
            for (auto& enemy : state.enemies) {
                if (enemy.health > 0 && segmentHitsCircle(bullet.previous, bullet.position, enemy.position, enemy.radius + Rules::FriendlyHitPadding)) {
                    bullet.life = 0;
                    --enemy.health;
                    burst(bullet.position, 3, 1);
                    if (enemy.health <= 0) destroy(enemy);
                    break;
                }
            }
        }
    }
}

void Run::updatePickups(float seconds) {
    auto& state = state_;
    for (auto& pickup : state.pickups) {
        pickup.life -= seconds;
        const Vec toward = state.player - pickup.position;
        if (length(toward) < Rules::PickupAttractDistance) {
            pickup.position += unit(toward) * (Rules::PickupAttractSpeed * seconds);
        }
        // A repair cannot reverse a defeat that happened earlier in this same step.
        if (state.result == Result::Active && length(toward) < Rules::PickupCollectDistance) {
            if (pickup.type == PickupType::Repair) state.health = std::min(Rules::MaximumHull, state.health + 1);
            else state.overdrive = Rules::OverdriveDuration;
            pickup.life = 0;
            burst(state.player, 18, 0);
            state.events.push_back({EventType::Pickup, state.player});
        }
    }
}

void Run::updateParticles(float seconds) {
    for (auto& particle : state_.particles) {
        particle.life -= seconds;
        particle.position += particle.velocity * seconds;
        particle.velocity = particle.velocity * std::max(0.f, 1 - 2 * seconds);
    }
}

void Run::removeExpired() {
    eraseDead(state_.enemies, [](const Enemy& enemy) { return enemy.health <= 0; });
    eraseDead(state_.bullets, [](const Bullet& bullet) {
        return bullet.life <= 0 || bullet.position.x < 0 || bullet.position.y < 0 ||
            bullet.position.x > ArenaWidth || bullet.position.y > ArenaHeight;
    });
    eraseDead(state_.pickups, [](const Pickup& pickup) { return pickup.life <= 0; });
    eraseDead(state_.particles, [](const Particle& particle) { return particle.life <= 0; });
}

void Run::finishWave() {
    auto& state = state_;
    // Cleanup precedes this check, so dead hostiles do not delay the next wave.
    if (state.result != Result::Active || state.waveDelay > 0 || state.remaining() != 0) return;
    state.score += state.wave * Rules::WaveClearScore;
    if (state.wave == FinalWave) {
        state.score += Rules::VictoryScore + state.health * Rules::RemainingHullScore;
        state.result = Result::Victory;
        state.events.push_back({EventType::Win, state.player});
    } else {
        beginWave(state.wave + 1);
    }
}

} // namespace starfall
