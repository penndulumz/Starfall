#define SDL_MAIN_HANDLED
#include "app/Game.hpp"
#include "support/RunFixture.hpp"
#include "support/ScoreChecks.hpp"
#include "support/ScoreFileBlocker.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace starfall::testing {

// Test-only controlled setup; assertions observe real input dispatch, navigation,
// persistence and the const Run view. No scenario logic is linked into Starfall.
class ApplicationScenarios {
  public:
    static int expectResourceFailure(const std::string& resource) {
        try {
            Game game(std::filesystem::temp_directory_path() / "starfall-resource-failure", true);
            game.initialize();
        } catch (const std::exception& error) {
            const std::string message = error.what();
            if (message.find(resource) != std::string::npos && message.find("Restore") != std::string::npos) {
                std::cout << "PASS: useful startup failure for " << resource << '\n';
                return 0;
            }
            throw std::runtime_error("Unexpected startup failure: " + message);
        }
        throw std::runtime_error("Missing required resource must fail startup: " + resource);
    }

    static int run(const std::filesystem::path& directory, const std::filesystem::path& legacyFixture) {
        std::filesystem::create_directories(directory);
        const auto unique = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        scoreCompatibility(directory / ("compatibility-" + unique), legacyFixture, directory);
        menuPresentation(directory / ("menus-" + unique), directory);
        combatPresentation(directory / ("presentation-" + unique), directory);
        hostilePresentation(directory / ("hostiles-" + unique), directory);
        Game game(directory / ("scores-" + unique), true);
        game.initialize();
        auto& setup = RunFixture::state(game.run);
        auto& screen = game.screen;
        auto& saved = game.saved;
        auto& scores = game.scores;
        auto& uiTime = game.uiTime;
        auto& mouse = game.mouse;
        const auto& dataDirectory = game.dataDirectory;
        const auto& smokeDirectory = directory;
        auto event = [&](const SDL_Event& input) { game.event(input); };
        auto action = [&](Action input) { game.action(input); };
        auto advance = [&](float dt, const Input& input) { game.advance(dt, input); };
        auto camera = [&]() { return game.camera(); };
        auto screenshot = [&](const std::string& filename) { capture(game, directory / filename); };

        auto require = [](bool condition, const char* message) {
            if (!condition)
                throw std::runtime_error(message);
        };
        auto press = [&](SDL_Keycode key) {
            SDL_Event value{};
            value.type = SDL_KEYDOWN;
            value.key.keysym.sym = key;
            event(value);
        };
        // Red regression: Start and Pause previously depended on the prior render.
        press(SDLK_RETURN);
        require(screen == Screen::Playing, "Enter before first render must start a run");
        SDL_Event immediate{};
        immediate.type = SDL_MOUSEBUTTONDOWN;
        immediate.button.button = SDL_BUTTON_LEFT;
        immediate.button.x = 1180;
        immediate.button.y = 40;
        event(immediate);
        require(screen == Screen::Paused, "Pause immediately after Start must work before rendering");
        immediate.button.x = 500;
        immediate.button.y = 435;
        event(immediate);
        require(screen == Screen::Controls, "Help must open without a pause render");
        press(SDLK_ESCAPE);
        require(screen == Screen::Paused, "Help must return to pause without rendering");
        press(SDLK_RETURN);
        require(screen == Screen::Playing, "Resume must work immediately after Help returns");
        // Independent visual expectation from the reference player art: its
        // cockpit is green, unlike the old cyan procedural player's cockpit.
        game.render();
        const SDL_Rect cockpitPixel{640, 400, 1, 1};
        Uint32 cockpit = 0;
        require(SDL_RenderReadPixels(game.renderer, &cockpitPixel, SDL_PIXELFORMAT_RGBA8888, &cockpit,
                                     sizeof(cockpit)) == 0,
                "Gameplay cockpit sample must be readable");
        const int cockpitRed = (cockpit >> 24) & 255;
        const int cockpitGreen = (cockpit >> 16) & 255;
        const int cockpitBlue = (cockpit >> 8) & 255;
        require(cockpitGreen > cockpitRed + 20 && cockpitGreen > cockpitBlue + 40,
                "Gameplay must render the reference player's green cockpit");
        const Vec projectilePosition = game.run.view().player + Vec{200, 0};
        setup.bullets.push_back({projectilePosition, projectilePosition, {0, -880}, 1, false});
        game.render();
        const SDL_Rect projectilePixel{840, 400, 1, 1};
        Uint32 projectile = 0;
        require(SDL_RenderReadPixels(game.renderer, &projectilePixel, SDL_PIXELFORMAT_RGBA8888, &projectile,
                                     sizeof(projectile)) == 0,
                "Friendly projectile sample must be readable");
        const int projectileRed = (projectile >> 24) & 255;
        const int projectileGreen = (projectile >> 16) & 255;
        const int projectileBlue = (projectile >> 8) & 255;
        require(projectileRed > projectileGreen + 40 && projectileBlue > projectileGreen + 40,
                "Friendly fire must use the reference pink projectile cell");
        action(Action::Menu);
        uiTime = 2;
        screenshot("01-welcome.bmp");
        press(SDLK_RETURN);
        require(screen == Screen::Playing, "Enter did not start the game");
        game.run.start(1909);
        setup.wave = 4;
        setup.queuedEnemies = 12;
        setup.waveDelay = 0;
        for (int i = 0; i < 960 && screen == Screen::Playing; ++i) {
            float t = i / 120.f;
            Vec target = game.run.view().enemies.empty() ? game.run.view().player + Vec{0, -100}
                                                         : game.run.view().enemies.front().position;
            advance(1.f / 120, {{std::sin(t), std::cos(t)}, target, true, i == 500});
            mouse = target - camera();
            uiTime += 1.f / 120;
        }
        require(screen == Screen::Playing, "Smoke-test pilot died before input checks");
        screenshot("02-gameplay.bmp");
        SDL_Event hudClick{};
        hudClick.type = SDL_MOUSEBUTTONDOWN;
        hudClick.button.button = SDL_BUTTON_LEFT;
        hudClick.button.x = 1180;
        hudClick.button.y = 40;
        event(hudClick);
        require(screen == Screen::Paused, "HUD Pause button did not pause");
        press(SDLK_ESCAPE);
        press(SDLK_ESCAPE);
        require(screen == Screen::Paused, "Escape did not pause");
        float pausedAt = game.run.view().elapsed;
        advance(.1f, {{1, 0}, {0, 0}, true});
        require(game.run.view().elapsed == pausedAt, "Pause did not freeze simulation");
        screenshot("03-paused.bmp");
        action(Action::Help);
        screenshot("04-controls.bmp");
        action(Action::Back);
        require(screen == Screen::Paused, "Controls did not return to pause");
        action(Action::Resume);
        SDL_Event focusLost{};
        focusLost.type = SDL_WINDOWEVENT;
        focusLost.window.event = SDL_WINDOWEVENT_FOCUS_LOST;
        event(focusLost);
        require(screen == Screen::Paused, "Losing window focus did not pause");
        action(Action::Resume);
        setup.health = 1;
        setup.invulnerable = 0;
        setup.bullets.push_back({game.run.view().player, game.run.view().player, {}, 2, true});
        advance(.02f, {{}, game.run.view().player + Vec{0, -100}});
        require(screen == Screen::End && game.run.view().result == Result::Defeat,
                "Defeat did not open end screen");
        SDL_Event typed{};
        typed.type = SDL_TEXTINPUT;
        std::copy_n("NOVA", 5, typed.text.text);
        event(typed);
        game.sound = std::make_unique<Sound>(true);
        require(game.sound->available(), "Application audio fixture must initialize");
        SDL_Event muteClick{};
        muteClick.type = SDL_MOUSEBUTTONDOWN;
        muteClick.button.button = SDL_BUTTON_LEFT;
        muteClick.button.x = 1100;
        muteClick.button.y = 770;
        event(muteClick);
        require(!game.sound->enabled(), "Sound button must mute while entering a name");
        press(SDLK_m);
        require(!game.sound->enabled(), "Typing M on the end screen must not toggle sound");
        event(muteClick);
        require(game.sound->enabled(), "Sound button must unmute while entering a name");
        game.nameSelected = false;
        SDL_Event nameClick{};
        nameClick.type = SDL_MOUSEBUTTONDOWN;
        nameClick.button.button = SDL_BUTTON_LEFT;
        nameClick.button.x = 120;
        nameClick.button.y = 399;
        event(nameClick);
        require(!game.nameSelected, "A click above the rendered name field must not select it");
        nameClick.button.y = 410;
        event(nameClick);
        require(game.nameSelected, "Clicking the visible name field must select it");
        screenshot("05-defeat.bmp");
        press(SDLK_RETURN);
        require(saved, "End-screen score did not save");
        std::size_t count = scores->entries().size();
        press(SDLK_RETURN);
        require(scores->entries().size() == count, "Score was saved twice");
        Scores reloaded(dataDirectory);
        require(!reloaded.entries().empty() && reloaded.entries().front().name == "NOVA",
                "Saved score did not survive reload");
        action(Action::Board);
        for (int i = 0; i < 9; ++i)
            scores->submit({"PILOT-" + number(i + 1, 2), std::max(0, game.run.view().score - 25 * (i + 1)),
                            1 + i, 30 + i * 20, false});
        screenshot("06-leaderboard.bmp");
        action(Action::Back);
        require(screen == Screen::End && saved, "Leaderboard did not return to completed run");
        action(Action::Start);
        require(game.run.view().score == 0 && game.run.view().health == 5, "Retry did not reset the run");
        setup.wave = 10;
        setup.queuedEnemies = 5;
        setup.waveDelay = 0;
        advance(.02f, {{}, game.run.view().player + Vec{0, -100}});
        require(!game.run.view().enemies.empty() && game.run.view().enemies.front().type == EnemyType::Boss,
                "Dreadnought did not spawn");
        setup.enemies.front().position = game.run.view().player + Vec{190, -180};
        screenshot("08-dreadnought.bmp");
        setup.enemies.clear();
        setup.bullets.clear();
        setup.wave = 10;
        setup.queuedEnemies = 0;
        setup.waveDelay = 0;
        setup.score = 12000;
        advance(.02f, {{}, game.run.view().player + Vec{0, -100}});
        require(screen == Screen::End && game.run.view().result == Result::Victory,
                "Victory did not open end screen");
        screenshot("07-victory.bmp");
        action(Action::Menu);
        SDL_Event click{};
        click.type = SDL_MOUSEBUTTONDOWN;
        click.button.button = SDL_BUTTON_LEFT;
        click.button.x = 200;
        click.button.y = 510;
        event(click);
        require(screen == Screen::Leaderboard, "Mouse did not open leaderboard");
        action(Action::Back);
        require(screen == Screen::Welcome, "Leaderboard did not return to welcome screen");
        // SDL scales queued mouse events after a resize; event() must not scale twice.
        SDL_SetWindowSize(game.window, 800, 600);
        SDL_Event queued{};
        while (SDL_PollEvent(&queued)) {
        }
        SDL_Event resizedClick{};
        resizedClick.type = SDL_MOUSEBUTTONDOWN;
        resizedClick.button.windowID = SDL_GetWindowID(game.window);
        resizedClick.button.button = SDL_BUTTON_LEFT;
        resizedClick.button.x = 125;
        resizedClick.button.y = 369;
        require(SDL_PushEvent(&resizedClick) == 1, "Resize scenario event must enter SDL queue");
        while (SDL_PollEvent(&queued))
            event(queued);
        require(screen == Screen::Leaderboard,
                "Letterboxed click must select the visible leaderboard control");
        press(SDLK_ESCAPE);
        press(SDLK_DOWN);
        press(SDLK_DOWN);
        press(SDLK_RETURN);
        require(screen == Screen::Controls, "Keyboard navigation must work on a newly returned screen");
        press(SDLK_ESCAPE);
        press(SDLK_F11);
        if (game.fullscreen) {
            // This verifies SDL's fullscreen coordinate route with the dummy backend;
            // real display sizing remains a separate interactive acceptance check.
            int fullscreenX = 0;
            int fullscreenY = 0;
            SDL_RenderLogicalToWindow(game.renderer, 200, 510, &fullscreenX, &fullscreenY);
            SDL_Event fullscreenClick{};
            fullscreenClick.type = SDL_MOUSEBUTTONDOWN;
            fullscreenClick.button.windowID = SDL_GetWindowID(game.window);
            fullscreenClick.button.button = SDL_BUTTON_LEFT;
            fullscreenClick.button.x = fullscreenX;
            fullscreenClick.button.y = fullscreenY;
            require(SDL_PushEvent(&fullscreenClick) == 1, "Fullscreen scenario event must enter SDL queue");
            while (SDL_PollEvent(&queued))
                event(queued);
            require(screen == Screen::Leaderboard,
                    "Fullscreen click must select the visible leaderboard control");
            press(SDLK_ESCAPE);
            press(SDLK_F11);
            require(!game.fullscreen, "F11 must return to a window after successful fullscreen entry");
        }
        SDL_SetWindowSize(game.window, ScreenWidth, ScreenHeight);
        while (SDL_PollEvent(&queued))
            event(queued);
        game.sound = std::make_unique<Sound>(false);
        SDL_SetHint(SDL_HINT_AUDIODRIVER, "dummy");
        {
            Sound audioProbe(true);
            require(audioProbe.warningKind() == AudioWarningKind::None && audioProbe.warning().empty(),
                    "Successful startup must preserve normal footer guidance without an audio warning");
            require(audioProbe.available() && audioProbe.enabled(),
                    "SDL audio did not initialize with the test driver");
            for (auto type : {EventType::Shot, EventType::Hit, EventType::Explosion, EventType::Pickup,
                              EventType::Dash, EventType::Wave, EventType::Win, EventType::Lose})
                audioProbe.play(type);
            SDL_Delay(40);
            audioProbe.toggle();
            require(!audioProbe.enabled(), "Sound toggle did not mute");
            audioProbe.toggle();
            require(audioProbe.enabled(), "Sound toggle did not unmute");
        }
        // A missing required clip must be detected even though the game can keep
        // running without audio. Mutate only this test build's staged resource.
        char* executableBase = SDL_GetBasePath();
        require(executableBase != nullptr, "Application audio test needs executable-relative resources");
        const auto shootingClip = std::filesystem::u8path(executableBase) / "resources/reference/shoot.flac";
        SDL_free(executableBase);
        const auto savedClip = directory / ("shoot-original-" + unique + ".flac");
        std::filesystem::rename(shootingClip, savedClip);
        try {
            game.sound = std::make_unique<Sound>(true);
            require(!game.sound->available(), "Missing shooting clip must disable audio without stopping play");
            require(game.sound->warningKind() == AudioWarningKind::Resource,
                    "Missing shooting clip must request package recovery by category");
            require(game.sound->warning().find("shoot.flac") != std::string::npos,
                    "Missing audio diagnostic must identify its packaged resource");
            action(Action::Menu);
            press(SDLK_RETURN);
            require(screen == Screen::Playing, "Missing audio must leave a run playable");
            screenshot("09-audio-resource-error.bmp");
        } catch (...) {
            std::filesystem::rename(savedClip, shootingClip);
            throw;
        }
        std::filesystem::rename(savedClip, shootingClip);
        std::filesystem::rename(shootingClip, savedClip);
        std::ofstream(shootingClip, std::ios::binary) << "fLaC invalid encoded audio";
        try {
            game.sound = std::make_unique<Sound>(true);
            require(!game.sound->available(), "Corrupt shooting FLAC must fail decoding instead of using a generated shot");
            require(game.sound->warningKind() == AudioWarningKind::Resource,
                    "Corrupt FLAC must request package recovery by category");
            require(game.sound->warning().find("cannot decode") != std::string::npos,
                    "Corrupt FLAC must report a decoder failure rather than a device problem");
        } catch (...) {
            std::filesystem::remove(shootingClip);
            std::filesystem::rename(savedClip, shootingClip);
            throw;
        }
        std::filesystem::remove(shootingClip);
        std::filesystem::rename(savedClip, shootingClip);
        game.sound = std::make_unique<Sound>(true);
        require(game.sound->available() && game.sound->warning().empty() &&
                    game.sound->warningKind() == AudioWarningKind::None,
                "Restoring the clip must allow the next audio initialization to succeed");
        game.sound.reset();
        const char* previousDriver = SDL_getenv("SDL_AUDIODRIVER");
        const std::string previousAudioDriver = previousDriver ? previousDriver : "dummy";
        SDL_setenv("SDL_AUDIODRIVER", "starfall-nonexistent-audio-driver", 1);
        try {
            game.sound = std::make_unique<Sound>(true);
            require(!game.sound->available() && !game.sound->enabled(),
                    "An unavailable audio driver must leave sound unavailable");
            require(game.sound->warningKind() == AudioWarningKind::DeviceUnavailable,
                    "Device failure must request device recovery independently of diagnostic wording");
            require(!game.sound->warning().empty(),
                    "Device failure must be distinguished from corrupt packaged audio");
            action(Action::Menu);
            press(SDLK_RETURN);
            require(screen == Screen::Playing, "A missing audio device must leave gameplay usable");
            screenshot("10-audio-device-unavailable.bmp");
        } catch (...) {
            SDL_setenv("SDL_AUDIODRIVER", previousAudioDriver.c_str(), 1);
            throw;
        }
        SDL_setenv("SDL_AUDIODRIVER", previousAudioDriver.c_str(), 1);
        game.sound = std::make_unique<Sound>(true);
        require(game.sound->available() && game.sound->warningKind() == AudioWarningKind::None &&
                    game.sound->warning().empty(),
                "Restoring the audio driver must remove device recovery advice");
        screenshot("11-audio-restored.bmp");
        action(Action::Menu);
        press(SDLK_DOWN);
        press(SDLK_DOWN);
        press(SDLK_DOWN);
        press(SDLK_RETURN);
        require(!game.running, "Keyboard Quit must close the welcome flow");
        std::ofstream report(smokeDirectory / "smoke-test.txt");
        report << "PASS: menu name-field focus, long names, empty/full leaderboards, outside-top-ten "
                  "confirmation, disabled Save, six smaller-window screen families, "
                  "pre-overhaul score compatibility, failed-save retry, separate identical runs, "
                  "unsaved menu/retry/abandon, pre-render start/pause/help, exact name hit region, resized/fullscreen mouse input, "
                  "name-entry sound button, welcome, keyboard start, simulation, mouse pause, pause freeze, "
                  "controls/back, focus-loss pause, defeat, name entry, save once, persistence reload, full "
                  "leaderboard/back, retry, Dreadnought spawn/render, victory, mouse navigation, audio "
                  "FLAC mixing and mute/unmute, missing/corrupt clip diagnostics, restored-resource "
                  "recovery, and playable missing-device fallback.\n";
        return 0;
    }

