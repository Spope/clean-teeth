cmake_minimum_required(VERSION 3.20)
project(brushy_sim C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

get_filename_component(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../.. ABSOLUTE)
set(UI_DIR ${REPO_ROOT}/device/ui)

# SDL2 (on macOS: `brew install sdl2`, which provides SDL2 on top of SDL3).
# In a browser (emcmake, see README "On a web page"): Emscripten's own SDL2.
if(EMSCRIPTEN)
    add_library(sdl2_web INTERFACE)
    target_compile_options(sdl2_web INTERFACE -sUSE_SDL=2)
    target_link_options(sdl2_web INTERFACE -sUSE_SDL=2)
    add_library(PkgConfig::SDL2 ALIAS sdl2_web)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
endif()

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

# In a browser: brushy_sim.js + brushy_sim.wasm, started by web/index.html, which looks like
# the prototype (its device and control panel: device.css). Both are copied next to them.
if(EMSCRIPTEN)
    target_sources(brushy_sim PRIVATE web.c)
    set_source_files_properties(web.c PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
    set_target_properties(brushy_sim PROPERTIES SUFFIX ".js")
    target_link_options(brushy_sim PRIVATE
        -sALLOW_MEMORY_GROWTH  # LVGL's image cache holds MBs
        -sENVIRONMENT=web
        -sEXIT_RUNTIME         # an error (unknown theme...) reaches the page's onExit
        -sEXPORTED_RUNTIME_METHODS=cwrap)
    configure_file(web/index.html ${CMAKE_BINARY_DIR}/index.html COPYONLY)
    configure_file(${REPO_ROOT}/prototype/css/device.css ${CMAKE_BINARY_DIR}/device.css COPYONLY)
endif()
