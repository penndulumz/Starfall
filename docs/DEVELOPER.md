# Starfall developer guide

Use this guide to build and maintain the Windows x64 C++17 game. The [architecture diagrams](ARCHITECTURE.md) describe current ownership and frame flow; the [lecturer walkthrough](LECTURER.md) provides a shorter explanation and repeatable demonstration. [USER.md](USER.md) covers operation and recovery, while [PLAYER.md](PLAYER.md) covers game rules.

## Build, test, and stage

Rebuilding needs CMake 3.20 or later and either Visual Studio's **Desktop development with C++** workload (including CMake tools and a Windows SDK), or a Windows x64 GCC toolchain with Ninja/`mingw32-make`. The source includes pinned dependency inputs; no old `dist` folder or dependency download is needed. Playing a supplied package needs no build tools.

From the source root, the beginner entry point is `BUILD.bat -Test`. For scripting without its final pause:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain GCC -Test -BuildDirectory build/lecturer-gcc -StageDirectory build/lecturer-stage
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain MSVC -Test -BuildDirectory build/lecturer-msvc -StageDirectory build/lecturer-stage-msvc
```

The wrapper searches PATH, then Visual Studio's CMake/Ninja through `vswhere`; GCC may also be discovered in common Scoop/MSYS2 locations. It installs/downloads nothing. `Auto` chooses GCC when found, otherwise MSVC. Default output directories are `build/gcc` or `build/msvc`, with installation into `dist`. Explicit paths may be source-relative or absolute; use a fresh build directory when changing compiler/generator.

[CMakeLists.txt](../CMakeLists.txt) owns sources, options, linking, tests, runtime staging, and installation. [Build.ps1](../scripts/Build.ps1) discovers tools and invokes configure, build, optional CTest, and install. A failed build or test prevents installation. Close a running executable before replacing it. Installation copies declared files but does not empty a preexisting stage directory: choose a fresh directory when checking package contents.

### Direct CMake

When CMake/CTest are on PATH, the Visual Studio route is:

```powershell
cmake -S . -B build/msvc-direct -A x64 -DBUILD_TESTING=ON
cmake --build build/msvc-direct --config Release --parallel
ctest --test-dir build/msvc-direct -C Release --output-on-failure
cmake --install build/msvc-direct --config Release --prefix build/portable-msvc
```

CMake selects the installed Visual Studio generator. GCC with Ninja uses:

```powershell
cmake -S . -B build/gcc-direct -G Ninja -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build/gcc-direct --parallel
ctest --test-dir build/gcc-direct --output-on-failure
cmake --install build/gcc-direct --prefix build/portable-gcc
```

For Ninja outside PATH, supply `-DCMAKE_MAKE_PROGRAM=<full-path>`, or use the wrapper. For an installed `mingw32-make`, select `-G "MinGW Makefiles"`. Supplied Windows builds use static compiler runtimes. Core/app libraries are explicitly STATIC; the SDL libraries are DLLs.

### Portable ZIP and source/history handoff

Commit tracked changes, then choose fresh stage/output directories. Packaging preserves existing directories/archives and fails rather than mixing them with a new build. This command configures and builds through the same wrapper; the CMake `starfall_package` target runs both registered test groupings, installs the declared runtime files, writes `PACKAGE-MANIFEST.json`, and creates the ZIP/checksum:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain GCC -Package -BuildDirectory build/release-gcc -StageDirectory build/release-stage -PackageOutputDirectory releases/new-release
python scripts/VerifyPackage.py releases/new-release/Starfall-Windows-x64.zip
```

Use `-Toolchain MSVC` and different build/stage/output directories for the other compiler. Python 3 is needed only for the optional archive-verification/source-lock tools, never for playing. CMake owns installation and portable archive contents; the wrapper has no second runtime file list. With an already configured build, `cmake --build build/release-gcc --config Release --target starfall_package` performs the same package acceptance operation. Set `STARFALL_PACKAGE_STAGE` and `STARFALL_PACKAGE_OUTPUT` when configuring directly. Packaging requires the bundled Windows x64 dependency route and `BUILD_TESTING=ON`.