  private:
    static void hostilePresentation(const std::filesystem::path& dataDirectory,
                                    const std::filesystem::path& captures) {
        Game game(dataDirectory, true);
        game.initialize();
        game.action(Action::Start);
        auto& setup = RunFixture::state(game.run);
        setup.queuedEnemies = 1;
        setup.waveDelay = 100;
        setup.announcement = 0;
        Enemy scout;
        scout.position = setup.player + Vec{0, 140};
        setup.enemies = {scout};
        game.render();
        // Original Scout cell has gray-brown hull at its center. The old
        // procedural Scout instead has a cyan/pink cockpit at this position.
        const SDL_Rect hullPixel{640, 540, 1, 1};
        Uint32 hull = 0;
        if (SDL_RenderReadPixels(game.renderer, &hullPixel, SDL_PIXELFORMAT_RGBA8888, &hull,
                                 sizeof(hull)) < 0 || hull != 0x302c2eff) {
            throw std::runtime_error("Scout must display the documented reference hull artwork");
        }
        capture(game, captures / "15-scout-atlas.bmp");
        // These colors are literal samples of the verified source artwork,
        // independent of Draw's atlas-selection code.
        struct HostileSample {
            EnemyType type;
            int health;
            float radius;
            int sampleY;
            Uint32 color;
        };
        const std::array<HostileSample, 3> samples{{
            {EnemyType::Gunner, 3, 23, 540, 0xa05b53ff},
            {EnemyType::Tank, 9, 29, 525, 0x71aa34ff},
            {EnemyType::Boss, 110, 66, 520, 0x8aebf1ff},
        }};
        for (const auto& sample : samples) {
            setup.enemies.front().type = sample.type;
            setup.enemies.front().health = setup.enemies.front().maxHealth = sample.health;
            setup.enemies.front().radius = sample.radius;
            game.render();
            const SDL_Rect cockpitPixel{640, sample.sampleY, 1, 1};
            Uint32 cockpit = 0;
            if (SDL_RenderReadPixels(game.renderer, &cockpitPixel, SDL_PIXELFORMAT_RGBA8888, &cockpit,
                                     sizeof(cockpit)) < 0 || cockpit != sample.color) {
                throw std::runtime_error("Every hostile must use its documented atlas cell and orientation");
            }
        }
        for (int remainingHull = 0; remainingHull <= 5; ++remainingHull) {
            setup.health = remainingHull;
            game.render();
            // Five equal-capacity slots, from the original fill's 120-pixel
            // interior at 2x scale. Empty slots must expose the dark frame.
            for (int segment = 0; segment < 5; ++segment) {
                const SDL_Rect segmentPixel{66 + segment * 48, 58, 1, 1};
                Uint32 pixel = 0;
                const Uint32 expected = segment < remainingHull ? 0xe1434bff : 0x37020fff;
                if (SDL_RenderReadPixels(game.renderer, &segmentPixel, SDL_PIXELFORMAT_RGBA8888, &pixel,
                                         sizeof(pixel)) < 0 || pixel != expected) {
                    throw std::runtime_error("Hull must clip the imported fill to five equal-capacity segments");
                }
            }
            capture(game, captures / ("16-hull-" + std::to_string(remainingHull) + ".bmp"));
        }
        setup.wave = 10;
        setup.queuedEnemies = 0;
        setup.score = 18450;
        setup.chain = 15;
        setup.elapsed = 316;
        setup.dashCooldown = 1.3f;
        setup.overdrive = 6.2f;
        setup.enemies = {
            {setup.player + Vec{-250, 0}, EnemyType::Scout, 2, 3, 19},
            {setup.player + Vec{0, -180}, EnemyType::Gunner, 2, 3, 23},
            {setup.player + Vec{0, 180}, EnemyType::Tank, 6, 9, 29},
            {setup.player + Vec{260, 0}, EnemyType::Boss, 55, 110, 66},
            {setup.player + Vec{1000, -500}, EnemyType::Scout, 3, 3, 19},
        };
        const Vec friendly = setup.player + Vec{-70, -100};
        const Vec hostile = setup.player + Vec{100, 120};
        setup.bullets = {{friendly, friendly, {0, -880}, 1, false},
                         {friendly + Vec{16, 0}, friendly + Vec{16, 0}, {0, -880}, 1, false},
                         {hostile, hostile, {-100, -100}, 1, true}};
        game.mouse = {700, 300};
        capture(game, captures / "17-hostiles-and-hud.bmp");

        // Let the real final-wave scheduler create all five hostiles. Fixtures
        // reposition them only for visibility; damage uses actual collisions.
        game.action(Action::Start);
        game.run.start(17);
        setup.wave = 10;
        setup.queuedEnemies = 5;
        setup.waveDelay = 0;
        for (int step = 0; step < 1200 && setup.queuedEnemies > 0; ++step) {
            setup.invulnerable = 2;
            game.advance(Rules::FixedStep, {{}, setup.player + Vec{0, -100}});
        }
        if (game.run.view().enemies.size() != 5 || game.run.view().enemies.front().type != EnemyType::Boss ||
            game.run.remaining() != 5 || game.screen != Screen::Playing) {
            throw std::runtime_error("The final encounter must include the Dreadnought and four supports");
        }
        const std::array<Vec, 5> positions{{{250, -100}, {-240, -80}, {-260, 150}, {180, 170}, {1000, 400}}};
        for (std::size_t index = 0; index < positions.size(); ++index) {
            setup.enemies[index].position = setup.player + positions[index];
            setup.enemies[index].cooldown = 100;
        }
        setup.bullets.clear();
        setup.particles.clear();
        setup.shake = 0;
        setup.announcement = 2;
        capture(game, captures / "18-final-wave.bmp");
        setup.announcement = 0;
        auto hitFrontHostile = [&](int shots) {
            const Vec target = game.run.view().enemies.front().position;
            setup.bullets.clear();
            for (int shot = 0; shot < shots; ++shot) {
                setup.bullets.push_back({target, target, {}, 1, false});
            }
            game.advance(Rules::FixedStep, {{}, setup.player + Vec{0, -100}});
        };
        hitFrontHostile(55);
        if (game.run.view().enemies.front().health != 55 || game.run.remaining() != 5) {
            throw std::runtime_error("Dreadnought durability must decrease through real projectile hits");
        }
        setup.particles.clear();
        setup.shake = 0;
        capture(game, captures / "19-dreadnought-half-hull.bmp");
        hitFrontHostile(55);
        if (game.run.view().result != Result::Active || game.screen != Screen::Playing ||
            game.run.remaining() != 4 || game.run.view().score != 5000 ||
            std::any_of(game.run.view().enemies.begin(), game.run.view().enemies.end(),
                        [](const Enemy& enemy) { return enemy.type == EnemyType::Boss; })) {
            throw std::runtime_error("Destroying the Dreadnought must leave its supporting hostiles active");
        }
        setup.particles.clear();
        setup.shake = 0;
        capture(game, captures / "20-supports-remain.bmp");
        for (int support = 0; support < 3; ++support) {
            hitFrontHostile(game.run.view().enemies.front().health);
        }
        if (game.run.view().result != Result::Active || game.run.remaining() != 1) {
            throw std::runtime_error("The final surviving support must prevent early victory");
        }
        const auto lastType = game.run.view().enemies.front().type;
        const int baseScore = lastType == EnemyType::Tank ? 350 : lastType == EnemyType::Gunner ? 200 : 100;
        const int beforeVictory = game.run.view().score;
        hitFrontHostile(game.run.view().enemies.front().health);
        // The fifth continuing kill has 2x score; final-wave clear + victory +
        // five retained hull segments add 1000 + 5000 + 1250 independently.
        if (game.screen != Screen::End || game.run.view().result != Result::Victory ||
            game.run.remaining() != 0 || game.run.view().kills != 5 ||
            game.run.view().score != beforeVictory + baseScore * 2 + 7250) {
            throw std::runtime_error("Clearing the whole final wave must open victory with unchanged bonuses");
        }
        capture(game, captures / "21-whole-wave-victory.bmp");
    }

