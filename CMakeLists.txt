cmake_minimum_required(VERSION 3.20)
project(Starfall VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
option(STARFALL_USE_SYSTEM_SDL2 "Use an installed SDL2 development package" OFF)

# Dependency inputs live with their headers/notices, never in generated packages.
set(STARFALL_BUNDLED_SDL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL2" CACHE PATH
    "Bundled SDL2 headers, import definitions, and Windows x64 runtime")

function(starfall_compiler_options target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    if(MINGW)
        target_link_options(${target} PRIVATE -static-libgcc -static-libstdc++)
    endif()
endfunction()

add_library(starfall_core STATIC src/simulation/Run.cpp src/persistence/Scores.cpp)
target_include_directories(starfall_core PUBLIC src)
starfall_compiler_options(starfall_core)

if(WIN32 AND NOT STARFALL_USE_SYSTEM_SDL2)
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "The bundled SDL2 runtime is Windows x64. Select an x64 C++ toolchain.")
    endif()
    set(STARFALL_SDL_DLL "${STARFALL_BUNDLED_SDL_ROOT}/bin/SDL2.dll")
    if(NOT EXISTS "${STARFALL_SDL_DLL}")
        message(FATAL_ERROR "Missing bundled SDL2 runtime: ${STARFALL_SDL_DLL}. Restore third_party/SDL2 from the source package, or configure with -DSTARFALL_USE_SYSTEM_SDL2=ON.")
    endif()
    add_library(starfall_sdl SHARED IMPORTED)
    set_target_properties(starfall_sdl PROPERTIES
        IMPORTED_LOCATION "${STARFALL_SDL_DLL}"
        INTERFACE_INCLUDE_DIRECTORIES "${STARFALL_BUNDLED_SDL_ROOT}/include")
    if(MSVC)
        execute_process(COMMAND "${CMAKE_AR}"
            "/def:${STARFALL_BUNDLED_SDL_ROOT}/SDL2.def"
            "/out:${CMAKE_CURRENT_BINARY_DIR}/SDL2.lib" /machine:x64
            RESULT_VARIABLE import_result OUTPUT_QUIET)
        if(NOT import_result EQUAL 0)
            message(FATAL_ERROR "Could not generate the SDL2 import library; use an x64 MSVC toolchain.")
        endif()
        set_target_properties(starfall_sdl PROPERTIES IMPORTED_IMPLIB "${CMAKE_CURRENT_BINARY_DIR}/SDL2.lib")
    else()
        # GNU ld supports importing directly from an existing Windows DLL.
        set_target_properties(starfall_sdl PROPERTIES IMPORTED_IMPLIB "${STARFALL_SDL_DLL}")
    endif()
    set(STARFALL_SDL_TARGET starfall_sdl)
else()
    find_package(SDL2 CONFIG REQUIRED)
    set(STARFALL_SDL_TARGET SDL2::SDL2)
endif()

add_subdirectory(third_party)

add_library(starfall_app STATIC src/app/Game.cpp src/app/Screens.cpp src/app/ScreenLayout.cpp
    src/presentation/Draw.cpp src/presentation/VisualResources.cpp src/platform/Paths.cpp src/audio/Sound.cpp)
target_link_libraries(starfall_app PUBLIC starfall_core ${STARFALL_SDL_TARGET} SDL2_image SDL2_ttf SDL2_mixer)
starfall_compiler_options(starfall_app)

add_executable(Starfall WIN32 src/main.cpp)
target_link_libraries(Starfall PRIVATE starfall_app)
starfall_compiler_options(Starfall)
if(CMAKE_CONFIGURATION_TYPES)
    set(STARFALL_RUNTIME_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
else()
    set(STARFALL_RUNTIME_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
endif()

# One staging owner prevents concurrent game/test links from copying into the
# same resource directory. It also refreshes assets when no C++ source changed.
add_custom_target(starfall_resources ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${STARFALL_RUNTIME_DIRECTORY}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SDL2_image>" "${STARFALL_RUNTIME_DIRECTORY}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SDL2_ttf>" "${STARFALL_RUNTIME_DIRECTORY}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SDL2_mixer>" "${STARFALL_RUNTIME_DIRECTORY}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/resources/reference"
        "${STARFALL_RUNTIME_DIRECTORY}/resources/reference"
    DEPENDS SDL2_image SDL2_ttf SDL2_mixer
    VERBATIM)
if(WIN32 AND NOT STARFALL_USE_SYSTEM_SDL2)
    add_custom_command(TARGET starfall_resources POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${STARFALL_SDL_DLL}" "${STARFALL_RUNTIME_DIRECTORY}")
endif()
add_dependencies(Starfall starfall_resources)

include(CTest)
if(BUILD_TESTING)
    add_executable(starfall_core_tests tests/core_tests.cpp)
    target_link_libraries(starfall_core_tests PRIVATE starfall_core)
    starfall_compiler_options(starfall_core_tests)
    add_test(NAME gameplay_and_scores COMMAND starfall_core_tests "${CMAKE_CURRENT_BINARY_DIR}/qa"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/leaderboard-v1.txt")
    add_executable(starfall_app_tests tests/application_tests.cpp)
    target_link_libraries(starfall_app_tests PRIVATE starfall_app)
    starfall_compiler_options(starfall_app_tests)
    add_dependencies(starfall_app_tests starfall_resources)
    add_test(NAME screen_flow COMMAND ${CMAKE_COMMAND}
        "-DAPP_TEST=$<TARGET_FILE:starfall_app_tests>"
        "-DRUNTIME_DIR=$<TARGET_FILE_DIR:Starfall>"
        "-DQA_DIR=${CMAKE_CURRENT_BINARY_DIR}/qa/navigation"
        "-DLEGACY_FIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/leaderboard-v1.txt"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/RunApplicationChecks.cmake")
    set_tests_properties(screen_flow PROPERTIES ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy")
endif()

install(TARGETS Starfall RUNTIME DESTINATION .)
if(WIN32 AND NOT STARFALL_USE_SYSTEM_SDL2)
    install(FILES "${STARFALL_SDL_DLL}" DESTINATION .)
endif()
install(FILES "${STARFALL_BUNDLED_SDL_ROOT}/LICENSE.txt" DESTINATION . RENAME SDL2-LICENSE.txt)
install(FILES LICENSE DESTINATION . RENAME LICENSE.txt)
install(FILES docs/PLAY.txt DESTINATION . RENAME READ-ME-FIRST.txt)
install(TARGETS ${STARFALL_RESOURCE_TARGETS} RUNTIME DESTINATION . LIBRARY DESTINATION .)
install(DIRECTORY resources DESTINATION .)
install(FILES third_party/RESOURCE-MANIFEST.md DESTINATION third_party)
install(FILES third_party/SDL2/README.md third_party/SDL2/LICENSE.txt DESTINATION third_party/SDL2)
install(FILES docs/resources-evidence.md DESTINATION docs)
install(FILES docs/CREDITS.md DESTINATION docs)
install(FILES docs/AUDIO.md docs/USER.md docs/PLAYER.md docs/SAVES.md docs/COMBAT.md DESTINATION docs)
install(DIRECTORY docs/screenshots DESTINATION docs)
install(FILES third_party/SDL_image/upstream/LICENSE.txt DESTINATION licenses RENAME SDL2_image-LICENSE.txt)
install(FILES third_party/SDL_mixer/upstream/LICENSE.txt DESTINATION licenses RENAME SDL2_mixer-LICENSE.txt)
install(FILES third_party/SDL_ttf/upstream/LICENSE.txt DESTINATION licenses RENAME SDL2_ttf-LICENSE.txt)
install(FILES third_party/freetype/upstream/LICENSE.TXT DESTINATION licenses RENAME FreeType-LICENSE.txt)
install(FILES third_party/freetype/upstream/docs/FTL.TXT DESTINATION licenses RENAME FreeType-FTL.txt)
install(FILES third_party/freetype/upstream/src/gzip/zlib.h DESTINATION licenses RENAME FreeType-zlib-source-and-license.h)
install(FILES third_party/SDL_mixer/upstream/src/codecs/dr_libs/LICENSE DESTINATION licenses RENAME dr_libs-LICENSE.txt)
install(FILES third_party/SDL_image/upstream/src/stb_image.h DESTINATION licenses RENAME stb_image-source-and-license.h)
install(FILES scripts/PortablePlay.bat DESTINATION . RENAME PLAY.bat)
install(FILES third_party/freetype/upstream/src/bdf/README DESTINATION licenses RENAME FreeType-BDF-README.txt)
install(FILES third_party/freetype/upstream/src/pcf/README DESTINATION licenses RENAME FreeType-PCF-README.txt)
install(FILES third_party/freetype/upstream/src/base/fthash.c DESTINATION licenses RENAME FreeType-fthash-source.c)
install(FILES third_party/freetype/upstream/include/freetype/internal/fthash.h DESTINATION licenses RENAME FreeType-fthash-header.h)
install(FILES third_party/freetype/upstream/src/autofit/ft-hb.c DESTINATION licenses RENAME FreeType-ft-hb-source.c)
install(FILES third_party/freetype/upstream/src/autofit/ft-hb.h DESTINATION licenses RENAME FreeType-ft-hb-header.h)

# Package the same install rules used by the beginner wrapper. A fresh stage is
# mandatory, so an old executable or QA profile cannot enter a new archive.
set(STARFALL_PACKAGE_STAGE "${CMAKE_CURRENT_BINARY_DIR}/portable" CACHE PATH "Fresh portable package staging directory")
set(STARFALL_PACKAGE_OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/releases" CACHE PATH "Portable ZIP output directory")
find_package(Git QUIET)
configure_file(scripts/Package.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/StarfallPackage.cmake" @ONLY)
add_custom_target(starfall_package
    COMMAND ${CMAKE_COMMAND} "-DSTARFALL_PACKAGE_CONFIG=$<CONFIG>" -P "${CMAKE_CURRENT_BINARY_DIR}/StarfallPackage.cmake"
    DEPENDS Starfall
    USES_TERMINAL VERBATIM)
if(BUILD_TESTING)
    add_dependencies(starfall_package starfall_core_tests starfall_app_tests)
endif()
