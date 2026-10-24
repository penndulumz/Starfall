# Validation

## Current overhaul evidence, 2026-10-24

The implemented overhaul now includes private Run ownership, shared input layouts, all sprite/hull/background/font presentation, loaded shooting audio, complete menus, and unchanged V1 score compatibility. The per-ticket sections below retain their original scope and results; an earlier ticket's pending wording describes that historical stage.

| Evidence | Established result | Limit |
| --- | --- | --- |
| Windows x64 GCC 15.2.0 and MSVC 19.50/Visual Studio 2026 builds | Both approved CTest groupings pass; clean portable candidate archives and extracted-package checks are verified | Final provenance and hashes are recorded in generated manifests/sidecars; physical display/audio checks remain unverified |
| Focused gameplay/persistence | 61 checks; all 131 hostiles cleared through real collisions; scoring, V1 compatibility, stable ties and failed-save retry | Invulnerable campaign pilot tests progression, not human balance; GCC193/MSVC188 simulated seconds are not a cross-compiler timing contract |
| Full application flow | Startup/resource decoding, navigation before first render, pause/focus loss, save/reopen/retry, real final-wave supports and victory, missing/corrupt visuals/audio, unavailable device | Controlled fixture setup remains outside production |
| Inspected software-rendered captures | Welcome, combat, pause/manual, defeat/name entry, leaderboard, victory, all ship directions/types, hull 0..5, map/HUD and final encounter | Real-monitor fullscreen/resize appearance and physical input remain unverified |
| SDL dummy audio | FLAC decoding/mixing, all event effects, mute and recovery paths | Audible speaker output remains unverified |
| Resource/dependency record | Selected original bytes, pinned sources/versions, decoder/runtime closure and notices recorded in the manifest | Exact reference-asset permission terms/source remain forthcoming after the user's permission attestation; probe DLL hashes are not final-package hashes |

Current screen previews are actual capture conversions under [screenshots](screenshots). The [lecturer walkthrough](LECTURER.md) identifies controlled demonstration steps and the [developer guide](DEVELOPER.md) gives reproducible build/test/install commands. Course approval, human balance, physical display/audio acceptance, and final release acceptance are not inferred from automated checks.

## Historical completed-game baseline

The following baseline predates the overhaul and describes the original procedural presentation. Its old DLL/resource and embedded smoke-test observations do not describe the current game. Current builds require packaged PNG/font/FLAC resources and all four SDL DLLs; scenario scripts now live in the external application test executable.

Validated on Windows x64 on 2026-10-23 with SDL2 2.30.10.

- GCC 15.2.0: clean build with `-Wall -Wextra -Wpedantic`; `scripts/Build.ps1 -Test` passed.
- MSVC 19.44 / Visual Studio 2022: Release CMake build passed; both CTest suites passed.
- 48 gameplay and persistence checks cover movement, bounds, dash cooldown, swept projectile hits, firing rate, damage protection, death, pickups, score chains, wave progression, boss attacks, resets, deterministic spawning, name validation, ranking, ties, top-ten truncation, reloads, malformed rows, and failed writes.
- A complete campaign simulation defeated all 131 hostiles, including the Dreadnought, and reached victory after 193 simulated seconds. The test pilot was invulnerable to isolate progression from bot skill; this is not a difficulty-balance playtest.
- Screen-flow checks cover keyboard start, mouse pause, frozen simulation while paused, controls and back navigation, focus-loss pause, defeat, name entry, single submission, disk reload, a full leaderboard, retry, boss rendering, victory, and mouse menu navigation.
- SDL audio initialization, effect mixing, and mute/unmute were exercised with its dummy audio driver. Automated tests do not verify audible speaker output.
- Eight 1280x800 software-rendered screen captures were visually inspected. Welcome and gameplay previews are in `docs/screenshots/`; the full generated set is under `build/qa/screens/`.
- The executable's imported DLLs are `KERNEL32.dll`, `msvcrt.dll`, and the supplied `SDL2.dll`. SDL2 itself imports only Windows system DLLs. No image, font, or audio asset files are needed at runtime.
- The portable ZIP passed archive integrity and file-list checks. Its extracted executable also passed the full screen-flow and audio checks from a separate working directory.

Test profiles and sample leaderboard entries are confined to `build/`. They are not included in the portable game or written to the player's normal save folder. Linux and macOS have not been tested.

## Overhaul audio slice, 2026-10-24

