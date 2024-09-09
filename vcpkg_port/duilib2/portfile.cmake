include(vcpkg_common_functions)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO winsoft666/duilib2
    HEAD_REF main
    REF b6027585138b5c633d6083a06c01797c92a21eea
    SHA512 7ba340004c14a9305479366ab8854187da1f993c49f96eae209c8eb0c51023d7f74f4d21e59600c37f5a129d7c7994ffd46f4b6601dc9362e4938c13a3c208b3
)

string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "static" LIBRARY_LINKAGE_HAS_STATIC)
string(COMPARE EQUAL "${VCPKG_CRT_LINKAGE}" "static" DUILIB2_USE_STATIC_CRT)

if(${LIBRARY_LINKAGE_HAS_STATIC})
	set(DUILIB2_BUILD_SHARED_LIBS OFF)
else()
	set(DUILIB2_BUILD_SHARED_LIBS ON)
endif()

set(UILIB_WITH_CEF OFF)
if("cef" IN_LIST FEATURES)
    set(UILIB_WITH_CEF ON)
    message(STATUS "UILIB_WITH_CEF=ON")
endif()

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DDUILIB2_BUILD_TESTS=OFF
		-DDUILIB2_BUILD_SHARED_LIBS=${DUILIB2_BUILD_SHARED_LIBS}
        -DDUILIB2_USE_STATIC_CRT=${DUILIB2_USE_STATIC_CRT}
		-DUILIB_WITH_CEF=${UILIB_WITH_CEF}
)

vcpkg_install_cmake()

if(EXISTS ${CURRENT_PACKAGES_DIR}/lib/cmake/duilib2)
    vcpkg_fixup_cmake_targets(CONFIG_PATH lib/cmake/duilib2)
elseif(EXISTS ${CURRENT_PACKAGES_DIR}/share/duilib2)
    vcpkg_fixup_cmake_targets(CONFIG_PATH share/duilib2)
endif()

file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/duilib2 RENAME copyright)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/share)

vcpkg_copy_pdbs()