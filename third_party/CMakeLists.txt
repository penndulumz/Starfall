# Build only the resource formats used by Starfall from pinned official sources.
# Upstream sources remain unchanged; local choices are confined to this wrapper.
enable_language(C)

if(MINGW)
    enable_language(RC)
    # windres escapes include-path spaces as backslashes when invoking GCC,
    # which Windows command parsing splits. Preprocess with CMake's quoted C
    # command first; windres then receives a self-contained resource file.
    set(CMAKE_RC_COMPILE_OBJECT
        "<CMAKE_C_COMPILER> -E -x c -DRC_INVOKED <DEFINES> <INCLUDES> <SOURCE> -o <OBJECT>.preprocessed.rc"
        "<CMAKE_RC_COMPILER> -O coff <FLAGS> <OBJECT>.preprocessed.rc <OBJECT>")
endif()

if(NOT TARGET SDL2::SDL2)
    if(NOT TARGET starfall_sdl)
        message(FATAL_ERROR "Configure Starfall's SDL2 target before its companion libraries.")
    endif()
    add_library(SDL2::SDL2 ALIAS starfall_sdl)
endif()

# The project ships ordinary ASCII text and PNG/FLAC resources. Avoid optional
# codec DLLs and shaping libraries that the supported presentation does not use.
set(BUILD_SHARED_LIBS ON)
set(SDL2IMAGE_INSTALL OFF CACHE BOOL "Starfall installs only its runtime package" FORCE)
set(SDL2IMAGE_SAMPLES OFF CACHE BOOL "No dependency samples" FORCE)
set(SDL2IMAGE_TESTS OFF CACHE BOOL "No dependency tests" FORCE)
set(SDL2IMAGE_VENDORED OFF CACHE BOOL "Use bundled stb PNG backend" FORCE)
set(SDL2IMAGE_BACKEND_STB ON CACHE BOOL "Decode PNG with bundled stb_image" FORCE)
set(SDL2IMAGE_BACKEND_WIC OFF CACHE BOOL "One reproducible PNG backend" FORCE)
foreach(format AVIF BMP GIF JPG JXL LBM PCX PNM QOI SVG TGA TIF WEBP XCF XPM XV)
    set(SDL2IMAGE_${format} OFF CACHE BOOL "Unused image format" FORCE)
endforeach()
set(SDL2IMAGE_PNG_SAVE OFF CACHE BOOL "Runtime only loads PNG resources" FORCE)
add_subdirectory(SDL_image/upstream EXCLUDE_FROM_ALL)

set(SDL2MIXER_INSTALL OFF CACHE BOOL "Starfall installs only its runtime package" FORCE)
set(SDL2MIXER_SAMPLES OFF CACHE BOOL "No dependency samples" FORCE)
set(SDL2MIXER_VENDORED OFF CACHE BOOL "Use built-in WAV and dr_flac" FORCE)
set(SDL2MIXER_FLAC ON CACHE BOOL "Reference shooting clip uses FLAC" FORCE)
set(SDL2MIXER_FLAC_DRFLAC ON CACHE BOOL "Decode FLAC without an external codec DLL" FORCE)
set(SDL2MIXER_FLAC_LIBFLAC OFF CACHE BOOL "Do not require libFLAC" FORCE)
set(SDL2MIXER_VORBIS OFF CACHE STRING "No OGG resources" FORCE)
foreach(format CMD GME MOD MP3 MIDI OPUS WAVPACK)
    set(SDL2MIXER_${format} OFF CACHE BOOL "Unused audio format" FORCE)
endforeach()
add_subdirectory(SDL_mixer/upstream EXCLUDE_FROM_ALL)

# Limit FreeType's install/build options to its own scope. The resulting static
# library is linked into SDL2_ttf, so it adds no runtime DLL to the portable game.
function(starfall_add_freetype)
    set(BUILD_SHARED_LIBS OFF)
    set(SKIP_INSTALL_ALL ON)
    foreach(dependency ZLIB BZIP2 PNG HARFBUZZ BROTLI)
        set(FT_DISABLE_${dependency} ON CACHE BOOL "No optional FreeType dependencies" FORCE)
    endforeach()
    add_subdirectory(freetype/upstream EXCLUDE_FROM_ALL)
endfunction()
starfall_add_freetype()
if(NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype ALIAS freetype)
endif()

# Let upstream SDL2_ttf's FindFreetype use the already-built official source
# target instead of discovering an unrelated system installation.
set(FREETYPE_LIBRARY freetype)
set(FREETYPE_INCLUDE_DIR_ft2build "${CMAKE_CURRENT_SOURCE_DIR}/freetype/upstream/include")
set(FREETYPE_INCLUDE_DIR_freetype2 "${CMAKE_CURRENT_SOURCE_DIR}/freetype/upstream/include")
set(SDL2TTF_INSTALL OFF CACHE BOOL "Starfall installs only its runtime package" FORCE)
set(SDL2TTF_SAMPLES OFF CACHE BOOL "No dependency samples" FORCE)
set(SDL2TTF_VENDORED OFF CACHE BOOL "Use the pinned FreeType target" FORCE)
set(SDL2TTF_HARFBUZZ OFF CACHE BOOL "Supported text needs no shaping dependency" FORCE)
add_subdirectory(SDL_ttf/upstream EXCLUDE_FROM_ALL)

if(MINGW)
    foreach(companion SDL2_image SDL2_mixer SDL2_ttf)
        target_link_options(${companion} PRIVATE -static-libgcc)
    endforeach()
endif()

# Callers use the upstream target names. Stage these three DLLs together with
# SDL2.dll and the declared resources; FreeType is already linked statically.
set(STARFALL_RESOURCE_TARGETS SDL2_image SDL2_mixer SDL2_ttf PARENT_SCOPE)