- GCC 15.2.0 and MSVC 19.50.35725.0 x64 builds passed both registered test groupings after the audio change.
- Application scenarios exercise the real shooting FLAC through SDL2_mixer, all original generated event chunks, mute/unmute, the sound button during name entry, missing/corrupt clip diagnostics and recovery, and gameplay with an unavailable audio driver.
- The missing-clip scenario first failed against the old implementation; a minimal presence guard passed. A valid-header/corrupt-FLAC scenario then failed until real mixer decoding replaced the generated shooting effect. The resulting full application checks pass.
- A shared CMake staging owner fixed an observed concurrent-copy failure between the game and application test executable. Both compiler routes pass with the shared staging target.
- Two audio-warning captures were inspected after resetting the test window to its logical capture size. The resource warning identifies shoot.flac and package recovery; the device warning leaves play available.
- Audio used SDL's dummy driver. Audible speaker output remains unverified. These checks validate this slice, not completed visual-overhaul or final-package acceptance.

## Overhaul ticket 01: authoritative build

The build refactor was verified on 2026-10-23 before the later gameplay/interface/resource tickets. The earlier results above describe the original game; these new results cover the build route with that same gameplay/persistence/application implementation.

- TDD regression: configuring a source-only copy without `dist` failed with the original `Missing dist/SDL2.dll` dependency. After moving the runtime input to `third_party/SDL2/bin`, a fresh source-only copy configured, built, passed both CTest suites, and installed successfully with no `dist` directory.
- CMake 4.1.2-msvc8 was discovered from Visual Studio's installation. The thin wrapper's GCC/Ninja route used GCC 15.2.0; its MSVC route selected Visual Studio 18 2026 and MSVC 19.50.35725.0. Both fresh Release builds passed `gameplay_and_scores` and `screen_flow` (2/2 suites each).
- The focused suite retained 48 checks and completed the 131-hostile campaign in 193 simulated seconds. The application suite retained its full screen-flow and dummy-audio checks. No new production testing seam was introduced.
- Each compiler's CMake installation staged exactly the executable, unchanged SDL2 runtime, SDL2 notice, project license, and standalone start guide. Both staged executables passed application flow from an unrelated working directory with isolated test save directories.
- The dependency input remains SDL2 2.30.10 Windows x64, 2,970,832 bytes, SHA-256 `3be02da904bdeca79b930c481489ce164740b6c722fc1e9dbe77bb10344536d7`. Generated runtime copies are installation outputs, not build prerequisites.
- GCC compiled with `-Wall -Wextra -Wpedantic`. MSVC compiled with `/W4` and reported the existing `strcpy` deprecation in the embedded smoke-test scenario; moving that scenario belongs to ticket 04. This validation does not claim a warning-free MSVC build.
- Checks ran in the isolated ticket workspace. No player-profile scores were read or modified, no new assets were added, and no claim of overhaul visual completion, human balance, or audible speaker output is made.

Commands, run from the ticket source root:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain GCC -BuildDirectory build/gcc-fresh -StageDirectory build/staged-gcc -Test
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain MSVC -BuildDirectory build/msvc-fresh -StageDirectory build/staged-msvc -Test
```

The source-only regression used a copy of source/dependency inputs with no generated `dist`, configured through Ninja with the same GCC compiler, then built, tested, and installed through CMake. Staged startup checks used `--smoke-test` with SDL's dummy video/audio drivers and a working directory outside each installed package.

## Overhaul ticket 03: Run ownership and shared rules

Verified on 2026-10-24 in the isolated ticket source workspace. A compile-time regression first failed because Run had no `view` interface. The implemented interface returns a borrowed const state even from a mutable Run; a compile-time assertion protects that production observation contract, and a new behavioral check observes a freshly started run through it.

- GCC 15.2.0/Ninja and MSVC 19.50/Visual Studio 2026 both built the grouped simulation and passed both existing CTest suites (2/2 each).
- The focused suite now reports 49 checks. Its seeded campaign still defeated all 131 hostiles in 193 simulated seconds through real projectile collisions.
- Existing fixtures now perform setup through test-only support and assertions through the const view. The embedded application smoke scenario temporarily uses the same fixture; ticket 04 removes that scenario and its test include from production application source.
- Shared rules retain the original timing, movement, firing, enemy, pickup, wave, and scoring values. Hull, dash recharge, fixed step, and displayed final-wave count derive from those shared definitions.
- Named internal update stages preserve their old order, including damage before pickup consumption and cleanup before wave completion. No balance adjustment or SDL dependency was added to the simulation.

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain GCC -BuildDirectory build/view-green -StageDirectory build/view-staged -Test
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain MSVC -BuildDirectory build/view-msvc -StageDirectory build/view-staged-msvc -Test
```

