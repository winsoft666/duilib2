###############################################################################
# Copyright (C) 2018 - 2020, winsoft666, <winsoft666@outlook.com>.
#
# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
# EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
#
# Expect bugs
#
# Please use and enjoy. Please let me know of any bugs/improvements
# that you have found/implemented and I will fix/incorporate them into this
# file.
###############################################################################

cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake;${CMAKE_MODULE_PATH};")
project(duilib2 CXX)

option(DUILIB2_BUILD_SHARED_LIBS "Build shared libraries" ON)
option(DUILIB2_BUILD_TESTS "Build demo project" ON)
option(DUILIB2_USE_STATIC_CRT "Set to ON to build with static CRT on Windows (/MT)." OFF)
option(UILIB_WITH_CEF "Include CEF Control." OFF)

set(DEBUG_LIBNAME_SUFFIX "-d" 
	CACHE STRING "Optional suffix to append to the library name for a debug build")
mark_as_advanced(DEBUG_LIBNAME_SUFFIX)

set(CEF_BINARY_VER "1.0"
	CACHE STRING "https://github.com/winsoft666/cef_binary tag version")
mark_as_advanced(CEF_BINARY_VER)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Unicode Support
add_definitions(-DUNICODE -D_UNICODE)
if(UILIB_WITH_CEF)
	add_definitions(-DUILIB_WITH_CEF)
	set(CEF_BINARY_URL "https://github.com/winsoft666/cef_binary/archive/v${CEF_BINARY_VER}.tar.gz")
	
	if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cef_binary.tar.gz")
		message(STATUS "Downloading ${CEF_BINARY_URL}...")
		file(DOWNLOAD "${CEF_BINARY_URL}" "${CMAKE_CURRENT_SOURCE_DIR}/cef_binary.tar.gz" SHOW_PROGRESS STATUS download_status)
		list(GET download_status 0 status_code)
		if (NOT "${status_code}" STREQUAL "0")
			message(FATAL_ERROR "Downloading ${CEF_BINARY_URL}... Failed. Status: ${download_status}")
		else()
			execute_process(
				COMMAND ${CMAKE_COMMAND} -E tar xzf "${CMAKE_CURRENT_SOURCE_DIR}/cef_binary.tar.gz"
				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
				)
		endif()
	endif()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


add_subdirectory(src)

if(UILIB_WITH_CEF)
	add_subdirectory(libcef_wrapper_2623)
endif()

if(DUILIB2_BUILD_TESTS)
	add_subdirectory(duidemo)
endif()