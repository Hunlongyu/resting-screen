cmake_minimum_required(VERSION 3.28)

project(Screen LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(SOURCES
  "src/include.h"
  "src/main.cpp"
  "src/assets/resource.rc"
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE
  "include"
)

target_precompile_headers(
  ${PROJECT_NAME}
  PRIVATE
  "include/pch.h"
)