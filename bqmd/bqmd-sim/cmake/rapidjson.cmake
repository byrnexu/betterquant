include(ExternalProject)
include(cmake/config.cmake)

set(RAPIDJSON_MAJOR_VER 1)
set(RAPIDJSON_MINOR_VER 1)
set(RAPIDJSON_PATCH_VER 0)
set(RAPIDJSON_URL_HASH  SHA256=bf7ced29704a1e696fbccf2a2b4ea068e7774fa37f6d7dd4039d0787f8bed98e)

set(RAPIDJSON_VER     ${RAPIDJSON_MAJOR_VER}.${RAPIDJSON_MINOR_VER}.${RAPIDJSON_PATCH_VER})
set(RAPIDJSON_ROOT    ${3RDPARTY_PATH}/rapidjson)
set(RAPIDJSON_INC_DIR ${RAPIDJSON_ROOT}/src/rapidjson-${RAPIDJSON_VER}/include)
set(RAPIDJSON_INSTALL echo "install rapidjson")

set(RAPIDJSON_URL https://github.com/Tencent/rapidjson/archive/v${RAPIDJSON_VER}.tar.gz)

ExternalProject_Add(rapidjson-${RAPIDJSON_VER}
    URL               ${RAPIDJSON_URL}
    URL_HASH          ${RAPIDJSON_URL_HASH} 
    DOWNLOAD_NAME     rapidjson-${RAPIDJSON_VER}.tar.gz
    PREFIX            ${RAPIDJSON_ROOT}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     ""
    INSTALL_COMMAND   ${RAPIDJSON_INSTALL}
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} rapidjson-${RAPIDJSON_VER})

if (NOT EXISTS ${RAPIDJSON_ROOT}/src/rapidjson-${RAPIDJSON_VER})
    add_custom_target(rescan-rapidjson ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS rapidjson-${RAPIDJSON_VER})
else()
    add_custom_target(rescan-rapidjson)
endif()

