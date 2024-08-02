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

if(DUILIB2_BUILD_SHARED_LIBS)
	set(LIB_NAME duilib2)
else()
	set(LIB_NAME duilib2-static)
endif()


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


file(GLOB ROOT_SOURCE_FILES 			./UIlib.cpp)
file(GLOB CONTROL_SOURCE_FILES 		./Control/*.cpp)
file(GLOB CORE_SOURCE_FILES 		    ./Core/*.cpp)
file(GLOB LAYOUT_SOURCE_FILES 		./Layout/*.cpp)
file(GLOB RENDER_SOURCE_FILES 		./Render/*.cpp)
file(GLOB UTILS_SOURCE_FILES 		./Utils/*.cpp)
if(UILIB_WITH_CEF)
	file(GLOB INTERNAL_CEF_SOURCE_FILES     ./Internal/Cef/*.cpp)
endif()
file(GLOB INTERNAL_SOURCE_FILES 		./Internal/*.cpp)

file(GLOB ROOT_HEADER_FILES 			./UIlib.h ./UIlibExport.h ./UILibInternal.h)
file(GLOB CONTROL_HEADER_FILES 		./Control/*.h ./Control/*.hpp)
file(GLOB CORE_HEADER_FILES 		    ./Core/*.h ./Core/*.hpp)
file(GLOB LAYOUT_HEADER_FILES 		./Layout/*.h ./Layout/*.hpp)
file(GLOB RENDER_HEADER_FILES 		./Render/*.h ./Render/*.hpp)
file(GLOB UTILS_HEADER_FILES 		./Utils/*.h ./Utils/*.hpp ./Utils/*.tlh)
if(UILIB_WITH_CEF)
	file(GLOB INTERNAL_CEF_HEADER_FILES     ./Internal/Cef/*.h)
endif()
file(GLOB INTERNAL_HEADER_FILES 		./Internal/*.h ./Internal/*.hpp)

source_group("Header Files" FILES 
	${ROOT_HEADER_FILES} ./StdAfx.h)
source_group("Header Files\\Control" FILES 
	${CONTROL_HEADER_FILES})
source_group("Header Files\\Core" FILES 
	${CORE_HEADER_FILES})
source_group("Header Files\\Layout" FILES 
	${LAYOUT_HEADER_FILES})
source_group("Header Files\\Render" FILES 
	${RENDER_HEADER_FILES})
source_group("Header Files\\Utils" FILES 
	${UTILS_HEADER_FILES})
source_group("Header Files\\Internal" FILES 
	${INTERNAL_HEADER_FILES})
if(UILIB_WITH_CEF)
	source_group("Header Files\\Internal\\Cef" FILES 
		${INTERNAL_CEF_HEADER_FILES})
endif()

source_group("Source Files" FILES 
	${ROOT_SOURCE_FILES} ./StdAfx.cpp)
source_group("Source Files\\Control" FILES 
	${CONTROL_SOURCE_FILES})
source_group("Source Files\\Core" FILES 
	${CORE_SOURCE_FILES})
source_group("Source Files\\Layout" FILES 
	${LAYOUT_SOURCE_FILES})
source_group("Source Files\\Render" FILES 
	${RENDER_SOURCE_FILES})
source_group("Source Files\\Utils" FILES 
	${UTILS_SOURCE_FILES})
source_group("Source Files\\Internal" FILES 
	${INTERNAL_SOURCE_FILES})
if(UILIB_WITH_CEF)
	source_group("Source Files\\Internal\\Cef" FILES 
		${INTERNAL_CEF_SOURCE_FILES})
endif()

if (MSVC)
    # set_source_files_properties(StdAfx.cpp
        # PROPERTIES
        # COMPILE_FLAGS "/YcStdAfx.h"
        # )
    # foreach( src_file ${ROOT_SOURCE_FILES}
		# ${CONTROL_SOURCE_FILES}
		# ${CORE_SOURCE_FILES}
		# ${LAYOUT_SOURCE_FILES}
		# ${RENDER_SOURCE_FILES}
		# ${UTILS_SOURCE_FILES}
		# ${INTERNAL_SOURCE_FILES} )
        # set_source_files_properties(
            # ${src_file}
            # PROPERTIES
            # COMPILE_FLAGS "/YuStdAfx.h"
            # )
    # endforeach()

endif (MSVC)


if(DUILIB2_BUILD_SHARED_LIBS)
	add_definitions(-DUILIB_EXPORTS)
	set(UILIB_STATIC NO)
	
	add_library(
		${LIB_NAME}
		SHARED
		${ROOT_HEADER_FILES} ./StdAfx.h
		${CONTROL_HEADER_FILES}
		${CORE_HEADER_FILES}
		${LAYOUT_HEADER_FILES}
		${RENDER_HEADER_FILES}
		${UTILS_HEADER_FILES}
		${INTERNAL_HEADER_FILES}
		${ROOT_SOURCE_FILES} ./StdAfx.cpp
		${CONTROL_SOURCE_FILES}
		${CORE_SOURCE_FILES}
		${LAYOUT_SOURCE_FILES}
		${RENDER_SOURCE_FILES}
		${UTILS_SOURCE_FILES}
		${INTERNAL_SOURCE_FILES}
		${INTERNAL_CEF_HEADER_FILES}
		${INTERNAL_CEF_SOURCE_FILES}
	)
	
	# set output name prefix/suffix
	set_target_properties(${LIB_NAME} PROPERTIES 
		OUTPUT_NAME ${LIB_NAME}
		DEBUG_OUTPUT_NAME ${LIB_NAME}${DEBUG_LIBNAME_SUFFIX})
else()
	add_definitions(-DUILIB_STATIC)
	set(UILIB_STATIC YES)
	
	add_library(
		${LIB_NAME}
		STATIC
		${ROOT_HEADER_FILES} ./StdAfx.h
		${CONTROL_HEADER_FILES}
		${CORE_HEADER_FILES}
		${LAYOUT_HEADER_FILES}
		${RENDER_HEADER_FILES}
		${UTILS_HEADER_FILES}
		${INTERNAL_HEADER_FILES}
		${ROOT_SOURCE_FILES} ./StdAfx.cpp
		${CONTROL_SOURCE_FILES}
		${CORE_SOURCE_FILES}
		${LAYOUT_SOURCE_FILES}
		${RENDER_SOURCE_FILES}
		${UTILS_SOURCE_FILES}
		${INTERNAL_SOURCE_FILES}
		${INTERNAL_CEF_HEADER_FILES}
		${INTERNAL_CEF_SOURCE_FILES}
	)
	
	# set output name prefix/suffix
    set_target_properties(${LIB_NAME} PROPERTIES 
		OUTPUT_NAME ${LIB_NAME}
		DEBUG_OUTPUT_NAME ${LIB_NAME}${DEBUG_LIBNAME_SUFFIX})
		
	set_target_properties(${LIB_NAME} PROPERTIES 
		INTERFACE_COMPILE_DEFINITIONS UILIB_STATIC)
endif()


target_include_directories(${LIB_NAME} PRIVATE 
	 ".")

if(UILIB_WITH_CEF)
	target_include_directories(${LIB_NAME} PRIVATE 
		"${CMAKE_CURRENT_SOURCE_DIR}/../cef_binary-${CEF_BINARY_VER}/2623/include")
endif()

if(UILIB_WITH_CEF)
	add_dependencies(${LIB_NAME} libcef_dll_wrapper_2623)
endif()


if(UILIB_WITH_CEF)
	target_link_libraries(${LIB_NAME} PRIVATE
		$<TARGET_LINKER_FILE:libcef_dll_wrapper_2623> 
		"${CMAKE_CURRENT_SOURCE_DIR}/../cef_binary-${CEF_BINARY_VER}/2623/lib/libcef.lib")
endif()

target_include_directories(${LIB_NAME} INTERFACE
	$<INSTALL_INTERFACE:include>)

if(UILIB_WITH_CEF)
	file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Debug" "${CMAKE_BINARY_DIR}/bin/Release")
	add_custom_command(
		TARGET ${LIB_NAME}
		POST_BUILD
		COMMAND "powershell" " -noprofile" "-command" "copy-item ${CMAKE_CURRENT_SOURCE_DIR}/../cef_binary-${CEF_BINARY_VER}/2623/bin/*.* ${CMAKE_BINARY_DIR}/bin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release> -force -recurse"
	)
endif()


install(TARGETS ${LIB_NAME}
	EXPORT ${LIB_NAME}-target
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)


install(FILES ${ROOT_HEADER_FILES} 			DESTINATION include/duilib2)
install(FILES ${CONTROL_HEADER_FILES} 		DESTINATION include/duilib2/Control)
install(FILES ${CORE_HEADER_FILES} 		    DESTINATION include/duilib2/Core)
install(FILES ${LAYOUT_HEADER_FILES} 		DESTINATION include/duilib2/Layout)
install(FILES ${RENDER_HEADER_FILES} 		DESTINATION include/duilib2/Render)
install(FILES ${UTILS_HEADER_FILES} 		DESTINATION include/duilib2/Utils)

# if(UILIB_WITH_CEF)
	# install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../cef_binary-${CEF_BINARY_VER}/2623/bin/" 
		# DESTINATION
		# bin)
# endif()

install(TARGETS ${LIB_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    OBJECTS DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