As with earlier checks, dummy audio and the invulnerable campaign pilot do not prove audible speaker output or human balance. Git metadata writes were still unavailable, so these results establish source/build behavior without claiming a commit or integration branch.

## Overhaul ticket 04: screen flow and external scenarios

Validated on 2026-10-24 with the Run ownership changes from ticket 03 integrated into the isolated screen-flow workspace.

- Red/green evidence: an application regression initially failed with `Enter before the first render must start a run`. The original welcome controls did not exist until rendering registered them. Explicit shared layouts made immediate Start/Pause/Help/Back/Resume pass without drawing an intervening frame.
- Screen control rectangles, labels, enablement, and focus order now have one source. Drawing consumes those declarations; the pilot-name hit test uses its actual rendered field rather than the older oversized region. Application scenarios cover clicks just above and inside the field.
- `starfall_app_tests` exercises the production `starfall_app` library. Production source no longer includes the embedded smoke script, capture implementation, synthetic campaign setup, or Run test-fixture include. Controlled scenario construction lives in test code, with assertions using the const Run view.
- GCC 15.2.0/Ninja and MSVC 19.50.35725.0/Visual Studio 2026 each built and passed both CTest suites. The imported ticket 03 focused suite passed 49 checks and the 131-hostile progression scenario. The MSVC build no longer reports the embedded smoke script's `strcpy` warning.
- External application flow retains welcome/start, gameplay, pause/freeze, controls/back, focus loss, defeat, name entry, once-only save, disk reopening, full leaderboard/back, retry, Dreadnought rendering, victory, and mouse navigation. It additionally exercises pre-render transitions, keyboard selection immediately after return, exact name hit geometry, name-entry sound controls, and resized/fullscreen mouse events passed through SDL's queue.
- Eight software-rendered BMP captures and the scenario report are produced beneath each build's `qa/navigation`. The defeat/name-field capture was visually inspected during this slice; full release visual acceptance remains with the presentation/release tickets.
- Resize/fullscreen input checks use SDL's dummy video backend. This does not establish real monitor/fullscreen appearance, human balance, or audible speaker output. Sound decoding/loading is still a later resource ticket.

Commands, run from the ticket source root:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain GCC -Test -BuildDirectory build/ticket04 -StageDirectory build/stage
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain MSVC -Test -BuildDirectory build/ticket04-msvc -StageDirectory build/stage-msvc
```

Application automation now runs through CTest's `screen_flow` target or `starfall_app_tests`; the old production `--smoke-test` route described in historical validation above is no longer part of the current executable. Test data is confined to unique directories under the build outputs, never the player's normal profile.

## Overhaul ticket 05: packaged visual-resource startup

Validated on 2026-10-24 first in the isolated candidate and then in the ticket's Git worktree based on the integration branch.

- TDD regression: an application startup scenario expecting a named failure for missing `spaceships.png` initially failed because the old game ignored external resources. Real production initialization now loads all eight reference PNG textures and validates the required font before rendering.
- Both compiler routes build the pinned SDL2_image/ttf/mixer sources and statically included FreeType through the authoritative CMake configuration. Starfall's own core/application libraries are explicitly static. The unchanged SDL2 runtime, all three companion DLLs, resources, manifest, credits, and original dependency notice texts are declared staging/install inputs.
- GCC 15.2.0/Ninja and MSVC 19.50.35725.0/Visual Studio 2026 passed the focused and full-application groupings. Application flow starts from an unrelated working directory using executable-relative resources. The coordinator also makes disposable staged copies and verifies missing PNG, corrupt PNG, missing font, and corrupt font errors through the same production initialization; each names the resource and asks for the complete package to be restored. The test route catches failures without a modal dialog.
- The welcome screen uses the verified original player cell `(3,0)`. The selected font replaces custom glyph rendering throughout all screens; changing score/time text uses a bounded 256-texture cache. All eight default-size captures were visually inspected for readable text, clipping, control alignment, and correct welcome sprite orientation/transparency. Combat sprites/background conversion remains planned in tickets 06/07, and shooting-clip integration remains ticket 08.
- Texture/font/library lifetimes are owned and cleaned up on partial initialization failure before their renderer shuts down. Resource directories and score directories remain independent; all automated saves use isolated QA directories.
- Staged packages contain the four runtime DLLs and actual resource tree. Both tested formats are decoded through the shipped libraries rather than a fake loader or an existence check. This does not establish real monitor/fullscreen appearance, audible speaker output, human balance, or the completed combat-art overhaul.

Commands, run from the ticket worktree:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain GCC -Test -BuildDirectory build/ticket05-gcc -StageDirectory build/stage-gcc
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain MSVC -Test -BuildDirectory build/ticket05-msvc -StageDirectory build/stage-msvc
```