    static void menuPresentation(const std::filesystem::path& dataDirectory,
                                 const std::filesystem::path& captures) {
        auto require = [](bool condition, const char* message) {
            if (!condition) throw std::runtime_error(message);
        };
        Game game(dataDirectory, true);
        game.initialize();
        auto press = [&](SDL_Keycode key) {
            SDL_Event event{};
            event.type = SDL_KEYDOWN;
            event.key.keysym.sym = key;
            game.event(event);
        };
        game.action(Action::Board);
        require(game.scores->entries().empty(), "A new profile must show an empty leaderboard");
        capture(game, captures / "11-empty-leaderboard.bmp");
        press(SDLK_ESCAPE);
        require(game.screen == Screen::Welcome, "The empty leaderboard must return to welcome");
        for (int i = 0; i < 10; ++i) {
            require(game.scores->submit({"MAX-NAME_123", 1000000000 - i * 100, 10, 86400, i % 2 == 0}).ok,
                    "The menu fixture must accept maximum supported display values");
        }
        game.action(Action::Start);
        auto& setup = RunFixture::state(game.run);
        setup.health = 1;
        setup.invulnerable = 0;
        setup.waveDelay = 100;
        setup.queuedEnemies = 1;
        setup.bullets.push_back({setup.player, setup.player, {}, 2, true});
        game.advance(.02f, {{}, setup.player + Vec{0, -100}});
        require(game.screen == Screen::End, "The name-entry menu check requires a completed run");

        // A player may select another action, then return to the visible name field.
        // Clicking that field must make its advertised Enter-to-Save action reliable.
        press(SDLK_DOWN);
        press(SDLK_DOWN);
        SDL_Event nameClick{};
        nameClick.type = SDL_MOUSEBUTTONDOWN;
        nameClick.button.button = SDL_BUTTON_LEFT;
        nameClick.button.x = 200;
        nameClick.button.y = 430;
        game.event(nameClick);
        SDL_Event typed{};
        typed.type = SDL_TEXTINPUT;
        std::copy_n("LONG-NAME_12", 13, typed.text.text);
        game.event(typed);
        game.event(nameClick);
        capture(game, captures / "12-long-name.bmp");
        press(SDLK_RETURN);
        require(game.screen == Screen::End && game.saved && game.pilotName == "LONG-NAME_12",
                "Clicking the name field then Enter must save, even after selecting another action");
        require(game.savedRank == 0 && game.saveMessage == "OUTSIDE THE TOP 10",
                "An outside-top-ten result must confirm submission without a retained rank");
        capture(game, captures / "13-outside-top-ten.bmp");
        std::copy_n("CHANGED", 8, typed.text.text);
        game.event(typed);
        press(SDLK_RETURN);
        require(game.pilotName == "LONG-NAME_12" && game.scores->entries().size() == 10,
                "A saved name and disabled Save action must not change or submit the run again");

        game.action(Action::Menu);
        capture(game, captures / "14-audio-unavailable.bmp");
        // Capture the actual smaller renderer output rather than a clipped logical-size surface.
        SDL_SetWindowSize(game.window, 800, 500);
        SDL_Event queued{};
        while (SDL_PollEvent(&queued)) game.event(queued);
        capture(game, captures / "16-welcome-small.bmp");
        game.action(Action::Help);
        capture(game, captures / "17-controls-small.bmp");
        press(SDLK_ESCAPE);
        game.action(Action::Start);
        game.action(Action::Pause);
        capture(game, captures / "18-paused-small.bmp");
        game.action(Action::Help);
        press(SDLK_ESCAPE);
        require(game.screen == Screen::Paused, "Small-window controls must return to pause");
        game.action(Action::Menu);
        game.action(Action::Board);
        capture(game, captures / "19-leaderboard-small.bmp");
        press(SDLK_ESCAPE);
        require(game.screen == Screen::Welcome, "Small-window leaderboard must return to welcome");
        game.action(Action::Start);
        setup.health = 1;
        setup.invulnerable = 0;
        setup.waveDelay = 100;
        setup.queuedEnemies = 1;
        setup.bullets.push_back({setup.player, setup.player, {}, 2, true});
        game.advance(.02f, {{}, setup.player + Vec{0, -100}});
        require(game.screen == Screen::End && game.run.view().result == Result::Defeat,
                "A small-window defeat must reach its end screen");
        capture(game, captures / "20-defeat-small.bmp");
        game.action(Action::Start);
        setup.wave = Rules::FinalWave;
        setup.queuedEnemies = 0;
        setup.waveDelay = 0;
        game.advance(.02f, {{}, setup.player + Vec{0, -100}});
        require(game.screen == Screen::End && game.run.view().result == Result::Victory,
                "A small-window victory must reach its end screen");
        capture(game, captures / "21-victory-small.bmp");
    }

