add_library(KeytopSysmon STATIC
    core/sysmon/types.h
    core/sysmon/parsers.h
    core/sysmon/parsers.cpp
    core/sysmon/cache_helpers.h
    core/sysmon/cache_helpers.cpp
    core/sysmon/collector.h
    core/sysmon/collector_system.cpp
    core/sysmon/collector_devices.cpp
    core/sysmon/collector_processes.cpp
    core/sysmon/gpu/gpu_provider.h
    core/sysmon/gpu/gpu_manager.h
    core/sysmon/gpu/gpu_manager.cpp
    core/sysmon/gpu/drm_helpers.h
    core/sysmon/gpu/drm_helpers.cpp
    core/sysmon/gpu/nvidia_nvml_provider.cpp
    core/sysmon/sampler.h
    core/sysmon/sampler.cpp
    core/sysmon/serialization.h
    core/sysmon/serialization.cpp
)
set_target_properties(KeytopSysmon PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(KeytopSysmon PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core")
target_link_libraries(KeytopSysmon PUBLIC
    Qt6::Core
    ${CMAKE_DL_LIBS}
)

add_executable(keytop
    main.cpp
    commands/command_result.h
    commands/reload_command.h
    commands/reload_command.cpp
    commands/sysmon_command.h
    commands/sysmon_command.cpp
    commands/top_command.h
    commands/top_command.cpp
    tui/top_tui.h
    tui/top_tui_helpers.h
    tui/top_tui_helpers.cpp
    tui/top_tui.cpp
    config/keytop_config.h
    config/keytop_config.cpp
)
set_target_properties(keytop PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
target_include_directories(keytop PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/core"
    "${CMAKE_CURRENT_SOURCE_DIR}/commands"
    "${CMAKE_CURRENT_SOURCE_DIR}/tui"
)
target_link_libraries(keytop PRIVATE
    KeytopSysmon
    Qt6::Core
    PkgConfig::NCURSESW
)
target_compile_definitions(keytop PRIVATE
    KEYTOP_DEFAULTS_INSTALL_DIR="${KEYTOP_DEFAULTS_INSTALL_DIR}"
    KEYTOP_BUILD_DEFAULTS_DIR="${KEYTOP_BUILD_DEFAULTS_DIR}"
)

install(TARGETS keytop
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
