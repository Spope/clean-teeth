# Themes for the device UI, shared by the simulator and the ESP32 build.
#
#   brushy_theme_sources(<out-var> <out-dir> <python> <theme-id>...)
#
# Generates C into <out-dir> from what tools/export_assets.sh exported for each theme
# (assets/themes/<id>.json and assets/png/<id>_*.png), again whenever they change:
# the LVGL images, themes.c (each theme's brushy_theme_t, in the order given: a long press
# cycles through them) and assets.h. Sets <out-var> to these files.
get_filename_component(BRUSHY_REPO_ROOT ${CMAKE_CURRENT_LIST_DIR}/../.. ABSOLUTE)

function(brushy_theme_sources out_var out_dir python)
    if(NOT ARGN)
        message(FATAL_ERROR "No theme to build: export one first (tools/export_assets.sh)")
    endif()
    set(assets ${BRUSHY_REPO_ROOT}/assets)
    set(srcs ${out_dir}/themes.c ${out_dir}/assets.h)
    set(deps ${BRUSHY_REPO_ROOT}/tools/png2lvgl.py)
    foreach(id ${ARGN})
        if(NOT EXISTS ${assets}/themes/${id}.json)
            message(FATAL_ERROR "Theme '${id}' not found: no assets/themes/${id}.json. "
                "Export it from prototype/themes/${id}.js first: tools/export_assets.sh ${id}")
        endif()
        file(GLOB pngs CONFIGURE_DEPENDS ${assets}/png/${id}_*.png)
        list(APPEND deps ${assets}/themes/${id}.json ${pngs})
        foreach(png ${pngs})
            get_filename_component(stem ${png} NAME_WE)
            list(APPEND srcs ${out_dir}/img_${stem}.c)
        endforeach()
    endforeach()
    string(REPLACE ";" ", " names "${ARGN}")
    add_custom_command(
        OUTPUT ${srcs}
        COMMAND ${python} ${BRUSHY_REPO_ROOT}/tools/png2lvgl.py ${out_dir} ${assets} ${ARGN}
        DEPENDS ${deps}
        COMMENT "Converting themes to C: ${names}"
        VERBATIM)
    set(${out_var} ${srcs} PARENT_SCOPE)
endfunction()

# The id of each theme exported to assets/themes/, in alphabetical order
function(brushy_all_themes out_var)
    file(GLOB jsons CONFIGURE_DEPENDS ${BRUSHY_REPO_ROOT}/assets/themes/*.json)
    set(ids)
    foreach(json ${jsons})
        get_filename_component(id ${json} NAME_WE)
        list(APPEND ids ${id})
    endforeach()
    list(SORT ids)
    set(${out_var} ${ids} PARENT_SCOPE)
endfunction()
