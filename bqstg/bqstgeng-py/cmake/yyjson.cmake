include(ExternalProject)
include(cmake/config.cmake)

set(YYJSON_MAJOR_VER 0)
set(YYJSON_MINOR_VER 5)
set(YYJSON_PATCH_VER 1)
set(YYJSON_HASH SHA256=b484d40b4e20cc3174a6fdc160d0f20f961417f9cb3f6dc1cf6555fffa8359f3)

set(YYJSON_VER       ${YYJSON_MAJOR_VER}.${YYJSON_MINOR_VER}.${YYJSON_PATCH_VER})
set(YYJSON_ROOT      ${3RDPARTY_PATH}/yyjson)
set(YYJSON_INC_DIR   ${YYJSON_ROOT}/src/yyjson-${YYJSON_VER}/src)
set(YYJSON_LIB_DIR   ${YYJSON_ROOT}/src/yyjson-${YYJSON_VER}/build)

set(YYJSON_URL           https://github.com/ibireme/yyjson/archive/refs/tags/${YYJSON_VER}.tar.gz)
set(YYJSON_CONFIGURE     cd ${YYJSON_ROOT}/src/yyjson-${YYJSON_VER} )
set(YYJSON_BUILD         cd ${YYJSON_ROOT}/src/yyjson-${YYJSON_VER} && mkdir build && cd build && cmake .. && make CXXFLAGS+='-fPIC')
set(YYJSON_INSTALL       echo "install yyjson")

ExternalProject_Add(yyjson-${YYJSON_VER}
    URL                   ${YYJSON_URL}
    URL_HASH              ${YYJSON_HASH} 
    DOWNLOAD_NAME         yyjson-${YYJSON_VER}.tar.gz
    PREFIX                ${YYJSON_ROOT}
    CONFIGURE_COMMAND     ${YYJSON_CONFIGURE}
    BUILD_COMMAND         ${YYJSON_BUILD}
    INSTALL_COMMAND       ${YYJSON_INSTALL}
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} yyjson-${YYJSON_VER})

if (NOT EXISTS ${YYJSON_ROOT}/src/yyjson-${YYJSON_VER})
    add_custom_target(rescan-yyjson ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS yyjson-${YYJSON_VER})
else()
    add_custom_target(rescan-yyjson)
endif()

