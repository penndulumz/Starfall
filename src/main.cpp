#define SDL_MAIN_HANDLED
#include "app/Game.hpp"
#include <iostream>
#include <stdexcept>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

static int launchGame(int argc, char** argv) {
    try {
        std::filesystem::path data;
        for (int i = 1; i < argc; ++i) {
            std::string argument = argv[i];
            if (argument == "--data-dir" && i + 1 < argc) {
                data = std::filesystem::u8path(argv[++i]);
            } else if (argument == "--help") {
                std::cout << "Starfall: --data-dir <folder>\n";
                return 0;
            } else
                throw std::runtime_error("Unknown or incomplete option: " + argument);
        }
        starfall::Game game(data);
        return game.execute();
    } catch (const std::exception& error) {
        std::cerr << "Starfall: " << error.what() << '\n';
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Starfall could not start", error.what(), nullptr);
        return 1;
    }
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return launchGame(__argc, __argv);
}
#endif

int main(int argc, char** argv) {
    return launchGame(argc, argv);
}
