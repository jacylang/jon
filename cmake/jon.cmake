if (CMAKE_VERSION VERSION_GREATER 3.10 OR CMAKE_VERSION VERSION_EQUAL 3.10)
    include_guard()
endif()

include(CMakePackageConfigHelpers)

function(jon_install_logic)
    set(targets_export_name jon-targets)
    set(PackagingTemplatesDir "${PROJECT_SOURCE_DIR}/packaging")

    write_basic_package_version_file(
        ${version_config}
        VERSION ${CMAKE_PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
        ${OPTIONAL_ARCH_INDEPENDENT}
    )
    configure_package_config_file(
        ${PackagingTemplatesDir}/jon-config.cmake.in
        ${project_config}
        INSTALL_DESTINATION ${JON_CMAKE_DIR})

    export(TARGETS jon NAMESPACE jon::
        FILE ${PROJECT_BINARY_DIR}/${targets_export_name}.cmake)

    install(
        FILES ${project_config} ${version_config}
        DESTINATION ${JON_CMAKE_DIR})
    install(EXPORT ${targets_export_name} DESTINATION ${JON_CMAKE_DIR}
        NAMESPACE jon::)
    
    install(TARGETS jon EXPORT ${targets_export_name} DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(FILES ${PROJECT_SOURCE_DIR}/include/jon.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    set(PKG_CONFIG_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc")
    configure_file("${PackagingTemplatesDir}/pkgconfig.pc.in" "${PKG_CONFIG_FILE_NAME}" @ONLY)
    install(FILES "${PKG_CONFIG_FILE_NAME}"
            DESTINATION "${CMAKE_INSTALL_LIBDIR_ARCHIND}/pkgconfig"
    )
endfunction(jon_install_logic)

macro(jon_set_cxx_standard)
    if (JON_CXX_STANDARD)
        set(JON_CXX_STANDARD ${JON_CXX_STANDARD})
    else()
        set(JON_CXX_STANDARD 17)
    endif()

    set(CMAKE_CXX_EXTENSIONS OFF)
endmacro()

if (JON_ENABLE_INSTALL)
    jon_install_logic()
endif()
