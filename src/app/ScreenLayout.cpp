#include "ScreenLayout.hpp"

namespace starfall {

std::vector<Button> screenLayout(Screen screen, bool saved, int savedRank, bool audioAvailable,
                                 bool audioEnabled) {
    std::vector<Button> buttons;
    switch (screen) {
    case Screen::Welcome:
        buttons = {{Action::Start, {72, 408, 324, 60}, "START MISSION", true},
                   {Action::Board, {72, 484, 324, 52}, "LEADERBOARD"},
                   {Action::Help, {72, 552, 324, 52}, "HOW TO PLAY"},
                   {Action::Quit, {72, 640, 154, 44}, "QUIT"}};
        break;
    case Screen::Playing:
        buttons = {{Action::Pause, {1120, 20, 136, 62}, "PAUSE"}};
        break;
    case Screen::Paused:
        buttons = {{Action::Resume, {420, 334, 440, 58}, "RESUME MISSION", true},
                   {Action::Help, {420, 410, 440, 50}, "HOW TO PLAY"},
                   {Action::Menu, {420, 478, 440, 50}, "ABANDON TO MENU"}};
        break;
    case Screen::Controls:
    case Screen::Leaderboard:
        buttons = {{Action::Back, {72, 694, 200, 44}, "BACK", true}};
        break;
    case Screen::End:
        buttons = {{Action::Save,
                    {430, 402, 154, 54},
                    saved ? (savedRank > 0 ? "SAVED" : "DONE") : "SAVE",
                    !saved,
                    !saved},
                   {Action::Start, {88, 636, 254, 54}, "PLAY AGAIN", true},
                   {Action::Board, {362, 636, 254, 54}, "LEADERBOARD"},
                   {Action::Menu, {636, 636, 220, 54}, "MAIN MENU"}};
        break;
    }
    buttons.push_back({Action::Mute,
                       {1060, 758, 180, 30},
                       !audioAvailable ? "NO AUDIO"
                       : audioEnabled  ? "SOUND ON"
                                       : "SOUND OFF",
                       false,
                       audioAvailable});
    return buttons;
}

} // namespace starfall
