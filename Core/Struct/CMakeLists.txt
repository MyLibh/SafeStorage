cmake_minimum_required(VERSION 3.8)

file(GLOB SOURCE_FILES *.cpp)

target_sources(Core PRIVATE ${SOURCE_FILES})