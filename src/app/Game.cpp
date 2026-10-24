#include "Game.hpp"
#include <stdexcept>

namespace starfall {

Game::Game(std::filesystem::path data, bool hideWindow) : dataDirectory(std::move(data)), hidden(hideWindow) {
}

Game::~Game() {
    SDL_StopTextInput();
    // Resources and callbacks must be gone before their SDL owners shut down.
    draw.reset();
    sound.reset();
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}

void Game::initialize() {
    SDL_SetMainReady();
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        throw std::runtime_error(SDL_GetError());
    }
    int width = ScreenWidth;
    int height = ScreenHeight;
    SDL_DisplayMode display{};
    if (!hidden && SDL_GetCurrentDisplayMode(0, &display) == 0) {
        float scale = std::min({1.f, (display.w - 80.f) / ScreenWidth, (display.h - 120.f) / ScreenHeight});
        width = static_cast<int>(ScreenWidth * scale);
        height = static_cast<int>(ScreenHeight * scale);
    }
    window = SDL_CreateWindow("STARFALL | SDL2 Space Shooter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              width, height,
                              SDL_WINDOW_RESIZABLE | (hidden ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN));
    if (!window) {
        throw std::runtime_error(SDL_GetError());
    }
    SDL_SetWindowMinimumSize(window, 800, 500);
    renderer = SDL_CreateRenderer(
        window, -1, hidden ? SDL_RENDERER_SOFTWARE : SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!renderer || SDL_RenderSetLogicalSize(renderer, ScreenWidth, ScreenHeight) < 0) {
        throw std::runtime_error(SDL_GetError());
    }
    draw = std::make_unique<Draw>(renderer);
    sound = std::make_unique<Sound>(!hidden);
    if (dataDirectory.empty()) {
        char* path = SDL_GetPrefPath("Starfall", "Starfall");
        if (!path) {
            throw std::runtime_error(
                "Cannot find a writable save folder. Use --data-dir with a writable directory.");
        }
        dataDirectory = std::filesystem::u8path(path);
        SDL_free(path);
    }
    scores = std::make_unique<Scores>(dataDirectory);
    refreshLayout();
}

void Game::changeScreen(Screen value) {
    screen = value;
    focused = 0;
    accumulator = 0;
    refreshLayout();
    if (screen == Screen::End && !saved) {
        SDL_StartTextInput();
    } else {
        SDL_StopTextInput();
    }
    if (!hidden) {
        SDL_ShowCursor(screen == Screen::Playing ? SDL_DISABLE : SDL_ENABLE);
    }
}

void Game::startRun() {
    run.start();
    saved = false;
    savedRank = 0;
    saveMessage.clear();
    changeScreen(Screen::Playing);
    sound->play(EventType::Wave);
}

void Game::finishRun() {
    saved = false;
    savedRank = 0;
    saveMessage.clear();
    pilotName = lastName;
    nameSelected = true;
    changeScreen(Screen::End);
}

void Game::saveScore() {
    if (saved) {
        return;
    }
    const auto& state = run.view();
    const auto result = scores->submit({pilotName, state.score, state.wave, static_cast<int>(state.elapsed),
                                        state.result == Result::Victory});
    if (!result.ok) {
        saveMessage = result.error;
        return;
    }
    saved = true;
    savedRank = result.rank;
    pilotName = Scores::cleanName(pilotName);
    lastName = pilotName;
    saveMessage = savedRank > 0 ? "SAVED / RANK " + number(savedRank, 2) : "OUTSIDE THE TOP 10";
    SDL_StopTextInput();
    sound->play(EventType::Pickup);
}

void Game::action(Action value) {
    switch (value) {
    case Action::Start:
        startRun();
        break;
    case Action::Quit:
        running = false;
        break;
    case Action::Board:
        returnScreen = screen;
        changeScreen(Screen::Leaderboard);
        break;
    case Action::Help:
        returnScreen = screen;
        changeScreen(Screen::Controls);
        break;
    case Action::Back:
        changeScreen(returnScreen);
        break;
    case Action::Pause:
        changeScreen(Screen::Paused);
        break;
    case Action::Resume:
        changeScreen(Screen::Playing);
        break;
    case Action::Menu:
        changeScreen(Screen::Welcome);
        break;
    case Action::Save:
        saveScore();
        break;
    case Action::Mute:
        sound->toggle();
        break;
    }
}

void Game::refreshLayout() {
    buttons = screenLayout(screen, saved, savedRank, sound && sound->available(), sound && sound->enabled());
}

