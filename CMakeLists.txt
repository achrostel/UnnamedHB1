# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_TOOLCHAIN_FILE $ENV{PSN00BSDK_LIBS}/cmake/sdk.cmake)
set (CMAKE_C_FLAGS "-O2")
# set(CMAKE_VERBOSE_MAKEFILE ON)

project(
	GAME
	LANGUAGES    C ASM
	VERSION      0.1.0
	DESCRIPTION  "HOMEBREW for PSX written with PSN00bSDK"
)

set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(FRAMEWORK_DIR ${PROJECT_SOURCE_DIR}/framework)
file(GLOB _sources ${SOURCE_DIR}/*.c ${FRAMEWORK_DIR}/*.c)
include_directories(${FRAMEWORK_DIR})

psn00bsdk_add_executable(GAME GPREL ${_sources})

include(${PROJECT_SOURCE_DIR}/assets/CMakeLists.txt)

# psn00bsdk_add_cd_image(
# 	iso      # Target name
# 	AHomebrew1 # Output file name (= template.bin + template.cue)
# 	iso.xml  # Path to config file
# 	DEPENDS GAME system.cnf
# )

install(
	FILES
		# ${PROJECT_BINARY_DIR}/cdxa.bin
		# ${PROJECT_BINARY_DIR}/cdxa.cue
		${PROJECT_BINARY_DIR}/GAME.exe
	TYPE BIN
)