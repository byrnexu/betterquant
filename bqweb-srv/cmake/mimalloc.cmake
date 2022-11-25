include(ExternalProject)
include(cmake/config.cmake)

set(MIMALLOC_MAJOR_VER 2)
set(MIMALLOC_MINOR_VER 0)
set(MIMALLOC_PATCH_VER 6)
set(MIMALLOC_URL_HASH  SHA256=9f05c94cc2b017ed13698834ac2a3567b6339a8bde27640df5a1581d49d05ce5)

set(MIMALLOC_VER       ${MIMALLOC_MAJOR_VER}.${MIMALLOC_MINOR_VER}.${MIMALLOC_PATCH_VER})
set(MIMALLOC_ROOT      ${3RDPARTY_PATH}/mimalloc)
set(MIMALLOC_INC_DIR   ${MIMALLOC_ROOT}/src/mimalloc-${MIMALLOC_VER}/include/)
set(MIMALLOC_LIB_DIR   ${MIMALLOC_ROOT}/src/mimalloc-${MIMALLOC_VER}/build/)

set(MIMALLOC_URL       https://github.com/microsoft/mimalloc/archive/refs/tags/v${MIMALLOC_VER}.tar.gz)
set(MIMALLOC_CONFIGURE cd ${MIMALLOC_ROOT}/src/mimalloc-${MIMALLOC_VER} && mkdir -p build && cd build && cmake ..)
set(MIMALLOC_BUILD     cd ${MIMALLOC_ROOT}/src/mimalloc-${MIMALLOC_VER} && cd build && make)
set(MIMALLOC_INSTALL   echo "install mimalloc")

ExternalProject_Add(mimalloc-${MIMALLOC_VER}
    URL               ${MIMALLOC_URL}
    URL_HASH          ${MIMALLOC_URL_HASH} 
    DOWNLOAD_NAME     mimalloc-${MIMALLOC_VER}.tar.gz
    PREFIX            ${MIMALLOC_ROOT}
    CONFIGURE_COMMAND ${MIMALLOC_CONFIGURE}
    BUILD_COMMAND     ${MIMALLOC_BUILD}
    INSTALL_COMMAND   ${MIMALLOC_INSTALL}
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} mimalloc-${MIMALLOC_VER})

if (NOT EXISTS ${MIMALLOC_ROOT}/src/mimalloc-${MIMALLOC_VER})
    add_custom_target(rescan-mimalloc ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS mimalloc-${MIMALLOC_VER})
else()
    add_custom_target(rescan-mimalloc)
endif()

