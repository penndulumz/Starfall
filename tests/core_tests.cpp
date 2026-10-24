#include "simulation/Run.hpp"
#include "support/RunFixture.hpp"
#include "support/ScoreChecks.hpp"
#include "support/ScoreFileBlocker.hpp"
#include "persistence/Scores.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

using namespace starfall;
static_assert(std::is_const_v<std::remove_reference_t<decltype(std::declval<Run&>().view())>>,
              "Observers must not mutate a Run through its view");
namespace fs = std::filesystem;
static int passed = 0;
static void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
    ++passed;
}
static Run fixture() {
    Run run;
    run.start(42);
    testing::RunFixture::state(run).queuedEnemies = 1;
    testing::RunFixture::state(run).waveDelay = 100;
    return run;
}

int main(int argc, char** argv) {
    try {
        const fs::path root = (argc > 1 ? fs::path(argv[1]) : fs::path("build/qa")) /
            ("core-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(root);
        Run freshRun;
        freshRun.start(7);
        check(freshRun.view().health == 5 && freshRun.view().wave == 1 &&
              freshRun.view().result == Result::Active, "Read-only view exposes a freshly started run");
        check(segmentHitsCircle({0, 0}, {100, 0}, {50, 0}, 2), "Swept projectile must hit between frames");
        check(!segmentHitsCircle({0, 0}, {100, 0}, {50, 8}, 2), "Projectile must miss outside radius");
        check(segmentHitsCircle({5, 5}, {5, 5}, {5, 5}, 2), "Stationary overlap must hit");
        auto straight = fixture(), diagonal = fixture();
        Vec origin = straight.view().player;
        straight.update(.05f, {{1, 0}, {2000, 900}});
        diagonal.update(.05f, {{1, 1}, {2000, 900}});
        check(std::abs(length(straight.view().player - origin) - length(diagonal.view().player - origin)) < .001f, "Diagonal movement must not be faster");
        testing::RunFixture::state(straight).player = {ArenaWidth, ArenaHeight};
        straight.update(.05f, {{1, 1}, {2000, 900}});
        check(straight.view().player.x <= ArenaWidth - 30 && straight.view().player.y <= ArenaHeight - 30, "Player stays inside arena");
        auto dash = fixture();
        dash.update(.01f, {{1, 0}, {2000, 900}, false, true});
        check(dash.view().dashCooldown > 2 && dash.view().invulnerable > 0 && dash.view().player.x > origin.x + 5, "Dash moves and protects the player");
        float cooldown = dash.view().dashCooldown;
        dash.update(.01f, {{1, 0}, {2000, 900}, false, true});
        check(dash.view().dashCooldown < cooldown, "Holding dash cannot reset its cooldown");
        auto firing = fixture();
        firing.update(.01f, {{}, {2000, 900}, true});
        check(firing.view().bullets.size() == 1, "Firing produces one projectile");
        firing.update(.01f, {{}, {2000, 900}, true});
        check(firing.view().bullets.size() == 1, "Fire rate limits repeated shots");
        auto hit = fixture();
        Enemy enemy;
        enemy.position = hit.view().player + Vec{0, -40}; enemy.health = enemy.maxHealth = 1;
        testing::RunFixture::state(hit).enemies.push_back(enemy);
        Vec from = hit.view().player + Vec{0, -100};
        testing::RunFixture::state(hit).bullets.push_back({from, from, {0, 10000}, 2, false});
        hit.update(.01f, {{}, hit.view().player + Vec{0, -200}});
        check(hit.view().kills == 1 && hit.view().score == 100 && hit.view().enemies.empty(), "Destroying an enemy awards one kill and score");
        auto injured = fixture();
        testing::RunFixture::state(injured).bullets.push_back({injured.view().player, injured.view().player, {}, 2, true});
        testing::RunFixture::state(injured).bullets.push_back({injured.view().player, injured.view().player, {}, 2, true});
        injured.update(.01f, {{}, {2000, 900}});
        check(injured.view().health == 4, "Invulnerability prevents multiple hits in one frame");
        testing::RunFixture::state(injured).health = 1; testing::RunFixture::state(injured).invulnerable = 0;
        testing::RunFixture::state(injured).pickups.push_back({injured.view().player, PickupType::Repair});
        testing::RunFixture::state(injured).bullets.push_back({injured.view().player, injured.view().player, {}, 2, true});
        injured.update(.01f, {{}, {2000, 900}});
        check(injured.view().result == Result::Defeat && injured.view().health == 0, "Zero hull ends the run");
        float endedTime = injured.view().elapsed;
        injured.update(.05f, {{1, 0}, {2000, 900}, true});
        check(injured.view().elapsed == endedTime, "Completed runs freeze simulation");
        auto repair = fixture();
        testing::RunFixture::state(repair).health = 4;
        testing::RunFixture::state(repair).pickups.push_back({repair.view().player, PickupType::Repair});
        repair.update(.01f, {{}, {2000, 900}});
        check(repair.view().health == 5 && repair.view().pickups.empty(), "Repair pickup restores hull and is consumed");
        testing::RunFixture::state(repair).pickups.push_back({repair.view().player, PickupType::Repair});
        repair.update(.01f, {{}, {2000, 900}});
        check(repair.view().health == 5, "Repair cannot exceed maximum hull");
        testing::RunFixture::state(repair).pickups.push_back({repair.view().player, PickupType::Overdrive});
        repair.update(.01f, {{}, {2000, 900}});
        repair.update(.01f, {{}, {2000, 900}, true});
        check(repair.view().overdrive > 9 && repair.view().bullets.size() == 2, "Overdrive fires paired shots");
        auto progression = fixture();
        testing::RunFixture::state(progression).queuedEnemies = 0; testing::RunFixture::state(progression).waveDelay = 0;
        progression.update(.01f, {{}, {2000, 900}});
        check(progression.view().wave == 2 && progression.view().queuedEnemies > 0 && progression.view().waveDelay > 0, "Clearing a wave schedules the next");
        testing::RunFixture::state(progression).wave = 10; testing::RunFixture::state(progression).queuedEnemies = 0; testing::RunFixture::state(progression).waveDelay = 0;
        progression.update(.01f, {{}, {2000, 900}});
        check(progression.view().result == Result::Victory && progression.view().score >= 6000, "Clearing wave ten wins and awards a bonus");
        progression.start(5);
        check(progression.view().result == Result::Active && progression.view().score == 0 && progression.view().health == 5 && progression.view().wave == 1, "Retry resets the entire run");
        auto chain = fixture();
        testing::RunFixture::state(chain).chain = 25; testing::RunFixture::state(chain).chainTime = .001f;
        chain.update(.01f, {{}, {2000, 900}});
        check(chain.multiplier() == 1, "Expired chain resets multiplier");
        auto first = fixture(), second = fixture();
        testing::RunFixture::state(first).waveDelay = testing::RunFixture::state(second).waveDelay = 0;
        first.update(.01f, {{}, {2000, 900}}); second.update(.01f, {{}, {2000, 900}});
        check(first.view().enemies[0].position.x == second.view().enemies[0].position.x, "A fixed seed yields deterministic spawns");
        auto edgeSpawn = fixture();
        testing::RunFixture::state(edgeSpawn).player = {30,30}; testing::RunFixture::state(edgeSpawn).waveDelay = 0;
        edgeSpawn.update(.01f, {{}, {2000,900}});
        check(length(edgeSpawn.view().enemies.front().position - edgeSpawn.view().player) >= 360, "Enemies do not spawn on a cornered player");
        auto boss = fixture();
        testing::RunFixture::state(boss).wave = 10; testing::RunFixture::state(boss).queuedEnemies = 5; testing::RunFixture::state(boss).waveDelay = 0;
        boss.update(.01f, {{}, {2000,900}});
        check(boss.view().enemies.front().type == EnemyType::Boss && boss.view().enemies.front().health == 110, "Final wave spawns the Dreadnought first");
        testing::RunFixture::state(boss).enemies.front().cooldown = 0;
        boss.update(.01f, {{}, {2000,900}});
        check(boss.view().bullets.size() == 7, "Dreadnought fires its spread attack");
        Run campaign;
        campaign.start(17);
        // An invulnerable test pilot isolates full campaign progression from bot skill.
        for (int frame = 0; frame < 18000 && campaign.view().result == Result::Active; ++frame) {
            testing::RunFixture::state(campaign).invulnerable = 2;
            Vec target = campaign.view().player + Vec{0,-100};
            float distance = 100000;
            for (const auto& foe : campaign.view().enemies) {
                float d = length(foe.position - campaign.view().player);
                if (d < distance) { distance = d; target = foe.position; }
            }
            Vec toward = unit(target - campaign.view().player);
            Vec movement = distance < 230 ? toward * -1.f : Vec{-toward.y, toward.x} * .35f;
            campaign.update(.05f, {movement, target, true, false});
        }
        check(campaign.view().result == Result::Victory && campaign.view().wave == 10 && campaign.view().kills > 100, "The entire campaign is completable through real projectile collisions");
        std::cout << "Campaign simulation: " << campaign.view().kills << " hostiles defeated in " << static_cast<int>(campaign.view().elapsed) << " simulated seconds\n";

        Scores scores(root / "scores");
        check(scores.entries().empty(), "First launch has an empty leaderboard");
        check(Scores::cleanName("   abc! xyz   ") == "ABC XYZ", "Names are normalized safely");
        check(Scores::cleanName("!!!") == "PILOT", "Empty names have a usable fallback");
        check(Scores::cleanName("ABCDEFGHIJKLMNOP").size() == 12, "Names are limited to twelve characters");
        for (int i = 0; i < 12; ++i) check(scores.submit({"PILOT", i * 100, 1, 20, false}).ok, "Score can be saved repeatedly");
        check(scores.entries().size() == 10 && scores.entries().front().points == 1100 && scores.entries().back().points == 200, "Only the ten highest scores are retained");
        auto tied = scores.submit({"TIED", 1100, 1, 20, false});
        check(tied.ok && tied.rank == 2 && scores.entries()[0].name == "PILOT", "Equal scores retain their original order");
        Scores reopened(root / "scores");
        check(reopened.entries().size() == 10 && reopened.entries()[1].name == "TIED", "Scores survive closing and reopening");
        check(reopened.submit({"LOW", 0, 1, 20, false}).rank == 0, "Scores outside the top ten report no rank");
        check(!reopened.submit({"INVALID", -1, 1, 20, false}).ok, "Invalid scores are rejected");
        fs::create_directories(root / "malformed");
        std::ofstream bad(root / "malformed" / "leaderboard.txt");
        bad << "STARFALL_SCORES_V1\n\"VALID\" 800 2 30 0\nGARBAGE\n\"BAD\" -5 1 20 0\n\"BAD\" 9 99 20 0\n\"BAD\" 9 1 20 4\n"
               "UNQUOTED 900 2 30 0\n";
        bad.close();
        Scores recovered(root / "malformed");
        check(recovered.entries().size() == 1 && recovered.entries()[0].name == "VALID", "Malformed rows, including an unquoted V1 name, do not discard valid scores");
        std::ofstream(root / "blocked") << "not a directory";
        Scores blocked(root / "blocked");
        check(!blocked.submit({"TEST", 1, 1, 1, false}).ok && blocked.entries().empty(), "Save failure is reported without changing in-memory scores");

        const fs::path legacyFixture = argc > 2 ? fs::path(argv[2]) : fs::path(__FILE__).parent_path() / "fixtures/leaderboard-v1.txt";
        const auto legacyDirectory = root / "legacy";
        fs::create_directories(legacyDirectory);
        fs::copy_file(legacyFixture, legacyDirectory / "leaderboard.txt");
        Scores legacy(legacyDirectory);
        check(testing::sameScores(legacy.entries(), testing::legacyScores()), "Pre-overhaul V1 fixture preserves every column and original tie order");
        const auto newTied = legacy.submit({"RETURNING", 2000, 4, 95, false});
        check(newTied.ok && newTied.rank == 4 && legacy.entries()[1].name == "EARLY-TIE" &&
              legacy.entries()[2].name == "LATER-TIE" && legacy.entries()[3].name == "RETURNING",
              "A new tied score follows both pre-overhaul tied scores");
        Scores legacyReloaded(legacyDirectory);
        check(testing::sameScores(legacyReloaded.entries(), legacy.entries()), "Old and newly submitted V1 results survive reopening unchanged");

        const auto writeDirectory = root / "write-failure";
        fs::create_directories(writeDirectory);
        fs::copy_file(legacyFixture, writeDirectory / "leaderboard.txt");
        Scores writeFailure(writeDirectory);
        const auto previousBytes = testing::scoreFileBytes(writeDirectory / "leaderboard.txt");
        fs::create_directory(writeDirectory / "leaderboard.tmp");
        const auto failedWrite = writeFailure.submit({"RETRY", 8000, 8, 170, false});
        check(!failedWrite.ok && !failedWrite.error.empty() && testing::sameScores(writeFailure.entries(), testing::legacyScores()),
              "A failed temporary-file write leaves retained memory unchanged");
        check(testing::scoreFileBytes(writeDirectory / "leaderboard.txt") == previousBytes &&
              testing::sameScores(Scores(writeDirectory).entries(), testing::legacyScores()),
              "A failed temporary-file write preserves the prior file and readable entries");
        fs::remove(writeDirectory / "leaderboard.tmp");
        check(writeFailure.submit({"RETRY", 8000, 8, 170, false}).ok,
              "A temporary-file failure can be retried after its directory is recovered");

#ifdef _WIN32
        const auto replaceDirectory = root / "replace-failure";
        fs::create_directories(replaceDirectory);
        fs::copy_file(legacyFixture, replaceDirectory / "leaderboard.txt");
        Scores replacement(replaceDirectory);
        const auto originalBytes = testing::scoreFileBytes(replaceDirectory / "leaderboard.txt");
        {
            testing::ScoreFileBlocker heldReader(replaceDirectory / "leaderboard.txt");
            const auto failedReplacement = replacement.submit({"RETRY", 8000, 8, 170, false});
            check(!failedReplacement.ok && failedReplacement.error == "CANNOT REPLACE SCORE FILE - TRY AGAIN",
                  "A final replacement failure is reported through the persistence result");
            check(testing::sameScores(replacement.entries(), testing::legacyScores()),
                  "A final replacement failure leaves retained memory unchanged");
            check(testing::scoreFileBytes(replaceDirectory / "leaderboard.txt") == originalBytes &&
                  testing::sameScores(Scores(replaceDirectory).entries(), testing::legacyScores()),
                  "A final replacement failure preserves prior file bytes and readable entries");
        }
        const auto retry = replacement.submit({"RETRY", 8000, 8, 170, false});
        check(retry.ok && retry.rank == 2 && replacement.entries().size() == 5 &&
              testing::sameScores(Scores(replaceDirectory).entries(), replacement.entries()),
              "A final replacement failure retries successfully after the reader releases the file");
#endif

        fs::create_directories(root / "unknown-format");
        std::ofstream(root / "unknown-format/leaderboard.txt") << "STARFALL_SCORES_V99\n\"TEST\" 900 2 30 0\n";
        Scores unknown(root / "unknown-format");
        check(unknown.entries().empty() && !unknown.warning().empty(), "Unknown file versions produce a visible warning");
        fs::create_directories(root / "unreadable/leaderboard.txt");
        Scores unreadable(root / "unreadable");
        check(unreadable.entries().empty() && !unreadable.warning().empty(), "An unreadable score path produces a visible warning");
        std::cout << "PASS: " << passed << " gameplay and persistence checks\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