The welcome preview was refreshed from the actual software-rendered capture. Final release hashes, the integrated combat/audio presentation, and extracted-archive acceptance are verified by their later tickets; historical probe hashes are not relabeled as final release hashes.

After merging the integration tip containing ticket 09, both compiler routes passed again with 61 focused checks, legacy score compatibility, and all four PNG/font failure cases. Both installed production executables started cleanly from an unrelated working directory under the dummy drivers, using explicit isolated save paths, and were stopped after 1.2 seconds. All 11 staged resource/notice files matched their source hashes; the test executable was excluded from installation. The shared staging target adopted from the audio slice prevents concurrent copies into a common runtime directory. All 752 vendor source blobs remained unchanged. The save-retry error capture was also inspected with the new font.

## Overhaul ticket 09: saved-score compatibility

Verified on 2026-10-24 against the existing V1 score contract. The new malformed-row regression first demonstrated that `std::quoted` accepted a bare, unquoted pilot name. The parser now rejects that malformed row while retaining valid historical quoted rows. The format header, values, user preference location, custom-directory option, name normalization, ranking, and submission semantics are unchanged.

- Fresh GCC 15.2.0/Ninja and MSVC 19.50/Visual Studio 2026 builds each passed `gameplay_and_scores` and `screen_flow` (2/2 suites). After adoption into the actual ticket worktree, a fresh GCC build and both suites passed there as well.
- The focused suite now reports 61 checks on Windows, including the original complete campaign (131 hostiles in 193 simulated seconds). A tracked, fictional V1 fixture preserves all columns, stable old ties, and old/new results across reopening.
- Real filesystem failure scenarios block temporary-file creation and final replacement separately. A test-only Windows reader without delete sharing blocks replacement while allowing reads. Tests verify returned errors, unchanged retained memory, unchanged previous file bytes, readable prior entries, and successful retry after recovery. No production failure hook is used.
- Application flow opens the old fixture, visits its leaderboard, completes and explicitly saves a run, exposes a failed replacement on the end screen, retries successfully, enforces one submission per Run, permits identical results from separate Runs, and reopens in a fresh application instance. Main Menu, Play Again, and active-run abandonment do not save automatically.
- The application suite adds `09-legacy-leaderboard.bmp` and `10-save-retry.bmp` captures. The save-error capture is Windows-specific; other operating systems remain unvalidated release targets.
- CTest receives the fixture path explicitly. Fixtures, test helpers, generated scores, and captures remain outside installation outputs. No player-profile data was opened or modified.

Initial two-compiler checks ran from the isolated ticket source copy:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain GCC -BuildDirectory build/persistence-gcc -StageDirectory build/persistence-staged -Test
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain MSVC -BuildDirectory build/persistence-msvc -StageDirectory build/persistence-staged-msvc -Test
```

The final adopted ticket worktree was checked with:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain GCC -BuildDirectory build/persistence-gcc -StageDirectory build/persistence-stage -Test
```

This ticket adds persistence evidence, not a claim of final overhaul artwork, audible speaker output, or human balance validation.

## Overhaul ticket 06: player, projectiles, and parallax

Validated on 2026-10-24 in the real ticket worktree. Application-level regressions first failed because gameplay still drew the procedural cyan cockpit and friendly projectile. The checks use independent color characteristics of the known reference cockpit/projectile artwork; they passed once gameplay used the selected atlas cells.

