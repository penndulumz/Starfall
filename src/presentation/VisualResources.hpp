#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <array>
#include <filesystem>
#include <map>
#include <memory>

namespace starfall {

enum class TextureId {
    Ships,
    Projectiles,
    Background1,
    Background2,
    Background3,
    Background4,
    HullFrame,
    HullFill,
    Count
};

struct TextureDeleter {
    void operator()(SDL_Texture* texture) const { SDL_DestroyTexture(texture); }
};
using Texture = std::unique_ptr<SDL_Texture, TextureDeleter>;

// Owns decoded visual resources. All returned handles are borrowed and remain
// valid only while this owner and its SDL renderer are alive.
class VisualResources {
  public:
    explicit VisualResources(SDL_Renderer* renderer);
    SDL_Texture* texture(TextureId id) const;
    TTF_Font* font(int pixelSize) const;

  private:
    struct LibraryLifetime {
        LibraryLifetime();
        ~LibraryLifetime();
        LibraryLifetime(const LibraryLifetime&) = delete;
        LibraryLifetime& operator=(const LibraryLifetime&) = delete;
    } libraries;

    struct FontDeleter {
        void operator()(TTF_Font* font) const { TTF_CloseFont(font); }
    };
    using Font = std::unique_ptr<TTF_Font, FontDeleter>;

    // Declared after libraries: textures/fonts are destroyed before IMG/TTF quit.
    std::array<Texture, static_cast<std::size_t>(TextureId::Count)> textures;
    std::filesystem::path directory;
    mutable std::map<int, Font> fonts;
};

} // namespace starfall
