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

set(EXE_NAME duidemo)


if (MSVC AND DUILIB2_USE_STATIC_CRT)
    set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
endif()


if (NOT DUILIB2_BUILD_SHARED_LIBS)
	# Use ppxnet static library
	add_definitions(-DUILIB_STATIC)
endif()

file(GLOB ROOT_SOURCE_FILES 			*.cpp)
list(FILTER ROOT_SOURCE_FILES EXCLUDE REGEX  ".*StdAfx.cpp$")

file(GLOB ROOT_HEADER_FILES 			*.h)
list(FILTER ROOT_HEADER_FILES EXCLUDE REGEX  ".*StdAfx.h$")

source_group("Header Files" FILES 
	${ROOT_HEADER_FILES} StdAfx.h)

source_group("Source Files" FILES 
	${ROOT_SOURCE_FILES} StdAfx.cpp)

add_executable(
	${EXE_NAME}
	WIN32
	${ROOT_HEADER_FILES} StdAfx.h
	${ROOT_SOURCE_FILES} StdAfx.cpp
	duidemo.rc
)

target_include_directories(${EXE_NAME} PUBLIC "../src")


if (MSVC)
    # set_source_files_properties(StdAfx.cpp
        # PROPERTIES
        # COMPILE_FLAGS "/YcStdAfx.h"
        # )
    # foreach( src_file ${ROOT_SOURCE_FILES})
        # set_source_files_properties(
            # ${src_file}
            # PROPERTIES
            # COMPILE_FLAGS "/YuStdAfx.h"
            # )
    # endforeach()
	
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /source-charset:.936")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /source-charset:.936")
endif (MSVC)
	
if(UILIB_WITH_CEF)
	target_link_libraries(${EXE_NAME}
		$<TARGET_LINKER_FILE:libcef_dll_wrapper_2623> 
		"${CMAKE_CURRENT_SOURCE_DIR}/../cef_binary-${CEF_BINARY_VER}/2623/lib/libcef.lib")
endif()

if(DUILIB2_BUILD_SHARED_LIBS)
	add_dependencies(${EXE_NAME} duilib2)
	
	target_link_libraries(${EXE_NAME} 
		$<TARGET_LINKER_FILE:duilib2>)
else()
	add_dependencies(${EXE_NAME} duilib2-static)
	
	target_link_libraries(${EXE_NAME} 
		$<TARGET_LINKER_FILE:duilib2-static>)
endif()