- Player and projectile sprites now use original source rectangles, center pivots, nearest scaling, and the shared clockwise rotation convention. Friendly pink and hostile cyan shots remain visually distinct; overdrive retains paired shots and gold feedback.
- All four 500-pixel background images tile with the reference world-offset strengths 0, 0.2, 0.4, and 0.7. Inspection showed native-pixel world tiles were too dense, so world extent is 2000 units, with a dark overlay for combat contrast. This is scaled relative to the pinned reference's 10000-unit tiles/250-unit ship and does not change simulation rules.
- GCC 15.2.0/Ninja and MSVC 19.50/Visual Studio 2026 passed both approved test groupings: 61 focused checks and complete application scenarios. The campaign defeated all 131 hostiles in 193 simulated seconds with GCC and 188 with MSVC; elapsed time is not an acceptance requirement across different standard-library implementations. Missing/corrupt visual-resource and score-compatibility cases remain green.
- New captures cover all four facing/shot directions, both pickups, a real dash with overdrive paired shots, and both bounded camera extremes. Default gameplay, direction, dash/overdrive, and boundary captures were inspected for sprite direction, shot origins, continuous background coverage, HUD readability, and feedback. The camera and player bounds are asserted through real application advancement before boundary captures.
- Rendering reads the const Run view; no movement, speed, firing, damage, dash, pickup, score, or wave rule changed. Old generated nebula/starfield code was retired. Hostile ship/boss/hull-frame presentation remains ticket 07, while simple pickup/effect geometry remains intentional.

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain GCC -Test -BuildDirectory build/ticket06-gcc -StageDirectory build/stage-gcc
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain MSVC -Test -BuildDirectory build/ticket06-msvc -StageDirectory build/stage-msvc
```

The gameplay and welcome previews were refreshed from actual captures. Dummy-driver captures and automated progression do not establish human difficulty balance, audible speaker output, or real-monitor fullscreen appearance.

After merging the current integration branch's loaded-audio slice, both compiler routes passed both groupings again. The combined application run retains the audio missing/corrupt/device cases and legacy-save fixture while adding combat direction, sprite selection, pickup, dash/overdrive, and bounded-camera captures. No final hostile ship/boss/hull-art completion is claimed by this ticket.

## Overhaul ticket 10: menus and audience guidance

Validated on 2026-10-24 in the actual `ticket/10` worktree after merging both loaded-audio and player/parallax integration.

- Red/green evidence: an application test selected another end-screen action, clicked the name field, typed a supported twelve-character name, and pressed Enter. It initially failed because Enter opened the previously focused action rather than saving. Restoring Save focus on a name-field click made the regression pass without changing submission semantics.
- GCC 15.2.0/Ninja and MSVC 19.50.35725.0/Visual Studio 2026 each passed both approved CTest groupings after the audio merge. The focused suite remains 61 checks with the 131-hostile campaign; application checks retain resource failures, audio, persistence, pause, navigation, and score retry.
- Menu scenarios exercise empty and full leaderboards with maximum supported names/points/times, a long selected name, unsaved/saved and outside-top-ten feedback, disabled Save, and unavailable-audio presentation. All scores remain in isolated build directories.
- Welcome, manual, pause, populated leaderboard, defeat/name entry, and victory captures were inspected at 1280x800 and actual 800x500 renderer output. Long-name/outside-top-ten and existing save-retry error states were also inspected for clipping and overlap. Reference player artwork now appears on both end screens; name-selection width uses the loaded font.
- Six primary screen previews were refreshed. Smaller captures remain under the build's QA directory. Dummy-driver resize/fullscreen coordinate checks remain automated evidence, not a claim of real-monitor/fullscreen or audible-speaker acceptance.
- Separate user/player guides are linked from the README, beginner setup, and packaged read-first text, and installed by CMake with save guidance and screenshots. Local links were checked in the source and installed user/player guides. Developer and architecture explanations were updated in the same ticket.
- Combat sprite/parallax changes belong to their separate tickets; this slice does not change gameplay, scoring, persistence format, or balance. Loaded-audio footer diagnostics and scenarios from ticket 08 are retained.
- After ticket 06 arrived during final integration, its combat checks and rationale were retained alongside the menu scenarios and both compiler suites passed again. Menu previews were regenerated and inspected against the integrated reference backgrounds. The complete campaign remained 131 hostiles; the compiler-specific elapsed times remain progression evidence rather than a cross-toolchain timing requirement.

Commands from the ticket worktree:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain GCC -Test -BuildDirectory build/ticket10-gcc -StageDirectory build/stage
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain MSVC -Test -BuildDirectory build/ticket10-msvc -StageDirectory build/stage-msvc
```

## Overhaul ticket 07: hostiles, hull, and final encounter

Validated on 2026-10-24 in the real ticket worktree after merging the integration branch containing tickets 06, 08, 09, and 10.

