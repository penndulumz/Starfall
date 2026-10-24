#include "Game.hpp"

namespace starfall {

void Game::button(Action value) {
    auto found = std::find_if(buttons.begin(), buttons.end(),
                              [value](const Button& item) { return item.action == value; });
    if (found == buttons.end()) {
        return;
    }
    const Button& item = *found;
    const auto [x, y, w, h] = item.rect;
    const bool primary = item.primary;
    const bool enabled = item.enabled;
    SDL_FPoint point{mouse.x, mouse.y};
    const bool hovered = mouseNavigation && SDL_PointInFRect(&point, &item.rect);
    const int index = static_cast<int>(found - buttons.begin());
    const bool selected =
        enabled && (hovered || (!mouseNavigation && screen != Screen::Playing && index == focused));
    Color edge = primary || selected ? Cyan : Color{53, 76, 96, 255};
    Color fill = primary ? (selected ? White : Cyan) : selected ? Color{23, 52, 66, 250} : Panel;
    if (!enabled) {
        edge = {35, 51, 66, 255};
        fill = {13, 23, 37, 220};
    }
    draw->fill(x, y, w, h, fill);
    draw->outline(x, y, w, h, edge);
    draw->text(item.label, x + 24, y + (h - 14) / 2, 2,
               !enabled   ? Muted
               : primary  ? Ink
               : selected ? Cyan
                          : White);
    if (w >= 200 && enabled) {
        const Color arrow = primary ? Ink : selected ? Cyan : Muted;
        draw->line({x + w - 32, y + h / 2 - 5}, {x + w - 26, y + h / 2}, arrow);
        draw->line({x + w - 26, y + h / 2}, {x + w - 32, y + h / 2 + 5}, arrow);
    }
    if (selected) {
        draw->fill(x - 6, y + 10, 2, h - 20, Cyan);
    }
}

void Game::footer(const std::string& left) {
    draw->fill(0, 748, ScreenWidth, 52, {6, 13, 24, 248});
    draw->line({32, 748}, {1248, 748}, {35, 52, 68, 255});
    std::string message = left;
    switch (sound->warningKind()) {
    case AudioWarningKind::None:
        break;
    case AudioWarningKind::Resource:
        message = "AUDIO RESOURCE ERROR / RESTORE COMPLETE GAME PACKAGE";
        break;
    case AudioWarningKind::DeviceUnavailable:
        message = "AUDIO DEVICE UNAVAILABLE / CHECK YOUR SOUND DEVICE / PLAY CAN CONTINUE";
        break;
    case AudioWarningKind::Initialization:
        message = "AUDIO INITIALIZATION FAILED / RESTART GAME / PLAY CAN CONTINUE";
        break;
    }
    draw->text(message, 40, 769, 1, sound->warningKind() == AudioWarningKind::None ? Muted : Gold);
    button(Action::Mute);
}

void Game::render() {
    refreshLayout();
    switch (screen) {
    case Screen::Welcome:
        welcome();
        break;
    case Screen::Playing:
        gameplay();
        break;
    case Screen::Paused:
        gameplay();
        pause();
        break;
    case Screen::Controls:
        controls();
        break;
    case Screen::Leaderboard:
        leaderboard();
        break;
    case Screen::End:
        end();
        break;
    }
}

void Game::welcome() {
    draw->background(uiTime, {uiTime * 4, 0}, true);
    draw->fill(0, 0, 620, 748, {5, 12, 25, 90});
    draw->text("SF / FLIGHT SYSTEMS", 72, 38, 2, White);
    draw->text("LOCAL OPERATIONS", 976, 38, 1, Muted);
    draw->circle({1185, 42}, 3, Cyan, true);
    draw->line({72, 80}, {1208, 80}, {44, 63, 83, 255});
    draw->text("01 / THE OUTER RIM", 72, 166, 2, Cyan);
    draw->text("STARFALL", 68, 218, 11, alpha(Cyan, 30));
    draw->text("STARFALL", 72, 214, 11, White);
    draw->fill(72, 312, 62, 3, Cyan);
    draw->text("THE LAST LIGHT IN THE VOID.", 72, 342, 2, White);
    draw->text("ONE PILOT. TEN WAVES. NO BACKUP.", 72, 372, 1, Muted);
    button(Action::Start);
    button(Action::Board);
    button(Action::Help);
    button(Action::Quit);
    draw->text("PERSONAL BEST", 270, 644, 1, Muted);
    draw->text(scores->entries().empty() ? "------" : number(scores->entries().front().points, 6), 270, 666,
               2, Gold);

    Vec center{936, 350};
    draw->circle(center, 198, alpha(Cyan, 26));
    draw->circle(center, 158, alpha(Cyan, 18));
    draw->circle(center, 114, alpha(Cyan, 15));
    for (int i = 0; i < 48; ++i) {
        float angle = i * Pi / 24 + uiTime * .025f;
        draw->line(center + direction(angle) * 190, center + direction(angle) * (i % 4 == 0 ? 204.f : 195.f),
                   alpha(Cyan, i % 4 == 0 ? 100 : 35));
    }
    draw->line({704, 350}, {758, 350}, alpha(Cyan, 65));
    draw->line({1118, 350}, {1172, 350}, alpha(Cyan, 65));
    draw->line({936, 122}, {936, 164}, alpha(Cyan, 65));
    draw->line({936, 536}, {936, 578}, alpha(Cyan, 65));
    draw->playerSprite(center + Vec{0, std::sin(uiTime * 1.6f) * 7}, -.36f, 4.2f);
    draw->text("SF-01 / INTERCEPTOR", 808, 110, 2, Muted);
    draw->text("REACTOR ONLINE", 978, 554, 1, Cyan);
    draw->line({1010, 489}, {1070, 536}, alpha(Cyan, 80));
    draw->card(690, 612, 518, 108);
    draw->text("MISSION BRIEF", 714, 632, 1, Cyan);
    draw->text("HOLD THE LINE.", 714, 657, 3, White);
    draw->text("BREAK THE SWARM. DEFEAT THE DREADNOUGHT.", 714, 696, 1, Muted);
    footer("ARROWS / TAB: SELECT     ENTER: CONFIRM     F11: FULLSCREEN     M: SOUND");
}

void Game::gameplay() {
    Vec view = camera();
    Vec jitter{std::sin(uiTime * 123) * run.view().shake, std::cos(uiTime * 107) * run.view().shake};
    Vec offset = view + jitter;
    draw->background(uiTime, offset, true);
    draw->outline(-offset.x, -offset.y, ArenaWidth, ArenaHeight, alpha(Cyan, 100));
    for (int i = 1; i < 13; ++i) {
        float x = i * 200.f - offset.x;
        draw->line({x, 15 - offset.y}, {x, 38 - offset.y}, alpha(Cyan, 100));
        draw->line({x, ArenaHeight - 38 - offset.y}, {x, ArenaHeight - 15 - offset.y}, alpha(Cyan, 100));
    }
    for (const auto& pickup : run.view().pickups) {
        Vec p = pickup.position - offset;
        Color color = pickup.type == PickupType::Repair ? Cyan : Gold;
        if (pickup.life < 3 && static_cast<int>(uiTime * 8) % 2 == 0)
            continue;
        draw->circle(p, 18 + std::sin(uiTime * 5) * 2, alpha(color, 45));
        draw->circle(p, 12, alpha(color, 25), true);
        if (pickup.type == PickupType::Repair) {
            draw->fill(p.x - 7, p.y - 2, 14, 4, color);
            draw->fill(p.x - 2, p.y - 7, 4, 14, color);
        } else
            draw->polygon({p + Vec{2, -9}, p + Vec{-6, 2}, p + Vec{0, 2}, p + Vec{-2, 9}, p + Vec{7, -2},
                           p + Vec{1, -2}},
                          color);
    }
    for (const auto& bullet : run.view().bullets) {
        draw->projectile(bullet, offset, run.view().overdrive > 0);
    }
    for (const auto& enemy : run.view().enemies) {
        Vec p = enemy.position - offset;
        if (p.x > -120 && p.x < ScreenWidth + 120 && p.y > -120 && p.y < ScreenHeight + 120)
            draw->enemy(enemy, p, run.view().player - offset, uiTime);
        if (p.x < 30 || p.x > ScreenWidth - 30 || p.y < 105 || p.y > 730) {
            Vec indicator{std::clamp(p.x, 26.f, 1254.f), std::clamp(p.y, 105.f, 724.f)};
            Vec aim = unit(p - Vec{ScreenWidth / 2.f, ScreenHeight / 2.f});
            Vec side{-aim.y, aim.x};
            draw->polygon(
                {indicator + aim * 7, indicator - aim * 4 + side * 4, indicator - aim * 4 - side * 4},
                alpha(Pink, 160));
        }
    }
    for (const auto& particle : run.view().particles) {
        Vec p = particle.position - offset;
        Color color = particle.tone == 0 ? Cyan : particle.tone == 1 ? Gold : Pink;
        int opacity = static_cast<int>(255 * particle.life / particle.duration);
        draw->fill(p.x, p.y, particle.size, particle.size, alpha(color, opacity));
    }
    Vec player = run.view().player - offset;
    if (run.view().invulnerable > 0)
        draw->circle(player, 36, alpha(Cyan, 100));
    if (run.view().invulnerable <= 0 || static_cast<int>(uiTime * 18) % 2 == 0)
        draw->playerSprite(player, std::atan2(run.view().facing.y, run.view().facing.x) + Pi / 2, .85f,
                           run.view().overdrive > 0 ? Gold : Color{255, 255, 255, 255});
    if (screen == Screen::Playing) {
        Vec aim = mouse;
        draw->circle(aim, 11, alpha(Cyan, 150));
        draw->line(aim + Vec{-17, 0}, aim + Vec{-8, 0}, Cyan);
        draw->line(aim + Vec{8, 0}, aim + Vec{17, 0}, Cyan);
        draw->line(aim + Vec{0, -17}, aim + Vec{0, -8}, Cyan);
        draw->line(aim + Vec{0, 8}, aim + Vec{0, 17}, Cyan);
    }
    draw->card(24, 20, 276, 62);
    draw->text("HULL INTEGRITY", 44, 32, 1, Muted);
    draw->hull({34, 46}, run.view().health);
    draw->card(320, 20, 300, 62);
    draw->text("SCORE", 340, 32, 1, Muted);
    draw->text(number(run.view().score, 6), 340, 49, 3, White);
    if (run.multiplier() > 1)
        draw->text("X" + number(run.multiplier()), 552, 53, 2, Gold);
    draw->card(818, 20, 260, 62);
    draw->text("WAVE " + number(run.view().wave, 2) + " / " + number(Rules::FinalWave), 838, 32, 2, White);
    draw->text(number(run.remaining(), 2) + " HOSTILES / " + clockText(static_cast<int>(run.view().elapsed)),
               838, 62, 1, Muted);
    button(Action::Pause);
    if (run.view().announcement > 0) {
        draw->card(402, 148, 476, 96, run.view().wave == Rules::FinalWave ? Pink : Cyan);
        draw->centered(run.view().wave == Rules::FinalWave ? "FINAL CONTACT"
                                                           : "WAVE " + number(run.view().wave, 2),
                       168, 4, run.view().wave == Rules::FinalWave ? Pink : White);
        draw->centered(run.view().waveDelay > 0
                           ? "INCOMING / " + number(static_cast<int>(std::ceil(run.view().waveDelay)), 2)
                           : "CLEAR ALL HOSTILES",
                       216, 1, Muted);
    }
    for (const auto& enemy : run.view().enemies)
        if (enemy.type == EnemyType::Boss) {
            draw->centered("DREADNOUGHT / " + number(enemy.health, 3) + " / " + number(enemy.maxHealth, 3),
                           103, 2, Pink);
            draw->fill(398, 128, 484, 7, alpha(Pink, 30));
            draw->fill(398, 128, 484.f * enemy.health / enemy.maxHealth, 7, Pink);
        }
    draw->card(24, 676, 218, 58);
    draw->text(run.view().dashCooldown <= 0 ? "SHIFT / DASH READY" : "DASH RECHARGING", 40, 690, 1,
               run.view().dashCooldown <= 0 ? Cyan : Muted);
    draw->fill(40, 714, 186, 4, {35, 53, 70, 255});
    draw->fill(40, 714, 186 * (1 - run.view().dashCooldown / Rules::DashRecharge), 4, Cyan);
    if (run.view().overdrive > 0)
        draw->text("OVERDRIVE / " + number(static_cast<int>(std::ceil(run.view().overdrive)), 2), 266, 704, 2,
                   Gold);
    if (run.view().chain > 1)
        draw->text(number(run.view().chain) + " KILL CHAIN", 560, 704, 2, Gold);
    draw->card(1066, 622, 190, 112);
    draw->text("SECTOR MAP", 1078, 634, 1, Muted);
    auto mapPoint = [](Vec position) {
        return Vec{1078 + position.x / ArenaWidth * 166, 654 + position.y / ArenaHeight * 67};
    };
    for (const auto& enemy : run.view().enemies) {
        Vec p = mapPoint(enemy.position);
        draw->fill(p.x - 1, p.y - 1, 3, 3, Pink);
    }
    Vec marker = mapPoint(run.view().player);
    draw->circle(marker, 3, Cyan, true);
    footer("WASD: MOVE   MOUSE: AIM   HOLD CLICK / SPACE: FIRE   SHIFT: DASH   ESC: PAUSE");
}

void Game::pause() {
    draw->fill(0, 0, ScreenWidth, ScreenHeight, {4, 10, 22, 221});
    draw->card(380, 192, 520, 408);
    draw->text("PAUSED", 420, 230, 5, White);
    draw->text("CATCH YOUR BREATH, PILOT.", 420, 292, 1, Muted);
    button(Action::Resume);
    button(Action::Help);
    button(Action::Menu);
    draw->text("ABANDONED RUNS DO NOT ENTER THE LEADERBOARD.", 420, 562, 1, Muted);
    footer("ESC / P: RESUME     ARROWS / TAB: SELECT     ENTER: CONFIRM");
}

void Game::controls() {
    draw->background(uiTime, {20, 0}, true);
    draw->text("FLIGHT MANUAL", 72, 102, 5, White);
    draw->text("EVERYTHING YOU NEED FOR YOUR FIRST MISSION.", 72, 162, 2, Muted);
    draw->card(72, 208, 1136, 468);
    draw->text("YOUR CONTROLS", 104, 238, 2, Cyan);
    auto row = [&](const std::string& key, const std::string& description, float y) {
        draw->fill(104, y, 206, 44, {25, 42, 60, 255});
        draw->outline(104, y, 206, 44, {48, 72, 91, 255});
        draw->text(key, 120, y + 15, 2, White);
        draw->text(description, 338, y + 15, 2, Muted);
    };
    row("WASD / ARROWS", "MOVE YOUR SHIP", 284);
    row("MOUSE", "AIM ANYWHERE", 344);
    row("HOLD CLICK", "FIRE / OR SPACE", 404);
    row("SHIFT", "DASH + EVADE", 464);
    row("ESC / P", "PAUSE MISSION", 524);
    row("F11 / M", "FULLSCREEN / SOUND", 584);
    draw->line({722, 240}, {722, 636}, {39, 57, 77, 255});
    draw->text("THE MISSION", 762, 238, 2, Gold);
    draw->text("SURVIVE 10 WAVES.", 762, 288, 2, White);
    draw->text("DEFEAT THE DREADNOUGHT.", 762, 322, 2, White);
    draw->text("CYAN +  REPAIRS YOUR HULL", 762, 380, 2, Cyan);
    draw->text("GOLD    10S OVERDRIVE", 762, 414, 2, Gold);
    draw->text("DASH THROUGH DANGER.", 762, 472, 2, White);
    draw->text("CHAIN KILLS FOR UP TO 5X.", 762, 506, 2, White);
    draw->text("SAVE YOUR SCORE AFTER A RUN.", 762, 564, 2, Muted);
    draw->text("TIP: KEEP MOVING. WATCH THE SECTOR MAP.", 762, 610, 1, Muted);
    button(Action::Back);
    footer("ARROWS / TAB: SELECT     ENTER: CONFIRM     ESC: BACK");
}

void Game::leaderboard() {
    draw->background(uiTime, {40, 10}, true);
    draw->text("HALL OF PILOTS", 72, 96, 5, White);
    draw->text("THE BEST TEN FLIGHTS ON THIS COMPUTER.", 72, 154, 2, Muted);
    draw->card(72, 198, 1136, 478, Gold);
    draw->text("RANK", 100, 220, 1, Muted);
    draw->text("PILOT", 206, 220, 1, Muted);
    draw->text("SCORE", 560, 220, 1, Muted);
    draw->text("WAVE", 790, 220, 1, Muted);
    draw->text("TIME", 930, 220, 1, Muted);
    draw->text("RESULT", 1060, 220, 1, Muted);
    draw->line({100, 240}, {1180, 240}, {41, 58, 76, 255});
    const auto& values = scores->entries();
    if (values.empty()) {
        draw->centered("NO FLIGHTS RECORDED", 384, 3, White);
        draw->centered("FINISH A RUN AND SAVE YOUR FIRST SCORE.", 432, 2, Muted);
    }
    for (std::size_t i = 0; i < values.size(); ++i) {
        const auto& entry = values[i];
        float y = 257 + i * 39.f;
        bool yours = returnScreen == Screen::End && saved && static_cast<int>(i) + 1 == savedRank;
        draw->fill(92, y - 7, 1096, 34,
                   yours        ? Color{26, 58, 63, 240}
                   : i % 2 == 0 ? Color{20, 33, 51, 210}
                                : Color{13, 25, 43, 100});
        Color medal = i == 0 ? Gold : i < 3 ? Cyan : Muted;
        draw->text(number(static_cast<int>(i) + 1, 2), 100, y, 2, medal);
        draw->text(entry.name, 206, y, 2, White);
        draw->text(number(entry.points, 6), 560, y, 2, i == 0 ? Gold : White);
        draw->text(number(entry.wave, 2), 790, y, 2, Muted);
        draw->text(clockText(entry.seconds), 930, y, 2, Muted);
        draw->text(entry.victory ? "VICTORY" : "LOST", 1060, y, 2, entry.victory ? Cyan : Pink);
    }
    draw->text("TOP 10 / LOCAL SCORES / EARLIER ENTRY WINS A TIE", 100, 652, 1, Muted);
    button(Action::Back);
    if (!scores->warning().empty())
        draw->text(scores->warning(), 304, 712, 1, Gold);
    footer("ARROWS / TAB: SELECT     ENTER: CONFIRM     ESC: BACK");
}

void Game::end() {
    bool won = run.view().result == Result::Victory;
    Color accent = won ? Gold : Pink;
    draw->background(uiTime, {uiTime * 2, 20}, true);
    draw->text(won ? "RUN COMPLETE / VICTORY" : "RUN COMPLETE / DEFEAT", 88, 92, 2, accent);
    draw->text(won ? "SECTOR CLEAR" : "SIGNAL LOST", 88, 136, 6, White);
    draw->text(won ? "THE OUTER RIM IS SAFE. FOR NOW." : "THE VOID REMEMBERS. FLY AGAIN.", 88, 200, 1, Muted);
    draw->card(88, 240, 524, 354, accent);
    draw->text("FINAL SCORE", 116, 262, 1, Muted);
    draw->text(number(run.view().score, 6), 116, 288, 6, won ? Gold : White);
    draw->text("WAVE " + number(run.view().wave, 2), 116, 354, 2, Cyan);
    draw->text(number(run.view().kills) + " KILLS", 290, 354, 2, White);
    draw->text(clockText(static_cast<int>(run.view().elapsed)), 464, 354, 2, Muted);
    draw->text("PILOT NAME / 12 CHARACTERS MAX", 116, 384, 1, Muted);
    draw->fill(PilotNameField.x, PilotNameField.y, PilotNameField.w, PilotNameField.h, {8, 18, 33, 255});
    draw->outline(PilotNameField.x, PilotNameField.y, PilotNameField.w, PilotNameField.h,
                  saved ? alpha(Cyan, 70) : Cyan);
    if (nameSelected && !saved) {
        // Measure the loaded font rather than assuming the old bitmap glyph width.
        const float selectionWidth = std::min(PilotNameField.w - 28,
                                               static_cast<float>(draw->textWidth(pilotName, 2) + 4));
        draw->fill(PilotNameField.x + 14, PilotNameField.y + 13, selectionWidth, 25,
                   {35, 70, 83, 255});
    }
    draw->text(pilotName + (!saved && static_cast<int>(uiTime * 2) % 2 == 0 ? "_" : ""), 132, 422, 2,
               saved ? Muted : White);
    button(Action::Save);
    draw->text(saved ? (savedRank > 0 ? "YOUR FLIGHT HAS BEEN RECORDED." : "SCORE DID NOT REACH THE TOP 10.")
                     : "TYPE YOUR NAME, THEN PRESS ENTER TO SAVE.",
               116, 480, 1, Muted);
    if (!saveMessage.empty())
        draw->text(saveMessage, 116, 512, saveMessage.size() > 36 ? 1 : 2, saved ? Cyan : Pink);
    draw->text(won ? "VICTORY BONUS INCLUDED IN YOUR SCORE." : "TIP: DASH MAKES YOU BRIEFLY INVULNERABLE.",
               116, 560, 1, Muted);
    Vec center{934, 392};
    draw->circle(center, 179, alpha(accent, 40));
    draw->circle(center, 155, alpha(accent, 18));
    if (won) {
        draw->playerSprite(center, -.28f, 3.5f);
        draw->text("MISSION ACCOMPLISHED", 812, 608, 2, Gold);
    } else {
        draw->playerSprite(center, .43f, 2.6f, alpha(Pink, 200));
        for (int i = 0; i < 24; ++i) {
            float angle = i * 2.4f;
            Vec p = center + direction(angle + uiTime * .03f) * (84 + i * 4.f);
            draw->fill(p.x, p.y, i % 3 + 2.f, i % 3 + 2.f, alpha(accent, 60 + i * 5));
        }
        draw->text("TRANSMISSION ENDED", 826, 608, 2, Pink);
    }
    button(Action::Start);
    button(Action::Board);
    button(Action::Menu);
    draw->text("SAVE YOUR SCORE BEFORE LEAVING THIS SCREEN.", 88, 716, 1, Muted);
    footer("TYPE: PILOT NAME     TAB / ARROWS: SELECT     ENTER: CONFIRM");
}

} // namespace starfall
