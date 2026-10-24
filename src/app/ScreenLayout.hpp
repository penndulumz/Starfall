#pragma once

#include <SDL.h>
#include <string>
#include <vector>

namespace starfall {

enum class Screen { Welcome, Playing, Paused, Controls, Leaderboard, End };
enum class Action { Start, Board, Help, Quit, Back, Pause, Resume, Menu, Save, Mute };

struct Button {
    Action action;
    SDL_FRect rect;
    std::string label;
    bool primary = false;
    bool enabled = true;
};

// Logical pixels. Drawing and hit testing use this same field rectangle.
inline constexpr SDL_FRect PilotNameField{116, 402, 296, 54};

// A layout exists before input, independent of whether a frame was drawn.
std::vector<Button> screenLayout(Screen screen, bool saved, int savedRank, bool audioAvailable,
                                 bool audioEnabled);

} // namespace starfall
