get_filename_component(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. ABSOLUTE)
set(UI_DIR ${REPO_ROOT}/device/ui)

idf_component_register(
    SRCS
        main.c
        hal_esp32.c
        ${UI_DIR}/brushy_ui.c
        ${UI_DIR}/brushy_fx.c
        ${UI_DIR}/brushy_tricks.c
        ${UI_DIR}/brushy_synth.c
        ${UI_DIR}/themes/themes.c
        ${UI_DIR}/themes/minecraft.c
        ${UI_DIR}/themes/dragon.c
    INCLUDE_DIRS . ${UI_DIR}
    PRIV_REQUIRES nvs_flash esp_psram)

# Images: assets/png/*.png -> LVGL C arrays (kept in flash), regenerated when a PNG changes
idf_build_get_property(python PYTHON)
file(GLOB ASSET_PNGS CONFIGURE_DEPENDS ${REPO_ROOT}/assets/png/*.png)
set(ASSET_DIR ${CMAKE_CURRENT_BINARY_DIR}/assets)
set(ASSET_SRCS)
foreach(png ${ASSET_PNGS})
    get_filename_component(stem ${png} NAME_WE)
    list(APPEND ASSET_SRCS ${ASSET_DIR}/img_${stem}.c)
endforeach()
add_custom_command(
    OUTPUT ${ASSET_SRCS} ${ASSET_DIR}/assets.h
    COMMAND ${python} ${REPO_ROOT}/tools/png2lvgl.py ${ASSET_DIR} ${ASSET_PNGS}
    DEPENDS ${ASSET_PNGS} ${REPO_ROOT}/tools/png2lvgl.py
    COMMENT "Converting PNG assets to LVGL images"
    VERBATIM)
add_custom_target(brushy_assets DEPENDS ${ASSET_SRCS} ${ASSET_DIR}/assets.h)
add_dependencies(${COMPONENT_LIB} brushy_assets)
target_sources(${COMPONENT_LIB} PRIVATE ${ASSET_SRCS})
target_include_directories(${COMPONENT_LIB} PRIVATE ${ASSET_DIR})
