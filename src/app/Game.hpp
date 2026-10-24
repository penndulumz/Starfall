#pragma once
#include "presentation/Draw.hpp"
#include "persistence/Scores.hpp"
#include "ScreenLayout.hpp"
#include "audio/Sound.hpp"
#include <filesystem>
#include <memory>

namespace starfall {
namespace testing {
class ApplicationScenarios;
}

class Game {
  public:
    explicit Game(std::filesystem::path dataDirectory = {}, bool hidden = false);
    ~Game();
    int execute();

  private:
    friend class testing::ApplicationScenarios;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    std::unique_ptr<Draw> draw;
    std::unique_ptr<Sound> sound;
    std::unique_ptr<Scores> scores;
    std::filesystem::path dataDirectory;
    bool hidden = false;
    Run run;
    Screen screen = Screen::Welcome, returnScreen = Screen::Welcome;
    std::vector<Button> buttons;
    int focused = 0, savedRank = 0;
    bool running = true, mouseNavigation = false, saved = false, nameSelected = true;
    bool fullscreen = false;
    float uiTime = 0, accumulator = 0;
    Vec mouse{640, 400};
    std::string pilotName = "PILOT", lastName = "PILOT", saveMessage;

    void initialize();
    void refreshLayout();
    void event(const SDL_Event& value);
    void action(Action value);
    void changeScreen(Screen value);
    void startRun();
    void finishRun();
    void advance(float dt, const Input& input);
    void saveScore();
    Vec camera() const;
    Input input() const;
    void render();
    void welcome();
    void gameplay();
    void pause();
    void controls();
    void leaderboard();
    void end();
    void footer(const std::string& left);
    void button(Action action);
};
} // namespace starfall
