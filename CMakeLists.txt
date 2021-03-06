cmake_minimum_required(VERSION 2.8.5)

project(powercap)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 1)
set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c99 -D_GNU_SOURCE")

include(GNUInstallDirs)

# See powercap-common.h for enumeration
set(POWERCAP_LOG_LEVEL 2 CACHE STRING "Set the log level: 0=DEBUG, 1=INFO, 2=WARN (default), 3=ERROR, 4=OFF")

include_directories(${PROJECT_SOURCE_DIR}/inc)

add_subdirectory(utils)

# Libraries

add_library(powercap src/powercap.c
                     src/powercap-sysfs.c
                     src/powercap-rapl.c
                     src/powercap-rapl-sysfs.c
                     src/powercap-common.c)
target_compile_definitions(powercap PRIVATE POWERCAP_LOG_LEVEL=${POWERCAP_LOG_LEVEL})
if (BUILD_SHARED_LIBS)
  set_target_properties(powercap PROPERTIES VERSION ${PROJECT_VERSION}
                                            SOVERSION ${VERSION_MAJOR})
endif()

# Tests

add_executable(powercap-test test/powercap-test.c)
target_link_libraries(powercap-test powercap)

add_executable(powercap-rapl-test test/powercap-rapl-test.c)
target_link_libraries(powercap-rapl-test powercap)

add_executable(powercap-sysfs-test test/powercap-sysfs-test.c)
target_link_libraries(powercap-sysfs-test powercap)

enable_testing()
macro(add_unit_test target)
  add_test(${target} ${EXECUTABLE_OUTPUT_PATH}/${target})
endmacro(add_unit_test)

add_unit_test(powercap-test)
# Requires a real system with root privileges
# add_unit_test(powercap-rapl-test)
add_unit_test(powercap-sysfs-test)

# pkg-config

set(PKG_CONFIG_EXEC_PREFIX "\${prefix}")
set(PKG_CONFIG_LIBDIR "\${prefix}/${CMAKE_INSTALL_LIBDIR}")
set(PKG_CONFIG_INCLUDEDIR "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")
set(PKG_CONFIG_CFLAGS "-I\${includedir}")

set(PKG_CONFIG_NAME "${PROJECT_NAME}")
set(PKG_CONFIG_DESCRIPTION "C bindings to the Linux Power Capping Framework in sysfs")
set(PKG_CONFIG_LIBS "-L\${libdir} -lpowercap")
set(PKG_CONFIG_LIBS_PRIVATE "")
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig.in
  ${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/powercap.pc
)

# Install

install(TARGETS powercap DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES inc/powercap.h inc/powercap-sysfs.h inc/powercap-rapl.h inc/powercap-rapl-sysfs.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
install(DIRECTORY ${CMAKE_BINARY_DIR}/pkgconfig/ DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# Uninstall

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
  @ONLY
)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
