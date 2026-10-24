#include "Paths.hpp"

#include <SDL.h>
#include <memory>
#include <stdexcept>

namespace starfall {

std::filesystem::path resourceDirectory() {
    std::unique_ptr<char, decltype(&SDL_free)> base(SDL_GetBasePath(), SDL_free);
    if (!base) {
        throw std::runtime_error("Cannot find the game executable directory: " + std::string(SDL_GetError()));
    }
    return std::filesystem::u8path(base.get()) / "resources" / "reference";
}

} // namespace starfall