void Game::event(const SDL_Event& value) {
    refreshLayout();
    // SDL_RenderSetLogicalSize already converts queued mouse events to logical pixels.
    // Only raw SDL_GetMouseState coordinates need SDL_RenderWindowToLogical.

    if (value.type == SDL_QUIT) {
        running = false;
        return;
    }
    if (value.type == SDL_WINDOWEVENT && value.window.event == SDL_WINDOWEVENT_FOCUS_LOST &&
        screen == Screen::Playing) {
        changeScreen(Screen::Paused);
        return;
    }
    if (value.type == SDL_MOUSEMOTION) {
        mouse = {static_cast<float>(value.motion.x), static_cast<float>(value.motion.y)};
        mouseNavigation = true;
    }
    if (value.type == SDL_MOUSEBUTTONDOWN && value.button.button == SDL_BUTTON_LEFT) {
        mouse = {static_cast<float>(value.button.x), static_cast<float>(value.button.y)};
        for (const auto& item : buttons) {
            SDL_FPoint point{mouse.x, mouse.y};
            if (item.enabled && SDL_PointInFRect(&point, &item.rect)) {
                // A transition rebuilds buttons, so copy before dispatch invalidates item.
                const Action chosen = item.action;
                action(chosen);
                return;
            }
        }
        SDL_FPoint point{mouse.x, mouse.y};
        if (screen == Screen::End && !saved && SDL_PointInFRect(&point, &PilotNameField)) {
            nameSelected = true;
            // The name field advertises Enter to save; restore that action when
            // the player returns here from another keyboard-selected control.
            const auto saveButton = std::find_if(
                buttons.begin(), buttons.end(), [](const Button& item) { return item.action == Action::Save; });
            focused = static_cast<int>(saveButton - buttons.begin());
            mouseNavigation = false;
        }
    }
    if (value.type == SDL_TEXTINPUT && screen == Screen::End && !saved) {
        std::string addition;
        for (unsigned char ch : std::string(value.text.text)) {
            if (ch >= 'a' && ch <= 'z') {
                ch = static_cast<unsigned char>(ch - 'a' + 'A');
            }
            if ((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == ' ' || ch == '-' || ch == '_') {
                addition += static_cast<char>(ch);
            }
        }
        if (!addition.empty()) {
            if (nameSelected) {
                pilotName.clear();
                nameSelected = false;
            }
            pilotName += addition;
            if (pilotName.size() > 12) {
                pilotName.resize(12);
            }
        }
    }
    if (value.type != SDL_KEYDOWN || value.key.repeat) {
        return;
    }
    const auto key = value.key.keysym.sym;
    if (key == SDLK_F11) {
        const bool desired = !fullscreen;
        if (SDL_SetWindowFullscreen(window, desired ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) == 0) {
            fullscreen = desired;
        }
        return;
    }
    if (key == SDLK_m && screen != Screen::End) {
        sound->toggle();
        return;
    }
    if (key == SDLK_ESCAPE || key == SDLK_p) {
        if (screen == Screen::Playing) {
            changeScreen(Screen::Paused);
        } else if (screen == Screen::Paused) {
            changeScreen(Screen::Playing);
        } else if (key == SDLK_ESCAPE && (screen == Screen::Controls || screen == Screen::Leaderboard)) {
            changeScreen(returnScreen);
        }
        return;
    }
    if (screen == Screen::Playing) {
        return;
    }
    if (screen == Screen::End && !saved && key == SDLK_BACKSPACE) {
        if (nameSelected) {
            pilotName.clear();
            nameSelected = false;
        } else if (!pilotName.empty()) {
            pilotName.pop_back();
        }
    }
    if (key == SDLK_UP || key == SDLK_DOWN || key == SDLK_TAB) {
        mouseNavigation = false;
        int increment = key == SDLK_UP || (key == SDLK_TAB && (value.key.keysym.mod & KMOD_SHIFT)) ? -1 : 1;
        for (std::size_t i = 0; i < buttons.size(); ++i) {
            focused =
                (focused + increment + static_cast<int>(buttons.size())) % static_cast<int>(buttons.size());
            if (buttons[focused].enabled) {
                break;
            }
        }
    }
    if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
        mouseNavigation = false;
        if (focused >= 0 && focused < static_cast<int>(buttons.size()) && buttons[focused].enabled) {
            action(buttons[focused].action);
        }
    }
}

Vec Game::camera() const {
    return {std::clamp(run.view().player.x - ScreenWidth / 2.f, 0.f, ArenaWidth - ScreenWidth),
            std::clamp(run.view().player.y - ScreenHeight / 2.f, 0.f, ArenaHeight - ScreenHeight)};
}
Input Game::input() const {
    auto* keys = SDL_GetKeyboardState(nullptr);
    Input value;
    value.movement = {static_cast<float>((keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) -
                                         (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])),
                      static_cast<float>((keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) -
                                         (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]))};
    int x = 0;
    int y = 0;
    const Uint32 pressed = SDL_GetMouseState(&x, &y);
    Vec logical;
    SDL_RenderWindowToLogical(renderer, x, y, &logical.x, &logical.y);
    value.aim = logical + camera();
    value.fire = (pressed & SDL_BUTTON(SDL_BUTTON_LEFT)) || keys[SDL_SCANCODE_SPACE];
    value.dash = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];
    // Clicking a HUD control does not also fire a projectile.
    SDL_FPoint point{logical.x, logical.y};
    for (const auto& item : buttons) {
        if (SDL_PointInFRect(&point, &item.rect)) {
            value.fire = false;
        }
    }
    return value;
}

void Game::advance(float dt, const Input& value) {
    if (screen != Screen::Playing) {
        return;
    }
    accumulator += std::min(dt, .1f);
    constexpr float step = Rules::FixedStep;
    while (accumulator >= step) {
        run.update(step, value);
        accumulator -= step;
        for (const auto& notification : run.view().events) {
            sound->play(notification.type);
        }
        if (run.view().result != Result::Active) {
            finishRun();
            break;
        }
    }
}

int Game::execute() {
    initialize();
    auto previous = SDL_GetPerformanceCounter();
    const double frequency = static_cast<double>(SDL_GetPerformanceFrequency());
    while (running) {
        auto now = SDL_GetPerformanceCounter();
        float dt = static_cast<float>((now - previous) / frequency);
        previous = now;
        uiTime += std::min(dt, .1f);
        SDL_Event value;
        while (SDL_PollEvent(&value)) {
            event(value);
        }
        advance(dt, input());
        render();
        SDL_RenderPresent(renderer);
        SDL_Delay(1);
    }
    return 0;
}

} // namespace starfall