- Application red/green checks first rejected the old procedural Scout, then the remaining procedural hostile bodies. Literal pixel samples from the verified original artwork pass once the documented four atlas cells are rendered. These checks observe actual application output rather than private draw helpers or draw-call counts.
- A second red/green slice rejected the geometric hull. The imported frame/fill now presents five equal-capacity slots. All six hull values are captured; filled/empty slot pixels are asserted, and frame borders, separators, partial fill, and empty capacity were visually inspected. The old procedural ship implementation is removed after its final end-screen callers were converted by ticket 10.
- GCC 15.2.0/Ninja and MSVC 19.50.35725.0/Visual Studio 2026 passed both approved CTest groupings and installation. All 61 focused checks remain, including the complete 131-hostile campaign through real collisions. Compiler-specific campaign time was 193 simulated seconds with GCC and 188 with MSVC; this is progression evidence, not a timing/balance contract.
- The combined application suite retains menu/small-window, persistence/legacy fixture, visual-resource failure, and audio/missing-device cases. New fixtures render every hostile type facing the player, damaged hostile strips, a populated HUD, offscreen markers, distinct friendly/hostile shots during overdrive, and five-segment hull values zero through five.
- The real final-wave scheduler creates the Dreadnought plus four supports. Fifty-five collision hits produce the half-durability capture; another fifty-five destroy the boss while all four supports keep the run active. The last support's real collision opens the victory end screen, with the fifth-kill multiplier and unchanged final-wave/victory/remaining-hull bonuses asserted independently.
- Captures `15-scout-atlas.bmp`, `16-hull-0.bmp` through `16-hull-5.bmp`, `17-hostiles-and-hud.bmp`, `18-final-wave.bmp`, `19-dreadnought-half-hull.bmp`, `20-supports-remain.bmp`, and `21-whole-wave-victory.bmp` are under each build's `qa/navigation`. Hostile/HUD, initial/half boss, supports-remaining, and victory images were inspected. Published gameplay and Dreadnought previews are direct PNG conversions of actual captures. The all-types preview is a controlled wave-ten presentation fixture.
- Production simulation and Rules files are unchanged. Human balance, real-monitor fullscreen appearance, and audible speaker output remain unverified; dummy drivers and an invulnerable campaign pilot do not establish those observations.

Commands from `build/worktrees/ticket-07`:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain GCC -Test -BuildDirectory build/combat-gcc -StageDirectory build/stage-gcc
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain MSVC -Test -BuildDirectory build/combat-msvc -StageDirectory build/stage-msvc
```

## Overhaul ticket 11: source and lecturer walkthrough

Verified on 2026-10-24 in the real `ticket/11` worktree, based on the integrated combat/audio/menu/save implementation. This ticket changes documentation, not gameplay or production interfaces; it does not add artificial tests for prose.

- The README now provides separate user, player, developer, and lecturer routes. The consolidated developer guide includes verified source/interface ownership, frame/coordinate order, shared rules, collision/save invariants, existing build/test/install commands, resource maintenance and failure behavior. The lecturer guide connects those sources to concrete C++17 examples and an isolated live/controlled presentation script.
- Architecture ownership, data-flow and frame-sequence arrows were checked against Game, ScreenLayout, Run, Draw/VisualResources, Sound and Scores. They show Game owning SDL handles, Draw owning its resource/cache members, screen drawing as Game methods, and Game calling RenderPresent. No remaining diagram is labeled as planned implementation.
- Both documented wrapper commands below configured, built, passed both registered groupings (2/2 each), and installed into fresh separate staging directories. Focused checks reported 61 passes and 131 campaign hostiles; GCC elapsed 193 simulated seconds and MSVC 188. A subsequent application-only CTest command from the guide also passed.
- Merged integration `905b16f`, containing the test-only hull-variable naming cleanup; both application test targets rebuilt and `screen_flow` passed again. MSVC recompilation produced no C4456 shadow warning. The cleanup belongs to ticket 07, not this documentation slice.
- The staged production executable started with the documented isolated absolute `--data-dir` from an unrelated working directory, stayed running, and produced no startup error with dummy drivers. This confirms CLI/resource startup, not a live physical-device presentation. The existing harness exercised the walkthrough's movement/dash/pause/results/save/reopen/final-wave behaviors and generated all named captures.
- Inspected all eight current published screen previews, plus newly generated dash/overdrive, half-boss, surviving-supports, whole-wave-victory and partial-hull frames. Text/controls, distinct ship/shot artwork, HUD capacity, boss durability and documented result flow agree with the walkthrough. Existing previews were already current, so no image bytes changed in this ticket.
- Audited 125 local documentation targets, including source pointers and heading fragments; all resolved. The installed user/player/save/combat/audio/credit guides had 17 resolving local targets. All ten reference assets matched manifest byte lengths and SHA-256; all 752 pinned dependency source files passed their verifier. Installed resource and notice bytes matched source, and staging contained the four expected SDL DLLs.
- Preserved historical validation sections beneath a clear baseline label; the old procedural game's no-asset/one-SDL observations are not current-runtime claims. Current limitations remain real speakers, real-monitor fullscreen/resize/input, human balance, forthcoming exact asset permission terms/source, and ticket 12's final clean archive/extraction acceptance. No course approval is claimed.

Commands run from the ticket worktree:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain GCC -Test -BuildDirectory build/lecturer-gcc -StageDirectory build/lecturer-stage
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain MSVC -Test -BuildDirectory build/lecturer-msvc -StageDirectory build/lecturer-stage-msvc
ctest --test-dir build/lecturer-gcc -R screen_flow --output-on-failure
python third_party/tools/verify_sources.py
```

