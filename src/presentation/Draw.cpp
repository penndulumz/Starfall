#include "Draw.hpp"

#include <array>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace starfall {

Color alpha(Color c, int a) {
    c.a = static_cast<Uint8>(std::clamp(a, 0, 255));
    return c;
}
std::string number(int value, int width) {
    std::ostringstream text;
    text << std::setfill('0') << std::setw(width) << value;
    return text.str();
}
std::string clockText(int seconds) {
    return number(seconds / 60, 2) + ":" + number(seconds % 60, 2);
}

Draw::Draw(SDL_Renderer* value) : renderer(value), resources(value) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}
Draw::~Draw() = default;

void Draw::fill(float x, float y, float w, float h, Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_FRect rect{x, y, w, h};
    SDL_RenderFillRectF(renderer, &rect);
}
void Draw::outline(float x, float y, float w, float h, Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_FRect rect{x, y, w, h};
    SDL_RenderDrawRectF(renderer, &rect);
}
void Draw::line(Vec a, Vec b, Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLineF(renderer, a.x, a.y, b.x, b.y);
}
void Draw::circle(Vec p, float radius, Color color, bool filled) {
    if (filled) {
        for (int y = -static_cast<int>(radius); y <= static_cast<int>(radius); ++y) {
            float half = std::sqrt(std::max(0.f, radius * radius - y * y));
            line({p.x - half, p.y + y}, {p.x + half, p.y + y}, color);
        }
    } else {
        int segments = std::max(24, static_cast<int>(radius * .7f));
        Vec last = p + Vec{radius, 0};
        for (int i = 1; i <= segments; ++i) {
            Vec next = p + direction(i * Pi * 2 / segments) * radius;
            line(last, next, color);
            last = next;
        }
    }
}
void Draw::polygon(const std::vector<Vec>& points, Color color) {
    if (points.size() < 3)
        return;
    std::vector<SDL_Vertex> vertices;
    std::vector<int> indices;
    for (Vec point : points)
        vertices.push_back({{point.x, point.y}, color, {0, 0}});
    for (int i = 1; i + 1 < static_cast<int>(points.size()); ++i)
        indices.insert(indices.end(), {0, i, i + 1});
    SDL_RenderGeometry(renderer, nullptr, vertices.data(), static_cast<int>(vertices.size()), indices.data(),
                       static_cast<int>(indices.size()));
}
int Draw::textWidth(const std::string& value, int scale) const {
    if (value.empty() || scale <= 0) {
        return 0;
    }
    int width = 0;
    if (TTF_SizeUTF8(resources.font(8 * scale), value.c_str(), &width, nullptr) < 0) {
        throw std::runtime_error("Cannot measure game text: " + std::string(TTF_GetError()));
    }
    return width;
}

