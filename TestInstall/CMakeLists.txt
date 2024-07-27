cmake_minimum_required(VERSION 3.8)
project(TestInstall)

find_library(ROKI_LOWLVL NAMES rokilowlvl)

if (NOT ROKI_LOWLVL)
  message(FATAL_ERROR "Library rokilowlvl is not found")
endif()

add_executable(TestInstall TestInstall.cpp)
target_link_libraries(TestInstall PUBLIC rokilowlvl)