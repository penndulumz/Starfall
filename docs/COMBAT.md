# Combat presentation

Player controls, hostile behavior, and scoring are explained in the [player guide](PLAYER.md). This note records how the renderer presents the unchanged simulation.

`Game::gameplay` in `src/app/Screens.cpp` reads the const Run view. It subtracts camera position and shake once, then supplies screen positions to `Draw`. `Draw` borrows textures from its owned `VisualResources`; no rendering operation changes Run state.

## Ship atlas

The original atlas is 640 by 256 pixels, with unpadded 128 by 128 cells. Sources face upward. Hostiles face the player using `atan2(dy, dx) + pi/2`, converted once to SDL's clockwise degrees around the destination center. Projectiles use their velocity with the same convention.

| Role | Cell | Source rectangle | Display square in logical pixels |
| --- | --- | --- | --- |
| Player | (3,0) | (384,0,128,128) | 54.4 in gameplay |
| Scout | (0,0) | (0,0,128,128) | 56 |
| Gunner | (0,1) | (0,128,128,128) | 66 |
| Tank | (2,0) | (256,0,128,128) | 80 |
| Dreadnought | (3,1) | (384,128,128,128) | 184 |

These are display extents including transparent cell margins, not collision diameters. Collision radii and all durability values remain defined in `simulation/Rules.hpp`. Small damaged hostiles retain their original colored durability strip. The Dreadnought has a separate HUD bar and remaining/maximum count.

The Dreadnought role, larger scale, pulsing command ring, attack rules, and supporting wave are Starfall's design. Cell `(3,1)` is reused reference ship artwork, not a dedicated upstream boss. Friendly pink and hostile cyan projectile cells remain distinct; overdrive uses paired friendly shots with gold modulation. The old procedural ship and hostile bodies have been removed. Pickups, particles, map markers, edge arrows, and controls remain original geometric feedback.

## Five-segment hull

Both hull PNGs are 128 by 16. Their shared interior occupies source `x=4..123`, `y=4..11`: 120 by 8 pixels. The HUD displays the full frame at 2x scale. Only the fill interior is cropped, with matching source and destination widths, to `remaining / Rules::MaximumHull` of capacity. Fill cannot cover an empty segment or the frame borders.

The reference image is continuous. Starfall adds four narrow interior separators to make five equal 24-source-pixel capacity slots. The frame is always drawn in full, including at zero hull. Separators stop inside its border. Original image bytes and colors are unchanged.

## Final-wave evidence

The application scenario lets the real wave-ten scheduler produce the Dreadnought and four supports, then repositions them for readable captures. Fifty-five real projectile collisions reduce the boss from 110 to 55 hull. Fifty-five more destroy it while the four supports keep the run active. The last support's destruction opens victory with the existing kill multiplier, wave-clear bonus, victory bonus, and remaining-hull bonus.

![Final encounter at half boss durability](screenshots/dreadnought.png)

Application captures also show every hostile type facing the player, friendly/hostile fire during overdrive, the populated HUD, an offscreen hostile, and all hull values from zero through five. The focused full-campaign scenario still clears all 131 hostiles through real collisions. Controlled captures and an invulnerable campaign pilot verify presentation/progression; human difficulty balance remains unverified. The complete source project includes detailed evidence in `docs/VALIDATION.md`. See also [resource provenance](resources-evidence.md).
