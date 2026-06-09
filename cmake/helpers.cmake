include_guard()

function(mtgs_copy_assets target_name src dst)
    message(STATUS "Defining custom asset copy target: ${target_name}")

    add_custom_target(${target_name}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${dst}/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${src}"
            "${dst}/assets"
        DEPENDS "${src}"
        COMMENT "Syncing assets from ${src} to ${dst}/assets"
    )
endfunction()