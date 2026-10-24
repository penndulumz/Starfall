# Reference resource evidence

Research date: 2026-10-23; source-build and format-probe validation completed 2026-10-24.

Implementation context: [ticket 02](../.scratch/starfall-overhaul/issues/02-reference-resource-evidence.md) and the [overhaul specification](../.scratch/starfall-overhaul/spec.md). This note records new observations of the actual bytes, separately from the earlier planning audit.

## Permission and provenance

The user stated on 2026-10-23: "I have permission covering the reference assets; I’ll provide the terms or source." The requested original resources are therefore staged for the authorized local implementation. The exact permission text/source is still forthcoming. This is user-attested permission, not an independently verified asset license; it must not be represented as MIT, CC0, or the SDL dependency license.

The pinned reference is [meemknight/game-in-cpp-full-course at a9b224196ad57fcfd6da2f70fd989ca4fa6ab5e1](https://github.com/meemknight/game-in-cpp-full-course/tree/a9b224196ad57fcfd6da2f70fd989ca4fa6ab5e1). The complete [Git tree](https://api.github.com/repos/meemknight/game-in-cpp-full-course/git/trees/a9b224196ad57fcfd6da2f70fd989ca4fa6ab5e1?recursive=1) contains no first-party/resource license or credits file. Its README links course videos without describing resource terms. The [repository metadata](https://api.github.com/repos/meemknight/game-in-cpp-full-course) currently returns `license: null`. These observations identify a documentation gap; they do not contradict the user's permission statement.

The resource ZIP was also fetched and inspected. It contains the same loose resource candidates and authoring files, no additional terms, and passes archive integrity checking. All eleven inspected candidate files match their loose-file bytes. No author was contacted. Unknown artwork/audio creators remain unknown.

The font itself identifies "By Devin Cook (DevinCook.com)", family "Commodore 64 Pixeled", version 1.1, 204 glyphs, and raw OS/2 `fsType` value 2. It contains no license text/name-table license URL. Font mirror sites' freeware descriptions were not accepted as primary permission evidence for this exact version. Preserve the user's forthcoming terms with the font instead of inventing a license.

## Actual decoded resources

Ten unchanged original resources are staged under `resources/reference`, retaining their relative upstream paths. The [resource manifest](../third_party/RESOURCE-MANIFEST.md) records hashes. All eight required PNGs were fully decoded using Pillow and visually inspected; dimensions below come from those decoded bytes, not inferred atlas counts.

| Resource | Verified properties |
| --- | --- |
| Ship atlas | 640 × 256, RGBA; five columns and two rows; 128 × 128 cells; ships face upward in source orientation. |
| Projectile atlas | 1500 × 1000, RGBA; three columns and two rows; 500 × 500 cells; bright tips above darker trails. |
| Four backgrounds | Each 500 × 500 RGBA; nebula base, transparent nebula layer, stars, and planets respectively. |
| Hull frame and fill | Each 128 × 16 RGBA; frame and fill share coordinates and should be clipped together. |
| Shooting FLAC | STREAMINFO: 24,000 Hz, stereo, 24-bit, 8,514 samples, 0.35475 seconds. Vorbis comments identify encoder `Lavf59.27.100`, not an asset creator. Both compiler builds decoded it through SDL_mixer into 68,112 bytes of device-format PCM and mixed playback with the dummy driver. |
| Candidate font | TrueType, family/version/glyph metadata as above; both compiler builds opened it through SDL_ttf and rasterized `PILOT 0123456789` into a 208 × 16 surface at 16-point size. |

The optional UI image is 100 × 100 RGBA and depicts a single button-like panel, rather than a documented atlas of complete menus. It was inspected but is not selected or copied into production. Existing original geometric controls remain suitable; there is no invented UI slice map. Decorative planets, loose ships, authoring files, and resource ZIP are also not production dependencies.

## Source rectangles and drawing convention

The [reference game layer](https://github.com/meemknight/game-in-cpp-full-course/blob/a9b224196ad57fcfd6da2f70fd989ca4fa6ab5e1/src/gameLayer/gameLayer.cpp) selects the player and possible hostile cells below. The specific Starfall role assignment is an overhaul design choice, not a claim that the reference had these hostile classes or a Dreadnought.

| Starfall role | Cell (column,row) | SDL source rectangle (x,y,w,h) |
| --- | --- | --- |
| Player | (3,0) | (384,0,128,128) |
| Scout | (0,0) | (0,0,128,128) |
| Gunner | (0,1) | (0,128,128,128) |
| Tank | (2,0) | (256,0,128,128) |
| Dreadnought | (3,1) | (384,128,128,128), with distinct Starfall scale/treatment |
| Friendly projectile | (1,1) | (500,500,500,500) |
| Hostile projectile | (0,0) | (0,0,500,500) |

Projectile selection is verified against the [reference projectile renderer](https://github.com/meemknight/game-in-cpp-full-course/blob/a9b224196ad57fcfd6da2f70fd989ca4fa6ab5e1/src/gameLayer/bullet.cpp). SDL should use the original PNG orientation and these unpadded rectangles, with a center pivot. For an upward-facing source sprite and screen-space direction `(dx,dy)`, the clockwise SDL angle is `atan2(dy,dx) × 180 / pi + 90` degrees. For the existing ship draw interface, which already passes that orientation in radians, convert the supplied angle once rather than adding 90 degrees twice.

The four recorded reference parallax strengths are 0, 0.2, 0.4, and 0.7. The [tiled renderer](https://github.com/meemknight/game-in-cpp-full-course/blob/a9b224196ad57fcfd6da2f70fd989ca4fa6ab5e1/src/gameLayer/tiledRenderer.cpp) applies a world offset opposite the camera before camera projection; preserve that convention deliberately rather than assuming those values are direct screen-scroll fractions. Visual tiling, camera bounds, and sprite/shot direction still require integrated game captures.

## Companion library acquisition

Official SDL2 companion releases verified through the GitHub release API are SDL2_image 2.8.8, SDL2_ttf 2.24.0, and SDL2_mixer 2.8.1. Each supplies Windows x64 runtime and VC/MinGW development archives. Release asset digests were null in the returned metadata, so this note does not invent upstream archive SHA-256 values. [Image release](https://github.com/libsdl-org/SDL_image/releases/tag/release-2.8.8), [font release](https://github.com/libsdl-org/SDL_ttf/releases/tag/release-2.24.0), [mixer release](https://github.com/libsdl-org/SDL_mixer/releases/tag/release-2.8.1).

Direct shell downloads could not connect, the GitHub connector does not download binary release archives, and the browser inventory contained no available browser. The reproducible alternative is to mirror the official pinned source files via the GitHub connector and build the same library versions locally. No unrelated vendored DLL is substituted.

| Library | Exact source revision | Intended local configuration |
| --- | --- | --- |
| SDL2_image 2.8.8 | `c1bf2245b0ba63a25afe2f8574d305feca25af77` | PNG through its bundled stb_image implementation; optional external codecs disabled. |
| SDL2_mixer 2.8.1 | `171eb2d420d5643e4ee11514a06e04a41a463bbd` | FLAC through its bundled dr_flac implementation, WAV support; unrelated external music codecs disabled. |
| SDL2_ttf 2.24.0 | `2a891473eaf05ba1707a4b7913e6c4db7de7458a` | FreeType, no HarfBuzz requirement for the supported ASCII pilot/menu text. |
| FreeType 2.13.3 | `42608f77f20749dd6ddc9e0536788eaad70ea4b5` | Static library, external zlib/bzip2/PNG/HarfBuzz/Brotli dependencies disabled. |

Sources live in `third_party/<library>/upstream`; source-lock manifests record each pinned file. `python third_party/tools/verify_sources.py` passed for all 752 files: 45 image, 113 mixer, 16 font, and 578 FreeType files. Verification compares actual byte length and Git blob SHA-1, not merely the SHA reported by the connector. One Latin-1 TGA source comment needed targeted byte restoration after connector transcoding; it now matches its original blob exactly. SDL2 itself remains the original verified 2.30.10 x64 runtime.

The local wrapper builds shared SDL_image/mixer/ttf and static FreeType, enables PNG/stb and FLAC/dr_flac, and disables unused external formats. Complete SDL2 import definitions were generated from the pinned runtime's 845 exported names for MSVC. Both compiler routes use that same unchanged DLL.

## Verified binaries and runtime closure

CMake 4.1.2 built the development-only resource probe with GCC 15.2.0/Ninja and MSVC 19.50.35725.0/Visual Studio 18 2026 x64. Each passed real PNG texture creation for all eight selected images, font rasterization, and FLAC decoding/mixer playback. Linked-version reports are image 2.8.8, font 2.24.0, mixer 2.8.1. Dummy audio proves decoding/mixing, not audible speakers. No complete game visual or balance validation is claimed by these probes.

Every probed DLL is PE x86_64. GCC companion DLLs directly import only `KERNEL32.dll`, `msvcrt.dll`, and `SDL2.dll`; MSVC companion DLLs directly import only `KERNEL32.dll` and `SDL2.dll`. SDL2's imports are Windows system DLLs (kernel, user, graphics, multimedia, IME, COM, version, security, setup, and shell). There is no FreeType, PNG, FLAC, compiler-runtime, or additional codec DLL to stage. Stage the three companion DLLs plus the existing SDL2 runtime.

The [manifest](../third_party/RESOURCE-MANIFEST.md) records these local probe-build hashes. Recalculate hashes for the final game/package build: different compiler, path, or PE timestamp can change bytes without changing the pinned source configuration. Reports and direct PE imports are saved under `build/resource-research` as `probe-gcc.txt`, `probe-msvc.txt`, and `runtime-inspection.json`.

The reproducible probe source/project are `third_party/tools/verify_formats.c` and `third_party/tools/probe`. For a CMake/toolchain already available in PATH:

```powershell
cmake -S third_party/tools/probe -B build/resource-probe-gcc -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release
cmake --build build/resource-probe-gcc --parallel
cmake -S third_party/tools/probe -B build/resource-probe-msvc -G "Visual Studio 18 2026" -A x64
cmake --build build/resource-probe-msvc --config Release --parallel
$env:SDL_VIDEODRIVER = 'dummy'
$env:SDL_AUDIODRIVER = 'dummy'
./build/resource-probe-gcc/starfall_resource_probe.exe ./resources/reference
./build/resource-probe-msvc/Release/starfall_resource_probe.exe ./resources/reference
```

This workspace used the CMake/Ninja executables bundled with Visual Studio 18 Community; the GCC configure also supplied their absolute executable paths. The standalone probe is a development evidence tool and is excluded from the portable game. The wrapper still builds the actual libraries used by production; no fake codec or substitute resource is involved.

## Reproduction and remaining evidence

The ignored `build/resource-research` directory contains the downloaded candidate bytes, resource ZIP, inspection script, and `inspection.json`. The script uses Python, Pillow, and fontTools to fully decode PNGs, inspect TrueType name metadata, parse the FLAC STREAMINFO/comments, check ZIP integrity, and compute SHA-256 plus Git blob SHA-1. These are development evidence tools; they are not shipped runtime dependencies.

The user's exact permission text/source remains forthcoming for the distribution notice. Local resource staging is authorized by their statement; neither this note nor that statement supplies missing license text to reproduce in a release. Source, format, binary architecture, and DLL-closure evidence is complete for local integration. Integrated game captures, error recovery, screen flow, real-device audio, and final package evidence belong to their later tickets.

## Integrated player and background presentation

Ticket 06 verified the additional [TiledRenderer header](https://github.com/meemknight/game-in-cpp-full-course/blob/a9b224196ad57fcfd6da2f70fd989ca4fa6ab5e1/include/gameLayer/tiledRenderer.h): its default tile extent is 10000 world units. The pinned game layer uses a 250-unit ship. Starfall uses 2000-unit background tiles around its approximately 54-unit player sprite, rather than treating a 500-pixel source image as 500 world units. A dark overlay reduces background contrast for readable combat; original image bytes and the four recorded parallax strengths are unchanged.

The integrated player uses cell `(3,0)` with a center pivot. Both projectile cells are now displayed: pink friendly `(1,1)` and cyan hostile `(0,0)`, with gold modulation for overdrive. Directional captures verify rotation from facing/velocity, and boundary captures verify viewport coverage with the bounded camera. No gameplay tuning or collision/score rule changed with these presentation choices.

## Integrated hostile and hull presentation

Ticket 07 implements all four recorded hostile mappings using the same upward-source, center-pivot convention. Display extents are Scout 56, Gunner 66, Tank 80, and Dreadnought 184 logical pixels. The Dreadnought reuses cell `(3,1)` with a larger scale and an original pulsing command ring. Its dedicated HUD count/bar reads actual simulation durability; neither this treatment nor the hostile role names are attributed to the reference.

The hull frame and fill are continuous aligned images, not pre-segmented art. Pixel inspection found the shared interior at `x=4..123`, `y=4..11`. At 2x display scale, Starfall clips matching fill source/destination widths and draws four separators only inside that interior to make five equal-capacity slots. The complete outer frame remains visible at every hull value. Captures from zero through five were inspected for intact borders and no fill beyond remaining capacity. Original asset bytes and hashes are unchanged. See [combat presentation](COMBAT.md) and [validation evidence](VALIDATION.md).
