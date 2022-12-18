cmake_minimum_required(VERSION 3.8)
project(CoreTest C CXX)

file(GLOB_RECURSE SOURCE_FILES *.cpp)

include_directories(${Core_INCLUDE_DIRS})

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} GTest::gtest_main Core)

add_test(${PROJECT_NAME} ${PROJECT_NAME})