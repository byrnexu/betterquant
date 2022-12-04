include(ExternalProject)
include(cmake/config.cmake)

set(MAGIC_ENUM_MAJOR_VER 0)
set(MAGIC_ENUM_MINOR_VER 7)
set(MAGIC_ENUM_PATCH_VER 3)
set(MAGIC_ENUM_URL_HASH  SHA256=b8d0cd848546fee136dc1fa4bb021a1e4dc8fe98e44d8c119faa3ef387636bf7)

set(MAGIC_ENUM_VER     ${MAGIC_ENUM_MAJOR_VER}.${MAGIC_ENUM_MINOR_VER}.${MAGIC_ENUM_PATCH_VER})
set(MAGIC_ENUM_ROOT    ${3RDPARTY_PATH}/magic_enum)
set(MAGIC_ENUM_INC_DIR ${MAGIC_ENUM_ROOT}/src/magic_enum-${MAGIC_ENUM_VER}/include)
set(MAGIC_INSTALL      echo "install magic enum")

set(MAGIC_ENUM_URL https://github.com/Neargye/magic_enum/archive/refs/tags/v${MAGIC_ENUM_VER}.tar.gz)

ExternalProject_Add(magic_enum-${MAGIC_ENUM_VER}
    URL               ${MAGIC_ENUM_URL}
    URL_HASH          ${MAGIC_ENUM_URL_HASH} 
    DOWNLOAD_NAME     magic_enum-${MAGIC_ENUM_VER}.tar.gz
    PREFIX            ${MAGIC_ENUM_ROOT}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     ""
    INSTALL_COMMAND   ${MAGIC_INSTALL}
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} magic_enum-${MAGIC_ENUM_VER})

if (NOT EXISTS ${MAGIC_ENUM_ROOT}/src/magic_enum-${MAGIC_ENUM_VER})
    add_custom_target(rescan-magic_enum ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS magic_enum-${MAGIC_ENUM_VER})
else()
    add_custom_target(rescan-magic_enum)
endif()

