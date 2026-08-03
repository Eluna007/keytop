add_library(KeytopRuntime STATIC
    core/runtime/clavis_paths.h
    core/runtime/clavis_paths.cpp
    core/runtime/rapl_helper_client.h
    core/runtime/rapl_helper_client.cpp
)
set_target_properties(KeytopRuntime PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(KeytopRuntime PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core")
target_link_libraries(KeytopRuntime PUBLIC Qt6::Core Qt6::Network)

add_library(KeytopSysmon STATIC
    core/sysmon/types.h
    core/sysmon/parsers.h
    core/sysmon/parsers.cpp
    core/sysmon/collector.h
    core/sysmon/collector_system.cpp
    core/sysmon/collector_devices.cpp
    core/sysmon/collector_processes.cpp
    core/sysmon/sampler.h
    core/sysmon/sampler.cpp
    core/sysmon/serialization.h
    core/sysmon/serialization.cpp
)
set_target_properties(KeytopSysmon PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(KeytopSysmon PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core")
target_link_libraries(KeytopSysmon PUBLIC
    KeytopRuntime
    Qt6::Core
    Qt6::Network
)

add_executable(keytop
    main.cpp
    commands/command_result.h
    commands/sysmon_command.h
    commands/sysmon_command.cpp
    commands/top_command.h
    commands/top_command.cpp
    tui/top_tui.h
    tui/top_tui_helpers.h
    tui/top_tui_helpers.cpp
    tui/top_tui.cpp
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
    KeytopRuntime
    KeytopSysmon
    Qt6::Core
    Qt6::Network
    PkgConfig::NCURSESW
)

add_executable(clavis-rapl-helper helper/rapl/main.cpp)
target_compile_features(clavis-rapl-helper PRIVATE cxx_std_17)
target_link_libraries(clavis-rapl-helper PRIVATE Qt6::Core)
set_target_properties(clavis-rapl-helper PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

install(TARGETS keytop
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
install(TARGETS clavis-rapl-helper
    RUNTIME DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}/keytop"
)
