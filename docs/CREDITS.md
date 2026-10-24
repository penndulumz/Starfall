# Starfall credits and resources

Starfall's C++ game code, ten-wave campaign, hostile roles, Dreadnought encounter design, controls, menus, scoring, leaderboard, geometric indicators, and non-shot sound generation are original project work under the supplied MIT code license.

The unchanged ship/projectile atlases, space background layers, hull frame/fill, pixel font, and shooting clip originate from the game-course reference pinned in the [resource manifest](../third_party/RESOURCE-MANIFEST.md). The font identifies Devin Cook as its creator, family Commodore 64 Pixeled, version 1.1. Other resource creators have not been independently verified. The user's permission statement and the remaining terms/source documentation are recorded in the [reference notice](../resources/reference/NOTICE.md); these assets are not described as MIT, CC0, or SDL-licensed.

The current presentation uses the reference font, player/hostile/projectile atlases, hull frame/fill, and four tiled background layers. The hostile role assignments, sprite scales, Dreadnought command ring, five-segment division of the continuous hull bar, tile scale, background dimming, gold overdrive treatment, and geometric pickup/effect feedback are Starfall presentation choices. The Dreadnought reuses ship cell `(3,1)` at a larger scale; the reference does not supply a dedicated Starfall boss resource. See the [combat mappings](COMBAT.md). The reference shooting FLAC plays through SDL2_mixer, with original non-shot event sounds generated into owned mixer chunks.

The portable package includes SDL2 2.30.10 and source-built SDL2_image 2.8.8, SDL2_ttf 2.24.0, and SDL2_mixer 2.8.1. FreeType 2.13.3 is linked statically into the font library. PNG uses bundled stb_image; FLAC support uses bundled dr_flac. Exact source revisions, verification, and local probe hashes are recorded in the manifest. Recompute final package hashes when making a release.

Dependency license texts are included with the game: the SDL2 notice and the `licenses` directory contain the companion zlib notices, stb_image MIT notice, dr_libs MIT No Attribution notice, FreeType FTL notice, and FreeType's bundled zlib notice. The two small decoder/license headers are included unchanged so their complete notices are retained.

The `licenses` directory also retains the unchanged BDF/PCF README and `fthash` source/header notices named by FreeType's `LICENSE.TXT`, plus the `ft-hb` source/header Old MIT notices for completeness. Source files in this notice directory preserve attribution text; they are not extra runtime dependencies.

Portions of this software are copyright (c) 2024 The FreeType Project (www.freetype.org). All rights reserved.

Detailed research and development-probe instructions remain in the source project's [resource evidence](resources-evidence.md); its planning-context links refer to the complete source package.
