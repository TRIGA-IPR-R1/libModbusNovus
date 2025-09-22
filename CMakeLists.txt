cmake_minimum_required(VERSION 3.12)

project(
  libModbusNovus
  VERSION 0.0.0
  DESCRIPTION "Comunicate with the NOVUS devices using MODBUS-RTU"
  HOMEPAGE_URL "https://github.com/TRIGA-IPR-R1/libModbusNovus"
  LANGUAGES CXX)

find_package(PkgConfig REQUIRED)

pkg_search_module(Modbus REQUIRED IMPORTED_TARGET libmodbus)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES
    ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})

set(libModbusNovus_SRC src/libModbusNovus.cpp)

add_library(modbusNovus STATIC ${libModbusNovus_SRC})
add_library(modbusNovus::modbusNovus ALIAS modbusNovus)

target_include_directories(modbusNovus
                           PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(modbusNovus PUBLIC PkgConfig::Modbus)

add_executable(modbusNovus-test src/test.cpp)
target_link_libraries(modbusNovus-test PRIVATE modbusNovus)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libModbusNovus/)
install(
  TARGETS modbusNovus
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

install(TARGETS modbusNovus-test
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

set(libModbusNovus_INCLUDE_DIRS
    ${CMAKE_INSTALL_PREFIX}/include/libModbusNovus)
set(libModbusNovus_LIBRARIES
    ${CMAKE_INSTALL_PREFIX}/lib/libModbusNovus.a)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/LibModbusNovusConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/resources/LibModbusNovusConfig.cmake
  @ONLY)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/resources/LibModbusNovusConfig.cmake
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/libModbusNovus)