The manifest records the full source commit, compiler, pinned versions, and every installed file's relative path, byte length, and SHA-256. It excludes itself from its own hash list; the archive checksum stays in a separate `.sha256` sidecar. This identifies exact output bytes without promising identical compiler output on another machine. An extracted package still needs the application, compatibility, failure-case, and visual checks described in [VALIDATION.md](VALIDATION.md).

For the GCC recipe above, extract to a fresh path with spaces and run the existing application coordinator against its DLLs/resources. The helper is added only to this disposable extraction and removed afterward; it is never installed or archived. Use the discovered CMake executable's full path if `cmake` is absent from PATH.

```powershell
$extracted = Join-Path $PWD 'build/Extracted Starfall'
$checks = Join-Path $PWD 'build/extracted-checks'
Expand-Archive -LiteralPath releases/new-release/Starfall-Windows-x64.zip -DestinationPath $extracted
Copy-Item -LiteralPath build/release-gcc/starfall_app_tests.exe -Destination $extracted
$env:SDL_VIDEODRIVER = 'dummy'
$env:SDL_AUDIODRIVER = 'dummy'
cmake "-DAPP_TEST=$extracted/starfall_app_tests.exe" "-DRUNTIME_DIR=$extracted" "-DQA_DIR=$checks" "-DLEGACY_FIXTURE=$PWD/tests/fixtures/leaderboard-v1.txt" -P tests/RunApplicationChecks.cmake
if ($LASTEXITCODE -ne 0) { throw 'Extracted application acceptance failed; retain QA output for diagnosis.' }
Remove-Item -LiteralPath "$extracted/starfall_app_tests.exe"
```

The coordinator creates an unrelated working directory and isolated saves, then runs full application/legacy/audio cases and disposable missing/corrupt image/font packages. MSVC's helper is `build/<name>/Release/starfall_app_tests.exe`. Inspect the eight screen captures plus small/error states, recheck manifest hashes after resource restoration, and launch production with an absolute isolated `--data-dir`. `PLAY.bat --check` verifies required filenames, while the application tests verify actual decoding. Close the check shell or remove these two SDL environment variables before checking real display/audio devices.

