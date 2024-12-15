cmake_minimum_required(VERSION 3.14...3.22)

project(PineDBStandalone LANGUAGES CXX)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

# CPMAddPackage( GITHUB_REPOSITORY jarro2783/cxxopts VERSION 3.0.0 OPTIONS "CXXOPTS_BUILD_EXAMPLES
# NO" "CXXOPTS_BUILD_TESTS NO" "CXXOPTS_ENABLE_INSTALL YES" )

CPMAddPackage(NAME PineDB SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

# ---- Create standalone executable ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

add_executable(${PROJECT_NAME} ${sources})

if(MSVC)
  add_compile_options(/W4)
  target_compile_options(${PROJECT_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 OUTPUT_NAME "PineDB")

target_link_libraries(${PROJECT_NAME} PineDB::PineDB fmt::fmt spdlog::spdlog)
