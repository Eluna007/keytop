cmake_minimum_required(VERSION 3.21)
project(Keytop VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(PkgConfig REQUIRED)
pkg_check_modules(NCURSESW IMPORTED_TARGET ncursesw REQUIRED)

set(KEYTOP_DEFAULTS_INSTALL_DIR
    "${CMAKE_INSTALL_FULL_DATADIR}/keytop/defaults")
set(KEYTOP_BUILD_DEFAULTS_DIR "${CMAKE_BINARY_DIR}/keytop-defaults")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/defaults/"
     DESTINATION "${KEYTOP_BUILD_DEFAULTS_DIR}")

add_subdirectory(src)

if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/keytop"
)
install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/defaults/config.conf"
    "${CMAKE_CURRENT_SOURCE_DIR}/defaults/colors.conf"
    "${CMAKE_CURRENT_SOURCE_DIR}/defaults/matugen.conf"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/keytop/defaults"
)
