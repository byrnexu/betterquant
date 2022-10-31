include(ExternalProject)
include(cmake/config.cmake)

set(WEBSOCKETPP_MAJOR_VER 0)
set(WEBSOCKETPP_MINOR_VER 8)
set(WEBSOCKETPP_PATCH_VER 2)
set(WEBSOCKETPP_URL_HASH  SHA256=6ce889d85ecdc2d8fa07408d6787e7352510750daa66b5ad44aacb47bea76755)

set(WEBSOCKETPP_VER     ${WEBSOCKETPP_MAJOR_VER}.${WEBSOCKETPP_MINOR_VER}.${WEBSOCKETPP_PATCH_VER})
set(WEBSOCKETPP_ROOT    ${3RDPARTY_PATH}/websocketpp)
set(WEBSOCKETPP_INC_DIR ${WEBSOCKETPP_ROOT}/src/websocketpp-${WEBSOCKETPP_VER}/)
set(WEBSOCKETPP_INSTALL echo "install websocketpp")

set(WEBSOCKETPP_URL https://github.com/zaphoyd/websocketpp/archive/refs/tags/${WEBSOCKETPP_VER}.tar.gz)

ExternalProject_Add(websocketpp-${WEBSOCKETPP_VER}
    URL               ${WEBSOCKETPP_URL}
    URL_HASH          ${WEBSOCKETPP_URL_HASH} 
    DOWNLOAD_NAME     websocketpp-${WEBSOCKETPP_VER}.tar.gz
    PREFIX            ${WEBSOCKETPP_ROOT}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     ""
    INSTALL_COMMAND   ${WEBSOCKETPP_INSTALL} 
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} websocketpp-${WEBSOCKETPP_VER})

if (NOT EXISTS ${WEBSOCKETPP_ROOT}/src/websocketpp-${WEBSOCKETPP_VER})
    add_custom_target(rescan-websocketpp ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS websocketpp-${WEBSOCKETPP_VER})
else()
    add_custom_target(rescan-websocketpp)
endif()

