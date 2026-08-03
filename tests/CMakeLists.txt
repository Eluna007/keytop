qt_add_executable(keytop_sysmon_core_tests sysmon_core_test.cpp)
target_link_libraries(keytop_sysmon_core_tests PRIVATE KeytopSysmon Qt6::Core Qt6::Test)
target_include_directories(keytop_sysmon_core_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src/core")
add_test(NAME keytop_sysmon_core_tests COMMAND keytop_sysmon_core_tests)

qt_add_executable(keytop_tui_helpers_tests top_tui_helpers_test.cpp "${CMAKE_CURRENT_SOURCE_DIR}/../src/tui/top_tui_helpers.cpp")
target_link_libraries(keytop_tui_helpers_tests PRIVATE Qt6::Core Qt6::Test)
target_include_directories(keytop_tui_helpers_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../src"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/tui"
)
add_test(NAME keytop_tui_helpers_tests COMMAND keytop_tui_helpers_tests)

qt_add_executable(keytop_integration_tests keytop_integration_test.cpp)
target_link_libraries(keytop_integration_tests PRIVATE Qt6::Core Qt6::Test)
target_compile_definitions(keytop_integration_tests PRIVATE
    KEYTOP_EXECUTABLE="$<TARGET_FILE:keytop>"
)
add_dependencies(keytop_integration_tests keytop)
add_test(NAME keytop_integration_tests COMMAND keytop_integration_tests)

qt_add_executable(keytop_rapl_helper_tests rapl_helper_client_test.cpp)
target_link_libraries(keytop_rapl_helper_tests PRIVATE
    KeytopRuntime Qt6::Core Qt6::Network Qt6::Test
)
target_include_directories(keytop_rapl_helper_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src/core")
add_test(NAME keytop_rapl_helper_tests COMMAND keytop_rapl_helper_tests)

