include(ExternalProject)
include(cmake/config.cmake)

set(NLOHMANN_JSON_MAJOR_VER 3)
set(NLOHMANN_JSON_MINOR_VER 11)
set(NLOHMANN_JSON_PATCH_VER 2)
set(NLOHMANN_URL_HASH SHA256=d69f9deb6a75e2580465c6c4c5111b89c4dc2fa94e3a85fcd2ffcd9a143d9273)

set(NLOHMANN_JSON_VER     ${NLOHMANN_JSON_MAJOR_VER}.${NLOHMANN_JSON_MINOR_VER}.${NLOHMANN_JSON_PATCH_VER})
set(NLOHMANN_JSON_ROOT    ${3RDPARTY_PATH}/nlohmann_json)
set(NLOHMANN_JSON_INC_DIR ${NLOHMANN_JSON_ROOT}/src/nlohmann_json-${NLOHMANN_JSON_VER}/include)
set(NLOHMANN_INSTALL      echo "install nlohmann")

set(NLOHMANN_JSON_URL https://github.com/nlohmann/json/archive/refs/tags/v${NLOHMANN_JSON_VER}.tar.gz)

ExternalProject_Add(nlohmann_json-${NLOHMANN_JSON_VER}
    URL               ${NLOHMANN_JSON_URL}
    URL_HASH          ${NLOHMANN_URL_HASH} 
    DOWNLOAD_NAME     nlohmann_json-${NLOHMANN_JSON_VER}.tar.gz
    PREFIX            ${NLOHMANN_JSON_ROOT}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     ""
    INSTALL_COMMAND   ${NLOHMANN_INSTALL} 
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} nlohmann_json-${NLOHMANN_JSON_VER})

if (NOT EXISTS ${NLOHMANN_JSON_ROOT}/src/nlohmann_json-${NLOHMANN_JSON_VER})
    add_custom_target(rescan-nlohmann_json ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS nlohmann_json-${NLOHMANN_JSON_VER})
else()
    add_custom_target(rescan-nlohmann_json)
endif()

