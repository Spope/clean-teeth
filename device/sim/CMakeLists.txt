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

# Images: assets/png/*.png -> LVGL C arrays, regenerated when a PNG changes
find_package(Python3 REQUIRED COMPONENTS Interpreter)
file(GLOB ASSET_PNGS CONFIGURE_DEPENDS ${REPO_ROOT}/assets/png/*.png)
set(ASSET_DIR ${CMAKE_BINARY_DIR}/assets)
set(ASSET_SRCS)
foreach(png ${ASSET_PNGS})
    get_filename_component(stem ${png} NAME_WE)
    list(APPEND ASSET_SRCS ${ASSET_DIR}/img_${stem}.c)
endforeach()
add_custom_command(
    OUTPUT ${ASSET_SRCS} ${ASSET_DIR}/assets.h
    COMMAND Python3::Interpreter ${REPO_ROOT}/tools/png2lvgl.py ${ASSET_DIR} ${ASSET_PNGS}
    DEPENDS ${ASSET_PNGS} ${REPO_ROOT}/tools/png2lvgl.py
    COMMENT "Converting PNG assets to LVGL images"
    VERBATIM)

add_executable(brushy_sim
    main.c
    hal_sim.c
    ${UI_DIR}/brushy_ui.c
    ${UI_DIR}/brushy_fx.c
    ${UI_DIR}/brushy_tricks.c
    ${UI_DIR}/brushy_synth.c
    ${UI_DIR}/themes/themes.c
    ${UI_DIR}/themes/minecraft.c
    ${UI_DIR}/themes/dragon.c
    ${ASSET_SRCS}
    ${ASSET_DIR}/assets.h)
target_include_directories(brushy_sim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${UI_DIR} ${ASSET_DIR})
target_link_libraries(brushy_sim PRIVATE lvgl PkgConfig::SDL2 m)
set_source_files_properties(main.c hal_sim.c ${UI_DIR}/brushy_ui.c ${UI_DIR}/brushy_fx.c
    ${UI_DIR}/brushy_tricks.c ${UI_DIR}/brushy_synth.c ${UI_DIR}/themes/themes.c
    ${UI_DIR}/themes/minecraft.c ${UI_DIR}/themes/dragon.c
    PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
