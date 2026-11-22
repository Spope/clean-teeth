cmake_minimum_required(VERSION 3.20)
project(brushy_sim C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

get_filename_component(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../.. ABSOLUTE)
set(UI_DIR ${REPO_ROOT}/device/ui)

# SDL2 (on macOS: `brew install sdl2`, which provides SDL2 on top of SDL3)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

# LVGL, pinned
set(LV_BUILD_CONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h CACHE PATH "" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_USE_THORVG_INTERNAL OFF CACHE BOOL "" FORCE) # vector graphics not used
include(FetchContent)
FetchContent_Declare(lvgl
    GIT_REPOSITORY https://github.com/lvgl/lvgl.git
    GIT_TAG v9.5.0
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(lvgl)
target_link_libraries(lvgl PUBLIC PkgConfig::SDL2)

# Themes: every one exported to assets/themes/ (see docs/themes.md), converted to C
find_package(Python3 REQUIRED COMPONENTS Interpreter)
include(${UI_DIR}/themes.cmake)
brushy_all_themes(THEMES)
set(ASSET_DIR ${CMAKE_BINARY_DIR}/assets)
brushy_theme_sources(THEME_SRCS ${ASSET_DIR} ${Python3_EXECUTABLE} ${THEMES})

add_executable(brushy_sim
    main.c
    hal_sim.c
    ${UI_DIR}/brushy_ui.c
    ${UI_DIR}/brushy_fx.c
    ${UI_DIR}/brushy_tricks.c
    ${UI_DIR}/brushy_synth.c
    ${THEME_SRCS})
target_include_directories(brushy_sim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${UI_DIR} ${ASSET_DIR})
target_link_libraries(brushy_sim PRIVATE lvgl PkgConfig::SDL2 m)
set_source_files_properties(main.c hal_sim.c ${UI_DIR}/brushy_ui.c ${UI_DIR}/brushy_fx.c
    ${UI_DIR}/brushy_tricks.c ${UI_DIR}/brushy_synth.c ${ASSET_DIR}/themes.c
    PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
