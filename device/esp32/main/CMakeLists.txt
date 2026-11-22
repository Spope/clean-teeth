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
    INCLUDE_DIRS . ${UI_DIR}
    PRIV_REQUIRES nvs_flash esp_psram esp_driver_i2c)

# One theme per device (menuconfig: Brushy Buddy > Theme): only its artwork goes into the
# firmware, as C generated from assets/ (images are kept in flash), again when it changes
include(${UI_DIR}/themes.cmake)
idf_build_get_property(python PYTHON)
set(ASSET_DIR ${CMAKE_CURRENT_BINARY_DIR}/assets)
brushy_theme_sources(THEME_SRCS ${ASSET_DIR} ${python} ${CONFIG_BRUSHY_THEME})
add_custom_target(brushy_assets DEPENDS ${THEME_SRCS})
add_dependencies(${COMPONENT_LIB} brushy_assets)
target_sources(${COMPONENT_LIB} PRIVATE ${THEME_SRCS})
target_include_directories(${COMPONENT_LIB} PRIVATE ${ASSET_DIR})
