cmake_minimum_required(VERSION 3.20)
project(StarfallResourceEvidence LANGUAGES C CXX)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
get_filename_component(STARFALL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(STARFALL_SDL_DLL "${STARFALL_ROOT}/third_party/SDL2/bin/SDL2.dll")
add_library(starfall_sdl SHARED IMPORTED)
set_target_properties(starfall_sdl PROPERTIES
    IMPORTED_LOCATION "${STARFALL_SDL_DLL}"
    INTERFACE_INCLUDE_DIRECTORIES "${STARFALL_ROOT}/third_party/SDL2/include")
if(MSVC)
    execute_process(COMMAND "${CMAKE_AR}"
        "/def:${STARFALL_ROOT}/third_party/SDL2/SDL2.def"
        "/out:${CMAKE_CURRENT_BINARY_DIR}/SDL2.lib" /machine:x64
        RESULT_VARIABLE import_result OUTPUT_QUIET)
    if(NOT import_result EQUAL 0)
        message(FATAL_ERROR "SDL2 import library generation failed")
    endif()
    set_target_properties(starfall_sdl PROPERTIES IMPORTED_IMPLIB "${CMAKE_CURRENT_BINARY_DIR}/SDL2.lib")
else()
    set_target_properties(starfall_sdl PROPERTIES IMPORTED_IMPLIB "${STARFALL_SDL_DLL}")
endif()
add_subdirectory("${STARFALL_ROOT}/third_party" companions)
add_executable(starfall_resource_probe "${STARFALL_ROOT}/third_party/tools/verify_formats.c")
target_link_libraries(starfall_resource_probe PRIVATE SDL2_image SDL2_ttf SDL2_mixer starfall_sdl)
foreach(runtime SDL2_image SDL2_ttf SDL2_mixer)
    add_custom_command(TARGET starfall_resource_probe POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${runtime}>" "$<TARGET_FILE_DIR:starfall_resource_probe>")
endforeach()
add_custom_command(TARGET starfall_resource_probe POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${STARFALL_SDL_DLL}" "$<TARGET_FILE_DIR:starfall_resource_probe>")