CTest was invoked through Visual Studio's discovered CMake-tools path where it was not on PATH. The wrapper exercises the same configure/build/CTest/install operations documented for direct CMake. Final archive commands remain with the portable-release ticket; this walkthrough does not invent an unavailable packaging command.

## Overhaul ticket 12: portable release candidate

Verified on 2026-10-24 in the real `ticket/12` worktree based on integration `82edb02`. This section records the initial candidate acceptance; subsequent final review/source closure is recorded below. Generated evidence remains in the original workspace's `build/release-verification`, outside temporary ticket worktrees, and private candidate archives/checksum sidecars remain under its `releases/candidate12-*` directories. No public upload is performed. The prior original-root dist and historical archive remain preserved until validated replacement.

- Fresh GCC 15.2.0/Ninja and MSVC 19.50.35725.0/Visual Studio 2026 configured and built from the same source, then passed both CTest groupings (2/2 each). Focused checks reported 61 passes; the real collision-driven campaign defeated 131 hostiles in 193 GCC/188 MSVC simulated seconds. CMake's `starfall_package` target then installed declared runtime files into fresh stages, generated exact manifests and archived them. Reversible packaging/prose changes received real artifact acceptance rather than artificial unit tests.
- Each portable ZIP contains exactly 53 files including its own manifest: Starfall, SDL2/SDL2_image/SDL2_ttf/SDL2_mixer, ten unchanged reference assets and notice, launcher/help, user/player/save/combat/audio guidance and previews, credits/resource evidence, and library notices. No test executables, source fixture/demo scores, personal profiles, temporary files, compiler DLLs or generated build outputs are present. ZIP CRC, safe paths, duplicate/case collisions, exact manifest/archive file equality, sizes, every SHA-256 and the external ZIP checksum passed.
- Parsed final PE headers/import descriptors for all five binaries in both packages. All are AMD64; transitive non-system dependencies are exactly the four shipped SDL DLLs. GCC imports Windows msvcrt plus Windows system libraries; MSVC imports only Windows system libraries and the shipped SDL set. FreeType and the selected PNG/FLAC codecs introduce no DLLs. All 752 unchanged vendor files still match their pinned upstream Git blobs. All ten shipped asset lengths/hashes match the resource manifest; all six additional FreeType BDF/PCF/fthash/ft-hb notice files match unchanged upstream bytes.
- Extracted GCC to `build/release-verification/Extracted Starfall` and MSVC to `Extracted Starfall MSVC`. Production startup remained running from separate unrelated folders with absolute isolated save paths containing spaces. The portable launcher's actual startup and quoted save-path forwarding also worked from the unrelated folder; `--help` and `--check` succeeded. Removing each DLL, ship atlas or clip produced actionable complete-package recovery text. These presence checks supplement real decoding, rather than substituting for it.
- Copied the corresponding application test helper only into each disposable extraction and ran `RunApplicationChecks.cmake` with that extraction as the runtime. Full navigation/input/combat/menu/legacy-score/save-retry and audio scenarios passed against the shipped DLLs/resources. Missing/corrupt PNG/font, missing/corrupt shooting FLAC, missing device, stable old-format scores, explicit once-only Save and reopening all passed. Helpers were then removed, resources restored, and every extracted file rechecked against the pristine manifest. QA scores stayed outside the portable tree; the actual user's default leaderboard was never read or modified. The unchanged production default route remains SDL_GetPrefPath; explicit save paths were exercised here.
- Inspected software-rendered release captures for welcome, combat, pause, flight manual, defeat/name, populated ten-row leaderboard, victory and Dreadnought. Also inspected actual 800x500 leaderboard/name frames, save-retry text, missing-audio/device footers, dash/overdrive, half-boss and surviving-supports. Text/control alignment and clipping, sprite/background presence, hull slots and boss/HUD indicators were readable. Captures and isolated production startup cover the lecturer script's controlled demonstration. Dummy resize/fullscreen events verify coordinate routing, not a physical monitor. Actual speaker output, physical display/fullscreen/resize appearance/input, human difficulty balance, exact forthcoming asset permission terms/source and course approval remain unverified.
- `ArchiveSource.ps1` created a source-only ZIP and complete private Git bundle with separate checksum sidecars. The source list exactly matched committed inputs plus SOURCE-COMMIT.txt (927 files at the initial candidate), without dist/build/releases/Git metadata. Bundle verification, local clone, candidate checkout and full fsck passed. A fresh GCC build/test/install from the extracted source path containing spaces passed both groupings. Its initial failure exposed windres escaping include spaces incorrectly; the local RC preprocessing rule fixed it without modifying pinned source. Logs retain both the red restoration and green restored rebuild.

