#pragma once

#include "VisualResources.hpp"
#include "simulation/Run.hpp"
#include <SDL.h>
#include <deque>
#include <map>
#include <string>
#include <vector>

namespace starfall {

constexpr int ScreenWidth = 1280, ScreenHeight = 800;
using Color = SDL_Color;
inline constexpr Color White{226, 240, 252, 255}, Muted{119, 146, 171, 255};
inline constexpr Color Cyan{87, 237, 229, 255}, Gold{255, 207, 100, 255};
inline constexpr Color Pink{255, 95, 139, 255}, Purple{174, 132, 255, 255};
inline constexpr Color Ink{8, 15, 30, 255}, Panel{13, 25, 43, 240};
Color alpha(Color color, int value);
std::string number(int value, int width = 0);
std::string clockText(int seconds);

class Draw {
  public:
    explicit Draw(SDL_Renderer* renderer);
    ~Draw();
    Draw(const Draw&) = delete;
    Draw& operator=(const Draw&) = delete;
    SDL_Renderer* renderer;
    void fill(float x, float y, float w, float h, Color color);
    void outline(float x, float y, float w, float h, Color color);
    void line(Vec a, Vec b, Color color);
    void circle(Vec center, float radius, Color color, bool filled = false);
    void polygon(const std::vector<Vec>& points, Color color);
    void text(const std::string& value, float x, float y, int scale, Color color);
    void centered(const std::string& value, float y, int scale, Color color);
    int textWidth(const std::string& value, int scale) const;
    void background(float time, Vec camera = {}, bool grid = false);
    void playerSprite(Vec position, float angle, float scale, Color tint = White);
    void projectile(const Bullet& bullet, Vec camera, bool overdrive);
    void enemy(const Enemy& value, Vec position, Vec player, float time);
    void hull(Vec position, int remaining);
    void card(float x, float y, float w, float h, Color accent = Cyan);

  private:
    void atlasSprite(TextureId texture, SDL_Rect source, Vec position, float extent, float rotationRadians,
                     Color tint);
    VisualResources resources;
    using TextKey = std::pair<int, std::string>;
    struct CachedText {
        Texture texture;
        int width;
        int height;
    };
    std::map<TextKey, CachedText> textCache;
    std::deque<TextKey> textOrder;
};

} // namespace starfall
