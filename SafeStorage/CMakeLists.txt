cmake_minimum_required(VERSION 3.8)
project(SafeStorageGUI C CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

file(GLOB_RECURSE SOURCE_FILES *.cpp)
file(GLOB_RECURSE UI_FILES *.ui)
file(GLOB_RECURSE RESOURCE_FILES *.qrc)

add_definitions(${Qt5Widgets_DEFINITIONS})

include_directories(${Qt5Widgets_INCLUDES})
include_directories(${Core_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${UI_FILES} ${RESOURCE_FILES})

target_link_libraries(${PROJECT_NAME} Qt5::Widgets Core)

include(FetchContent)
FetchContent_Declare(
    PVS_CMakeModule
    GIT_REPOSITORY "https://github.com/viva64/pvs-studio-cmake-module.git"
    GIT_TAG        "master" 
)
FetchContent_MakeAvailable(PVS_CMakeModule)

include("${pvs_cmakemodule_SOURCE_DIR}/PVS-Studio.cmake")
pvs_studio_add_target(TARGET SafeStorage.analyze ALL
                      OUTPUT FORMAT tasklist
                      RECURSIVE ANALYZE ${PROJECT_NAME} Core
                      MODE GA:1,2,3 OP:1,2,3 64:1,2
                      LOG target.err
                      ARGS -j 4 -e */zlib/* -e */openssl/* -e */Qt/* -e */ui/*)