Create the separate source-only ZIP and local Git history bundle from a committed checkout:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/ArchiveSource.ps1 -OutputDirectory releases/new-release
git bundle verify releases/new-release/Starfall-History.bundle
git clone -b integration/starfall-overhaul releases/new-release/Starfall-History.bundle build/restored-history
```

The bundle preserves `main`, the integration branch, and the selected revision (default HEAD). A pre-merge candidate can be checked out by the source ZIP's full `SOURCE-COMMIT.txt` value. The source ZIP includes tracked sources, pinned dependency inputs, resources, tests, and documentation plus that provenance marker. It excludes generated `dist`, build outputs, releases, and Git metadata. Extract it and use `BUILD.bat -Test` to rebuild; for no-compiler play use the portable ZIP. Source-only packaging is deliberate, so cleaning generated outputs cannot remove build prerequisites. Keep these packages local/private under the recorded user asset authorization until exact permission terms/source are supplied; the code license does not grant asset rights.

### Inputs and outputs

| Location | Responsibility |
| --- | --- |
| `src`, `tests`, CMake and scripts | Source and approved test groupings |
| `third_party/SDL2` | Pinned SDL2 headers, import definitions, runtime, notices |
| Other `third_party` directories | Pinned companion/FreeType sources, source locks and notices |
| `resources/reference` | Selected unchanged runtime artwork, font, clip, and notice |
| `build/<name>` | Disposable compiler outputs, isolated scores and QA captures |
| `dist` or explicit stage | Installed runtime/resources and user-facing documentation |
| SDL current-user preference folder | Player leaderboard, separate from all the above |

`STARFALL_BUNDLED_SDL_ROOT` can select an alternate root with the same dependency layout. `STARFALL_USE_SYSTEM_SDL2=ON` supports an installed SDL2 development package; Linux/macOS remain unvalidated release targets. The exact dependency versions, source verification, and notice choices are in the [manifest](../third_party/RESOURCE-MANIFEST.md).

## Source map and ownership

| Source | Interface and responsibility |
| --- | --- |
| [main.cpp](../src/main.cpp) | Parses `--data-dir`/`--help`, constructs Game, reports startup exceptions |
| [app/Game.hpp](../src/app/Game.hpp), [Game.cpp](../src/app/Game.cpp) | Owns SDL window/renderer, Run, Draw, Sound, Scores; coordinates input, transitions, timing, save actions, cleanup |
| [app/ScreenLayout.hpp](../src/app/ScreenLayout.hpp), [ScreenLayout.cpp](../src/app/ScreenLayout.cpp) | Screen/action types; button values, labels, enabled states and logical rectangles; shared pilot-name field |
| [app/Screens.cpp](../src/app/Screens.cpp) | Game's welcome, gameplay/HUD, pause, controls, leaderboard, and end drawing methods |
| [simulation/Run.hpp](../src/simulation/Run.hpp), [Run.cpp](../src/simulation/Run.cpp) | SDL-independent state, input/events, movement, collisions, hostiles, pickups, waves/results |
| [simulation/Rules.hpp](../src/simulation/Rules.hpp), [Geometry.hpp](../src/simulation/Geometry.hpp) | Shared named tuning; vectors, normalization, clamping and collision declaration |
| [presentation/Draw.hpp](../src/presentation/Draw.hpp), [Draw.cpp](../src/presentation/Draw.cpp) | SDL drawing, cached text, sprites, parallax, hull; owns VisualResources |
| [presentation/VisualResources.hpp](../src/presentation/VisualResources.hpp), [VisualResources.cpp](../src/presentation/VisualResources.cpp) | Owns eight decoded textures, font handles and image/font library lifetime |
| [platform/Paths.cpp](../src/platform/Paths.cpp) | Shared executable-relative `resourceDirectory()` |
| [audio/Sound.hpp](../src/audio/Sound.hpp), [Sound.cpp](../src/audio/Sound.cpp) | One mixer/device, shooting clip, original generated chunks, channels, mute and warnings |
| [persistence/Scores.hpp](../src/persistence/Scores.hpp), [Scores.cpp](../src/persistence/Scores.cpp) | Name validation, retained ranking, V1 loading and safe replacement |
| [tests/core_tests.cpp](../tests/core_tests.cpp) | Focused gameplay/persistence behavior and complete campaign |
| [tests/application_tests.cpp](../tests/application_tests.cpp), [coordinator](../tests/RunApplicationChecks.cmake) | Real application flow/resources, controlled scenarios and captures; disposable resource failures |
| [tests/support](../tests/support) | Test-only Run setup and real persistence-failure support |

Game owns Run by value and the other main objects with `unique_ptr`. Screens.cpp adds methods to Game; there is no separate screen engine. Run owns private mutable state and a seeded random generator. `Run::start(seed)` resets it, `update(seconds,input)` changes it, and `view()` returns a borrowed `const RunState&`. The view reflects later updates; it is not a frozen snapshot. Updates may invalidate references/iterators into entity vectors. Copy a snapshot if it must survive an update.

Draw owns VisualResources and a bounded 256-entry text cache. Fonts/textures use unique_ptr deleters; library declaration order ensures they are destroyed before TTF/image quit. Game resets Draw/Sound before destroying the renderer/window and quitting SDL. Sound stops channels before freeing chunks and closing its mixer. A constructor failure unwinds already-created owners; Game also cleans up partially initialized SDL handles. See the [ownership diagram](ARCHITECTURE.md#ownership-and-data-flow).

## Frame, coordinates, and algorithms

`Game::execute` measures frame time, advances bounded UI animation time, drains events, samples held input, advances active gameplay, draws, presents, and yields briefly. `advance` accumulates at most 0.1 seconds from a rendered frame, then runs zero or more `Rules::FixedStep` updates of 1/120 second. Input is reused within those steps. This bounds catch-up after a long interruption. A screen transition resets the accumulator.

Run events belong to its latest update and are consumed inside the step loop; otherwise several updates before a render could lose sound cues. Paused/menu frames continue drawing and UI animation while Run timers, damage and progression freeze. Focus loss enters pause. An ended Run stops stepping and opens the end screen. The [frame sequence](ARCHITECTURE.md#one-frame) shows these calls.

### Logical input and world drawing

The logical canvas is 1280x800; world bounds are 2600x1800. World x points right, y down; speeds use world units per second, timers seconds. A bounded camera follows the player. Continuous mouse aim is logical mouse position plus camera; drawing subtracts camera plus shake.

`SDL_RenderSetLogicalSize` already transforms queued mouse events, including letterboxing after resize. `Game::event` uses those coordinates directly. Only raw `SDL_GetMouseState` is converted with `SDL_RenderWindowToLogical`. Converting events twice would break alignment.

`screenLayout` supplies button values before input and after transitions; drawing uses the same rectangles. The named `PilotNameField` serves both drawing and hit tests. Correct input therefore does not wait for a previous render. Clicking the name field restores Save focus, and clicking a HUD control suppresses firing.

### Simulation order and collision

Run advances timers, player/weapons, spawning, hostiles, projectiles, pickups, particles, cleanup, then wave completion. Cleanup removes dead hostiles before testing the remaining count. A repair cannot undo a defeat earlier in the step. Victory requires the final wave's queued and live hostiles to be empty; a dead boss with surviving supports is insufficient. Completed runs remain frozen.

`segmentHitsCircle` projects a target's center onto the projectile's traveled segment, clamps the fraction to [0,1], and compares squared distance with squared hit radius. A stationary segment tests current overlap. Checking only the endpoint could skip a target at high speed; this swept check uses the entire shot path. Hostiles are tested in collection order, and a friendly bullet stops at its first registered hit.

Seeded starts and fixed input/steps make regression scenarios reproducible in the same environment. Different standard-library distributions/floating-point implementations need not produce identical campaign time; the tests assert rules and outcomes instead.

### Safe score replacement

Scores loads `leaderboard.txt` with header `STARFALL_SCORES_V1`. Rows retain quoted name, points, reached wave, whole elapsed seconds, and victory flag. Validation limits remain twelve supported ASCII characters, points 0..1,000,000,000, waves 1..10, and seconds 0..86,400. Malformed rows are skipped without losing valid neighbors; unknown headers/unreadable files expose warnings.

Loading stably sorts descending and keeps ten. Submission inserts a new equal score after existing equals, truncates a copied candidate, then writes, flushes and closes `leaderboard.tmp`. Windows `MoveFileExW` replaces the destination with replacement/write-through flags; only successful replacement publishes the candidate to memory. A failure keeps previous disk and memory entries and permits retry. This is not a promise against arbitrary power loss or concurrent writers.

Game enforces explicit once-per-completed-Run submission; Scores permits identical results from different Runs. Game selects `SDL_GetPrefPath("Starfall","Starfall")` or `--data-dir`; resource location never chooses saves. See [SAVES.md](SAVES.md).

## Resources, audio, and failure handling

`resourceDirectory()` uses SDL's executable base directory plus `resources/reference`, independently of process working directory. VisualResources validates all eight PNG dimensions and the required font at startup. Draw uses white cached text textures with color modulation, nearest atlas scaling, and center pivots. [COMBAT.md](COMBAT.md) records exact ship cells, display extents, five-segment hull clipping and boss treatment.

The four source backgrounds are 500px squares rendered as 2000-world-unit tiles around a roughly 54-unit player. The reference uses 10000-unit tiles around a 250-unit ship; directly treating source pixels as world units made this scene too dense. A dark overlay keeps combat readable. Reference world-offset strengths remain `0, 0.2, 0.4, 0.7`: offset is `-camera*strength`, then camera projection is subtracted again, giving screen scroll `-camera*(1+strength)`. Floor-based tile origins preserve coverage when shake crosses zero. The faint world grid and geometric pickups/effects remain original feedback.

Sound owns a single SDL2_mixer path for the reference shooting FLAC and original generated event chunks. It loads clips once, uses a bounded channel pool, and drops new feedback if channels are full. Missing devices leave play available; missing/corrupt audio resources identify a package defect. See [AUDIO.md](AUDIO.md) for formats, cleanup and recovery.

| Failure | Application behavior / maintenance response |
| --- | --- |
| Required PNG/font missing, corrupt or wrong dimensions | Startup error identifies the resource and asks to restore the package; partial owners clean up |
| PNG/font library or SDL/window/renderer initialization | Startup error; main reports after Game cleanup |
| Audio device unavailable | Footer warning, silent playable game; retry device initialization by restarting |
| Audio clip missing/corrupt or decoder unavailable | Resource warning; gameplay continues but release validation must fail |
| Default save path unavailable | Startup error suggests writable `--data-dir` |
| Scores unreadable or unknown format | Leaderboard warning; retain a backup before repair |
| Score temporary write/final replacement fails | Actionable end-screen error, unchanged previous entries, Save can retry |

### Making changes

- Change gameplay tuning in Rules, then check the focused behavior and HUD values. Preserve units and final-result invariants; presentation must not mutate the view.
- For a new screen action, update Screen/Action, shared layout, Game dispatch, drawing, and an application outcome scenario together. Keep all hit rectangles in the layout.
- For a resource change, establish its provenance/terms, exact bytes/dimensions and atlas mapping first. Update the manifest, VisualResources or Sound, CMake install inputs, credits and real-resource captures. Add a new dependency only through the authoritative CMake/source-lock path.
- Retain original resource bytes and notices. To verify the 752 pinned dependency source files, run `python third_party/tools/verify_sources.py`.
- Keep DLL/resource copies under the single `starfall_resources` target. Both game/tests depend on it; competing executable post-build copies caused a real race. The default build refreshes asset-only changes.
- The local MinGW RC rule preprocesses resource files with the C compiler before windres, preserving quoted include paths when the extracted source folder contains spaces. Upstream source/notice bytes remain untouched.
- Keep fixtures and capture helpers in tests. No public mutation setters or production demo shortcuts are required.

## Verification and documentation

CTest retains two approved groupings: `gameplay_and_scores` and `screen_flow`. Core tests exercise rules, real collisions/campaign progression, ranking, compatibility and filesystem failures. Application tests link the same `starfall_app` library, exercising production initialization/navigation/rendering/save code with real packaged resources. Test-only friends set up difficult states, then assert observable outcomes.

Application flow runs from an unrelated working directory with SDL dummy drivers. Its coordinator passes the fictional legacy fixture explicitly, and uses disposable package copies for missing/corrupt visuals. Audio cases restore altered staged files. Unique save folders, BMP captures and temporary packages stay under the build's QA directory; test binaries/fixtures are not installed.

After the documented GCC build, run application flow alone with:

```powershell
ctest --test-dir build/lecturer-gcc -R screen_flow --output-on-failure
```

Captures are in `build/lecturer-gcc/qa/navigation`; MSVC uses the corresponding build directory, without an extra Release component for QA. The [lecturer script](LECTURER.md#repeatable-demonstration) names a useful viewing sequence. It covers natural controls plus controlled dash/pickups, all hostiles/hull values, final supports/victory, legacy scores and save retry.

Current evidence is in [VALIDATION.md](VALIDATION.md). Automated progression with an invulnerable test pilot is not human balance; dummy mixing is not audible output; dummy resize/fullscreen checks are not real-monitor acceptance. Inspect screenshots when changing presentation and record those manual boundaries.

The source documentation provides developer/lecturer/architecture detail. CMake installs the user/player/save/combat/audio guides, credits, manifest, notices, and current screen previews with the game. Resource evidence includes historical planning context intended for the full source project. Keep local links and audience routes accurate whenever changing those documents.