    static void combatPresentation(const std::filesystem::path& dataDirectory,
                                   const std::filesystem::path& captures) {
        Game game(dataDirectory, true);
        game.initialize();
        game.action(Action::Start);
        auto& setup = RunFixture::state(game.run);
        setup.waveDelay = 100;
        setup.queuedEnemies = 1;
        setup.announcement = 0;
        setup.pickups = {{setup.player + Vec{-140, 90}, PickupType::Repair},
                         {setup.player + Vec{140, 90}, PickupType::Overdrive}};
        const std::array<Vec, 4> directions{{{0, -1}, {1, 0}, {0, 1}, {-1, 0}}};
        for (std::size_t index = 0; index < directions.size(); ++index) {
            const Vec direction = directions[index];
            setup.facing = direction;
            setup.bullets.clear();
            const Vec position = setup.player + direction * 130;
            setup.bullets.push_back({position, position, direction * Rules::ShotSpeed, 1, false});
            // Keep a hostile shot visible for color/shape distinction.
            const Vec hostile = setup.player + Vec{-160, -120};
            setup.bullets.push_back({hostile, hostile, {100, 100}, 1, true});
            game.mouse = setup.player + direction * 230 - game.camera();
            capture(game, captures / ("11-flight-direction-" + std::to_string(index) + ".bmp"));
        }
        setup.bullets.clear();
        setup.overdrive = Rules::OverdriveDuration;
        game.advance(Rules::FixedStep, {{}, setup.player + Vec{180, -180}, true, true});
        if (game.run.view().bullets.size() != 2 || game.run.view().dashCooldown <= 0) {
            throw std::runtime_error(
                "Overdrive and dash presentation must use real paired shots and recharge");
        }
        game.uiTime = 0;
        for (int step = 0; step < 24; ++step) {
            game.advance(Rules::FixedStep, {{}, setup.player + Vec{180, -180}, false, false});
        }
        capture(game, captures / "12-dash-overdrive.bmp");

        setup.invulnerable = 0;
        setup.shake = 0;
        setup.bullets.clear();
        setup.pickups.clear();
        setup.dashTime = 0;
        setup.player = {Rules::PlayerMargin, Rules::PlayerMargin};
        game.advance(Rules::FixedStep, {{-1, -1}, setup.player + Vec{150, 50}, true});
        if (game.camera().x != 0 || game.camera().y != 0 || game.run.view().player.x < Rules::PlayerMargin) {
            throw std::runtime_error("Top-left presentation must preserve camera and arena bounds");
        }
        // Show the bounded view with the ship clear of its fixed HUD overlays.
        setup.player = {140, 140};
        capture(game, captures / "13-arena-top-left.bmp");
        setup.player = {Rules::ArenaWidth - Rules::PlayerMargin, Rules::ArenaHeight - Rules::PlayerMargin};
        setup.dashTime = 0;
        game.advance(Rules::FixedStep, {{1, 1}, setup.player - Vec{150, 50}, true});
        if (game.camera().x != Rules::ArenaWidth - ScreenWidth ||
            game.camera().y != Rules::ArenaHeight - ScreenHeight ||
            game.run.view().player.x > Rules::ArenaWidth - Rules::PlayerMargin) {
            throw std::runtime_error("Bottom-right presentation must preserve camera and arena bounds");
        }
        setup.player = {Rules::ArenaWidth - 280, Rules::ArenaHeight - 180};
        capture(game, captures / "14-arena-bottom-right.bmp");
    }

