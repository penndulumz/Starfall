#include "VisualResources.hpp"
#include "platform/Paths.hpp"

#include <SDL_image.h>
#include <stdexcept>
#include <string>

namespace starfall {
namespace {

std::runtime_error resourceError(const std::filesystem::path& file, const std::string& detail) {
    return std::runtime_error("Cannot load required resource '" + file.u8string() +
                              "'. Restore the complete game package. " + detail);
}

struct ImageDescription {
    const char* name;
    int width;
    int height;
};

constexpr std::array<ImageDescription, static_cast<std::size_t>(TextureId::Count)> Images{
    {{"spaceShip/stitchedFiles/spaceships.png", 640, 256},
     {"spaceShip/stitchedFiles/projectiles.png", 1500, 1000},
     {"background1.png", 500, 500},
     {"background2.png", 500, 500},
     {"background3.png", 500, 500},
     {"background4.png", 500, 500},
     {"healthBar.png", 128, 16},
     {"health.png", 128, 16}}};

} // namespace

VisualResources::LibraryLifetime::LibraryLifetime() {
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
        const std::string error = IMG_GetError();
        IMG_Quit();
        throw std::runtime_error("Cannot initialize PNG decoding: " + error);
    }
    if (TTF_Init() < 0) {
        const std::string error = TTF_GetError();
        IMG_Quit();
        throw std::runtime_error("Cannot initialize font rendering: " + error);
    }
}

VisualResources::LibraryLifetime::~LibraryLifetime() {
    TTF_Quit();
    IMG_Quit();
}

VisualResources::VisualResources(SDL_Renderer* renderer) : directory(resourceDirectory()) {
    for (std::size_t index = 0; index < Images.size(); ++index) {
        const auto& image = Images[index];
        const auto file = directory / image.name;
        Texture loaded(IMG_LoadTexture(renderer, file.u8string().c_str()));
        if (!loaded) {
            throw resourceError(file, IMG_GetError());
        }
        int width = 0;
        int height = 0;
        if (SDL_QueryTexture(loaded.get(), nullptr, nullptr, &width, &height) < 0 || width != image.width ||
            height != image.height) {
            throw resourceError(file, "The image dimensions do not match the packaged reference atlas.");
        }
        SDL_SetTextureScaleMode(loaded.get(), SDL_ScaleModeNearest);
        SDL_SetTextureBlendMode(loaded.get(), SDL_BLENDMODE_BLEND);
        textures[index] = std::move(loaded);
    }
    font(8); // Validate the required font during startup, before any screen draws.
}

SDL_Texture* VisualResources::texture(TextureId id) const {
    return textures.at(static_cast<std::size_t>(id)).get();
}

TTF_Font* VisualResources::font(int pixelSize) const {
    auto found = fonts.find(pixelSize);
    if (found != fonts.end()) {
        return found->second.get();
    }
    const auto file = directory / "CommodorePixeled.ttf";
    Font loaded(TTF_OpenFont(file.u8string().c_str(), pixelSize));
    if (!loaded) {
        throw resourceError(file, TTF_GetError());
    }
    TTF_SetFontHinting(loaded.get(), TTF_HINTING_MONO);
    return fonts.emplace(pixelSize, std::move(loaded)).first->second.get();
}

} // namespace starfall
