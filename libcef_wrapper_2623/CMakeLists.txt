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

set(LIB_NAME libcef_dll_wrapper_2623)


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

file(GLOB_RECURSE SOURCE_FILES                ./libcef_dll/*.cc)
file(GLOB_RECURSE HEADER_FILES                ./libcef_dll/*.h)


source_group("Header Files" FILES 
	${HEADER_FILES})

source_group("Source Files" FILES 
	${SOURCE_FILES})




add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN -DUSING_CEF_SHARED)

add_library(
	${LIB_NAME}
	STATIC
	${HEADER_FILES}
	${SOURCE_FILES}
)

# set output name prefix/suffix
set_target_properties(${LIB_NAME} PROPERTIES 
	OUTPUT_NAME ${LIB_NAME}
	DEBUG_OUTPUT_NAME ${LIB_NAME}${DEBUG_LIBNAME_SUFFIX})
	

target_include_directories(${LIB_NAME} PRIVATE 
	"../cef_binary-${CEF_BINARY_VER}/2623/include" ".")


# Set output directory
set_target_properties(${LIB_NAME}
	PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

