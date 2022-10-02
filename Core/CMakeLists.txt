cmake_minimum_required(VERSION 3.8)
project(Core C CXX)

find_package(OpenSSL REQUIRED COMPONENTS Crypto)
find_package(ZLIB REQUIRED COMPONENTS ZLIB)
find_package(Qt5 REQUIRED COMPONENTS Core)

file(GLOB SOURCE_FILES *.cpp)

add_definitions(${Qt5Core_DEFINITIONS})

include_directories(${Qt5Core_INCLUDES})

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} OpenSSL::Crypto ZLIB::ZLIB Qt5::Core)

set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR} CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

add_subdirectory(Crypto)
add_subdirectory(Stream)
add_subdirectory(Struct)