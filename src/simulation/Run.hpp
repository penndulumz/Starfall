#pragma once

#include "Geometry.hpp"
#include <cstdint>
#include <random>
#include <vector>

namespace starfall {

enum class EnemyType { Scout, Gunner, Tank, Boss };
enum class PickupType { Repair, Overdrive };
enum class Result { Active, Defeat, Victory };
enum class EventType { Shot, Hit, Explosion, Pickup, Dash, Wave, Win, Lose };

struct Event {
    EventType type;
    Vec position;
};
struct Bullet {
    Vec position;
    Vec previous;
    Vec velocity;
    float life = 2;
    bool hostile = false;
};
struct Enemy {
    Vec position;
    EnemyType type = EnemyType::Scout;
    int health = Rules::ScoutHull;
    int maxHealth = Rules::ScoutHull;
    float radius = Rules::ScoutRadius;
    float cooldown = 2;
    float phase = 0;
};
struct Pickup {
    Vec position;
    PickupType type;
    float life = 14;
};
struct Particle {
    Vec position;
    Vec velocity;
    float life;
    float duration;
    float size;
    int tone;
};
struct Input {
    Vec movement;
    Vec aim;
    bool fire = false;
    bool dash = false;
};

// A borrowed const view of the current run; valid until its owning Run is destroyed.
// Run updates this state in place. Callers must copy any snapshot they need to retain;
// start/update may invalidate references and iterators into its entity collections.
struct RunState {
    Vec player{ArenaWidth / 2, ArenaHeight / 2};
    Vec facing{0, -1};
    Vec dashDirection{0, -1};
    int health = Rules::MaximumHull;
    int score = 0;
    int wave = 1;
    int kills = 0;
    int chain = 0;
    int queuedEnemies = 0;
    float elapsed = 0;
    float invulnerable = 0;
    float shotCooldown = 0;
    float dashCooldown = 0;
    float dashTime = 0;
    float overdrive = 0;
    float chainTime = 0;
    float waveDelay = 0;
    float spawnTimer = 0;
    float announcement = 0;
    float shake = 0;
    Result result = Result::Active;
    std::vector<Enemy> enemies;
    std::vector<Bullet> bullets;
    std::vector<Pickup> pickups;
    std::vector<Particle> particles;
    std::vector<Event> events;

    int multiplier() const {
        return 1 + std::min(Rules::MaximumMultiplier - 1, chain / Rules::KillsPerMultiplier);
    }
    int remaining() const { return queuedEnemies + static_cast<int>(enemies.size()); }
};

namespace testing { class RunFixture; }

// SDL-independent rules. Only start/update may change state in production.
class Run {
public:
    void start(std::uint32_t seed = std::random_device{}());
    void update(float seconds, const Input& input);
    const RunState& view() const noexcept { return state_; }
    int multiplier() const { return state_.multiplier(); }
    int remaining() const { return state_.remaining(); }

private:
    // Defined only in test support, allowing controlled scenario setup without setters.
    friend class testing::RunFixture;
    RunState state_;
    std::mt19937 random_{1};

    float between(float low, float high);
    void beginWave(int number);
    void spawnEnemy();
    void burst(Vec position, int count, int tone);
    void damagePlayer();
    void destroy(Enemy& enemy);
    void updateTimers(float seconds);
    void updatePlayer(float seconds, const Input& input);
    void updateSpawning(float seconds);
    void updateEnemies(float seconds);
    void updateProjectiles(float seconds);
    void updatePickups(float seconds);
    void updateParticles(float seconds);
    void removeExpired();
    void finishWave();
};

} // namespace starfall