void Draw::text(const std::string& value, float x, float y, int scale, Color color) {
    if (value.empty() || scale <= 0) {
        return;
    }
    const TextKey key{scale, value};
    auto found = textCache.find(key);
    if (found == textCache.end()) {
        using Surface = std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>;
        Surface pixels(TTF_RenderUTF8_Blended(resources.font(8 * scale), value.c_str(), {255, 255, 255, 255}),
                       SDL_FreeSurface);
        if (!pixels) {
            throw std::runtime_error("Cannot render game text: " + std::string(TTF_GetError()));
        }
        Texture texture(SDL_CreateTextureFromSurface(renderer, pixels.get()));
        if (!texture) {
            throw std::runtime_error(SDL_GetError());
        }
        SDL_SetTextureScaleMode(texture.get(), SDL_ScaleModeNearest);
        // Dynamic score/time strings must not grow a permanent texture collection.
        constexpr std::size_t MaximumCachedText = 256;
        if (textCache.size() >= MaximumCachedText) {
            textCache.erase(textOrder.front());
            textOrder.pop_front();
        }
        found = textCache.emplace(key, CachedText{std::move(texture), pixels->w, pixels->h}).first;
        textOrder.push_back(key);
    }
    auto& cached = found->second;
    SDL_SetTextureColorMod(cached.texture.get(), color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(cached.texture.get(), color.a);
    const SDL_FRect destination{x, y, static_cast<float>(cached.width), static_cast<float>(cached.height)};
    SDL_RenderCopyF(renderer, cached.texture.get(), nullptr, &destination);
}

void Draw::centered(const std::string& value, float y, int scale, Color color) {
    text(value, (ScreenWidth - textWidth(value, scale)) / 2.f, y, scale, color);
}

void Draw::playerSprite(Vec position, float angle, float scale, Color tint) {
    atlasSprite(TextureId::Ships, {3 * 128, 0, 128, 128}, position, 64 * scale, angle, tint);
}

void Draw::atlasSprite(TextureId image, SDL_Rect source, Vec position, float extent, float rotationRadians,
                       Color tint) {
    // Atlas sources face up; SDL rotates clockwise around the destination center.
    const SDL_FRect destination{position.x - extent / 2, position.y - extent / 2, extent, extent};
    auto* texture = resources.texture(image);
    SDL_SetTextureColorMod(texture, tint.r, tint.g, tint.b);
    SDL_SetTextureAlphaMod(texture, tint.a);
    SDL_RenderCopyExF(renderer, texture, &source, &destination, rotationRadians * 180.f / Pi, nullptr,
                      SDL_FLIP_NONE);
}

void Draw::projectile(const Bullet& bullet, Vec camera, bool overdrive) {
    const Vec position = bullet.position - camera;
    const Vec facing = unit(bullet.velocity);
    const float angle = std::atan2(facing.y, facing.x) + Pi / 2;
    const Color feedback = bullet.hostile ? Cyan : overdrive ? Gold : Pink;
    circle(position, bullet.hostile ? 7.f : 6.f, alpha(feedback, 35), true);
    line(position - facing * (bullet.hostile ? 12.f : 23.f), position, alpha(feedback, 150));
    // Original unpadded 500-pixel cells: friendly (1,1), hostile (0,0).
    const SDL_Rect source = bullet.hostile ? SDL_Rect{0, 0, 500, 500} : SDL_Rect{500, 500, 500, 500};
    const Color tint = !bullet.hostile && overdrive ? Gold : Color{255, 255, 255, 255};
    atlasSprite(TextureId::Projectiles, source, position, bullet.hostile ? 18.f : 24.f, angle, tint);
}

void Draw::background(float, Vec camera, bool grid) {
    fill(0, 0, ScreenWidth, ScreenHeight, Ink);
    // The source images are 500 pixels, not 500 world units. The reference uses
    // 10000-unit tiles around a 250-unit ship; scale down for Starfall's ship.
    constexpr float TileExtent = 2000.f;
    constexpr std::array<float, 4> Strengths{0.f, .2f, .4f, .7f};
    constexpr std::array<TextureId, 4> Layers{TextureId::Background1, TextureId::Background2,
                                              TextureId::Background3, TextureId::Background4};
    for (std::size_t layer = 0; layer < Layers.size(); ++layer) {
        // Reference convention: shift the world layer by -camera * strength,
        // then project world to screen by subtracting camera. Thus these are
        // world-offset strengths, not direct screen-scroll fractions.
        const Vec scroll = camera * (1.f + Strengths[layer]);
        // floor also handles negative camera shake at the arena's top/left.
        const float firstX = std::floor(scroll.x / TileExtent) * TileExtent - scroll.x;
        const float firstY = std::floor(scroll.y / TileExtent) * TileExtent - scroll.y;
        for (float y = firstY; y < ScreenHeight; y += TileExtent) {
            for (float x = firstX; x < ScreenWidth; x += TileExtent) {
                const SDL_FRect tile{x, y, TileExtent, TileExtent};
                SDL_RenderCopyF(renderer, resources.texture(Layers[layer]), nullptr, &tile);
            }
        }
    }
    // Keep hostile shots and small ships readable over the original bright art.
    fill(0, 0, ScreenWidth, ScreenHeight, {4, 8, 18, 170});
    // A faint arena grid remains original navigation feedback, separate from art.
    if (grid) {
        const float firstX = std::floor(camera.x / 160.f) * 160.f - camera.x;
        const float firstY = std::floor(camera.y / 160.f) * 160.f - camera.y;
        for (float x = firstX; x < ScreenWidth; x += 160) {
            line({x, 0}, {x, ScreenHeight}, alpha(Cyan, 9));
        }
        for (float y = firstY; y < ScreenHeight; y += 160) {
            line({0, y}, {ScreenWidth, y}, alpha(Cyan, 9));
        }
    }
}

void Draw::enemy(const Enemy& value, Vec p, Vec player, float time) {
    const float angle = std::atan2(player.y - p.y, player.x - p.x) + Pi / 2;
    SDL_Rect source{0, 0, 128, 128}; // Scout: cell (0,0).
    float extent = 56;
    Color color = Pink;
    switch (value.type) {
    case EnemyType::Scout:
        break;
    case EnemyType::Gunner:
        source.y = 128; // Cell (0,1).
        extent = 66;
        color = Gold;
        break;
    case EnemyType::Tank:
        source.x = 256; // Cell (2,0).
        extent = 80;
        color = Purple;
        break;
    case EnemyType::Boss:
        source = {384, 128, 128, 128}; // Cell (3,1), not dedicated boss art.
        extent = 184;
        // The larger scale and pulsing command ring are Starfall's treatment
        // of the reference cell. They do not alter the collision radius.
        circle(p, 94, alpha(Pink, 12), true);
        circle(p, 88 + 2 * std::sin(time * 3), alpha(Pink, 100));
        break;
    }
    atlasSprite(TextureId::Ships, source, p, extent, angle, {255, 255, 255, 255});
    if (value.health < value.maxHealth && value.type != EnemyType::Boss) {
        fill(p.x - 19, p.y + value.radius + 10, 38, 3, alpha(color, 40));
        fill(p.x - 19, p.y + value.radius + 10, 38.f * value.health / value.maxHealth, 3, color);
    }
}

void Draw::hull(Vec position, int remaining) {
    const SDL_FRect frame{position.x, position.y, 256, 32};
    SDL_RenderCopyF(renderer, resources.texture(TextureId::HullFrame), nullptr, &frame);
    // Both original images are 128x16. Their shared fill interior is x=4..123,
    // y=4..11. Crop the source AND destination, leaving all frame borders intact.
    const int filledWidth = 120 * std::clamp(remaining, 0, Rules::MaximumHull) / Rules::MaximumHull;
    if (filledWidth > 0) {
        const SDL_Rect source{4, 4, filledWidth, 8};
        const SDL_FRect fillArea{position.x + 8, position.y + 8, filledWidth * 2.f, 16};
        SDL_RenderCopyF(renderer, resources.texture(TextureId::HullFill), &source, &fillArea);
    }
    // The reference is a continuous bar; these equal-capacity separators are
    // Starfall's five-segment presentation, confined to its interior.
    for (int segment = 1; segment < Rules::MaximumHull; ++segment) {
        fill(position.x + 8 + 240.f * segment / Rules::MaximumHull - 1, position.y + 8, 2, 16, Ink);
    }
}

void Draw::card(float x, float y, float w, float h, Color accent) {
    fill(x, y, w, h, Panel);
    outline(x, y, w, h, {36, 55, 75, 255});
    line({x, y}, {x + 28, y}, accent);
    line({x, y}, {x, y + 16}, accent);
    line({x + w - 28, y + h}, {x + w, y + h}, alpha(accent, 90));
}

} // namespace starfall