    static void scoreCompatibility(const std::filesystem::path& dataDirectory,
                                   const std::filesystem::path& legacyFixture,
                                   const std::filesystem::path& captures) {
        auto require = [](bool condition, const char* message) {
            if (!condition)
                throw std::runtime_error(message);
        };
        std::filesystem::create_directories(dataDirectory);
        std::filesystem::copy_file(legacyFixture, dataDirectory / "leaderboard.txt");
        std::vector<Score> savedEntries;
        {
            Game game(dataDirectory, true);
            game.initialize();
            require(game.dataDirectory == dataDirectory,
                    "An explicit test save directory must remain independent of packaged resources");
            require(sameScores(game.scores->entries(), legacyScores()),
                    "Application startup must preserve every pre-overhaul V1 score field and tie order");
            auto press = [&](SDL_Keycode key) {
                SDL_Event event{};
                event.type = SDL_KEYDOWN;
                event.key.keysym.sym = key;
                game.event(event);
            };
            auto completedRun = [&](int points) {
                game.action(Action::Start);
                auto& setup = RunFixture::state(game.run);
                setup.score = points;
                setup.wave = 4;
                setup.elapsed = 95;
                setup.health = 1;
                setup.invulnerable = 0;
                setup.waveDelay = 100;
                setup.queuedEnemies = 1;
                setup.bullets.push_back({setup.player, setup.player, {}, 2, true});
                game.advance(.02f, {{}, setup.player + Vec{0, -100}});
                require(game.screen == Screen::End && game.run.view().result == Result::Defeat,
                        "A completed compatibility run must open the real end screen");
                SDL_Event typed{};
                typed.type = SDL_TEXTINPUT;
                std::copy_n("RETURNING", 10, typed.text.text);
                game.event(typed);
            };

            game.action(Action::Board);
            capture(game, captures / "09-legacy-leaderboard.bmp");
            press(SDLK_ESCAPE);
            require(game.screen == Screen::Welcome, "Legacy leaderboard must return to welcome");
            completedRun(2000);
#ifdef _WIN32
            const auto originalFile = scoreFileBytes(dataDirectory / "leaderboard.txt");
            {
                ScoreFileBlocker heldReader(dataDirectory / "leaderboard.txt");
                press(SDLK_RETURN);
                require(!game.saved && !game.saveMessage.empty() &&
                            game.saveMessage == "CANNOT REPLACE SCORE FILE - TRY AGAIN",
                        "Failed replacement must be visible and leave the run available to save");
                require(sameScores(game.scores->entries(), legacyScores()) &&
                            sameScores(Scores(dataDirectory).entries(), legacyScores()) &&
                            scoreFileBytes(dataDirectory / "leaderboard.txt") == originalFile,
                        "Application save failure must preserve retained memory and previous file");
                capture(game, captures / "10-save-retry.bmp");
            }
#endif
            press(SDLK_RETURN);
            require(game.saved && game.savedRank == 4 && game.scores->entries().size() == 5,
                    "Recovered Save must retain the new result after existing tied scores");
            require(game.scores->entries()[1].name == "EARLY-TIE" &&
                        game.scores->entries()[2].name == "LATER-TIE" &&
                        game.scores->entries()[3].name == "RETURNING",
                    "Old tied entries must remain ahead of the newly saved application result");
            press(SDLK_RETURN);
            require(game.scores->entries().size() == 5, "A completed run may submit only once");
            game.action(Action::Board);
            game.action(Action::Back);
            require(game.screen == Screen::End && game.saved,
                    "Leaderboard return must preserve the completed run's submitted state");

            completedRun(2000);
            press(SDLK_RETURN);
            require(game.saved && game.savedRank == 5 && game.scores->entries().size() == 6,
                    "A different completed run may legitimately have the same pilot name and score");
            savedEntries = game.scores->entries();

            completedRun(9000);
            game.action(Action::Menu);
            require(sameScores(Scores(dataDirectory).entries(), savedEntries),
                    "Main Menu must not silently submit an unsaved completed run");
            completedRun(9000);
            game.action(Action::Start);
            require(game.screen == Screen::Playing && !game.saved && game.run.view().score == 0 &&
                        sameScores(Scores(dataDirectory).entries(), savedEntries),
                    "Play Again must reset the run without silently submitting the previous result");
            game.action(Action::Menu);
            require(sameScores(Scores(dataDirectory).entries(), savedEntries),
                    "Abandoning an active run must not write a score");
        }
        {
            Game reopened(dataDirectory, true);
            reopened.initialize();
            require(sameScores(reopened.scores->entries(), savedEntries),
                    "Reopening the application must retain old and newly saved completed runs");
        }
    }

    static void capture(Game& game, const std::filesystem::path& file) {
        game.render();
        int width = 0;
        int height = 0;
        if (SDL_GetRendererOutputSize(game.renderer, &width, &height) < 0) {
            throw std::runtime_error(SDL_GetError());
        }
        using Surface = std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>;
        Surface surface(
            SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888),
            SDL_FreeSurface);
        if (!surface || SDL_RenderReadPixels(game.renderer, nullptr, surface->format->format, surface->pixels,
                                             surface->pitch) < 0) {
            throw std::runtime_error(SDL_GetError());
        }
        if (SDL_SaveBMP(surface.get(), file.u8string().c_str()) < 0) {
            throw std::runtime_error(SDL_GetError());
        }
        SDL_RenderPresent(game.renderer);
    }
};

} // namespace starfall::testing

int main(int argc, char** argv) {
    try {
        if (argc == 3 && std::string(argv[1]) == "--expect-resource-failure") {
            return starfall::testing::ApplicationScenarios::expectResourceFailure(argv[2]);
        }
        return starfall::testing::ApplicationScenarios::run(argc > 1 ? argv[1] : "build/application-qa",
                                                            argc > 2 ? argv[2]
                                                                     : "tests/fixtures/leaderboard-v1.txt");
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