Actual first-candidate commands were run from the ticket worktree with absolute original-root evidence/artifact paths:

```powershell
$verificationRoot = 'C:/Users/Admin/Desktop/spaceshooter-SDL2/build/release-verification'
$artifactRoot = 'C:/Users/Admin/Desktop/spaceshooter-SDL2/releases'
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain GCC -Package -BuildDirectory "$verificationRoot/gcc" -StageDirectory "$verificationRoot/stage-gcc" -PackageOutputDirectory "$artifactRoot/candidate12-gcc"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/Build.ps1 -Toolchain MSVC -Package -BuildDirectory "$verificationRoot/msvc" -StageDirectory "$verificationRoot/stage-msvc" -PackageOutputDirectory "$artifactRoot/candidate12-msvc"
python scripts/VerifyPackage.py "$artifactRoot/candidate12-gcc/Starfall-Windows-x64.zip"
python scripts/VerifyPackage.py "$artifactRoot/candidate12-msvc/Starfall-Windows-x64.zip"
python third_party/tools/verify_sources.py
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/ArchiveSource.ps1 -OutputDirectory "$artifactRoot/candidate12-source-fixed"
```

Build/package logs are in `logs`; full compiler outputs and CTest logs remain in `gcc`, `msvc`, and `source-fixed-build`; extracted coordinator output/captures are in `gcc-extracted-flow` and `msvc-extracted-flow`; converted inspected PNGs are in `captures`. `gcc-verification.json`, `msvc-verification.json` and source/history reports retain exact hashes/imports/results. Later candidate/review refreshes use new stages/output folders and record their own provenance/sidecars; no archive hash is embedded into tracked documents that would alter that same archive. Reproduction commands and extraction acceptance steps are in the [developer guide](DEVELOPER.md#portable-zip-and-sourcehistory-handoff). Final review cannot be inferred from the existence of these initial archives.

## Overhaul final review and source closure

Independent standards and spec reviews were completed against the integration implementation and preserved source baseline. Standards reported no hard violations and one design judgment: the footer selected audio recovery advice by matching an English diagnostic prefix. Fix `98957cd9327b357aec5799ede710cd9c3f1f8182` supplies `AudioWarningKind` from actual failure sites, and the UI switches on the category. Real missing/corrupt clip, missing-device, successful-startup and restored-resource application scenarios assert those categories; no new test seam or private mutation hook was introduced.

GCC and MSVC each passed both normal CTest groupings after the fix (2/2). Retained logs are `build/release-verification/review-fixes/gcc-final.log` and `msvc-final.log`; the original red and green logs are also retained. The integration merge rebuilt the application and passed targeted `screen_flow` (1/1). Resource/device/restored footer captures were inspected and fit their controls. Independent follow-up then cleared the original standards judgment with **zero unresolved findings and zero hard violations**; spec found **no new findings**. The [full review record](../.scratch/starfall-overhaul/code-review.md) distinguishes the judgment, correction and independent clearance.

The reviewed integration tip before this documentation/status closure is `89426cc9fb66790f6a9fc5b18d8ea0515bb5e267`. All twelve tickets are resolved and the spec implementation is complete. This closure changes only documentation/status; no executable source or tests changed, so it does not repeat runtime suites or create artificial prose tests. Final portable/source/history artifacts are refreshed only after the closure and coordination log are integrated. Their generated manifests, external checksum sidecars and retained verification reports will identify the resulting exact integration commit; canonical promotion and temporary-worktree cleanup remain delivery operations at this stage and are not yet claimed.

The implemented resource route uses the user's explicit authorization for local/private packages; exact permission terms/source remain forthcoming and no general asset license is invented. Physical speaker output, physical monitor/fullscreen/resize appearance/input, human difficulty balance and course approval remain unverified. Dummy decoding/mixing, controlled software captures and automated campaign progression provide the narrower evidence described above. The user's actual default leaderboard remains untouched throughout verification.
