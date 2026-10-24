# SDL2 dependency

Starfall uses SDL2 2.30.10, with its upstream [zlib license](LICENSE.txt).

- Headers: unchanged files from the official [release-2.30.10 include directory](https://github.com/libsdl-org/SDL/tree/release-2.30.10/include). Unused OpenGL, OpenGL ES, EGL, Vulkan, and SDL test headers are omitted.
- Windows build input: `bin/SDL2.dll`, 64-bit, version 2.30.10. Copied unchanged from the existing `C:\Program Files\Crow Translate\bin\SDL2.dll` installation because this environment restricts shell downloads. It depends only on Windows system DLLs. CMake stages this runtime beside the executable; generated `dist` contents are never build prerequisites.
- Runtime size: 2,970,832 bytes.
- Runtime SHA-256: `3be02da904bdeca79b930c481489ce164740b6c722fc1e9dbe77bb10344536d7`.
- `SDL2.def`: all 845 exports read from the unchanged pinned runtime, for generating an MSVC import library for both the game and companion libraries. GNU ld can link directly against the DLL. Expanding this definition does not change the runtime bytes.

The official SDL2 release and alternative development bundles are available from [libsdl-org/SDL release 2.30.10](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.10). A compatible official Windows x64 runtime can replace the included one. Preserve SDL2's license when distributing the game.

The game does not require SDL_image, SDL_ttf, SDL_mixer, or an external asset pack.

Those companion dependencies are planned for later overhaul tickets and have not been added by the build refactor. `STARFALL_BUNDLED_SDL_ROOT` may select another complete copy of this header/definition/runtime layout; the default is this directory. Use an x64 toolchain with the bundled x64 runtime.
