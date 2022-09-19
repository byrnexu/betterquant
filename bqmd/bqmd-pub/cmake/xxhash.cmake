include(ExternalProject)
include(cmake/config.cmake)

set(XXHASH_MAJOR_VER 0)
set(XXHASH_MINOR_VER 8)
set(XXHASH_PATCH_VER 1)
set(XXHASH_HASH SHA256=3bb6b7d6f30c591dd65aaaff1c8b7a5b94d81687998ca9400082c739a690436c)

set(XXHASH_VER       ${XXHASH_MAJOR_VER}.${XXHASH_MINOR_VER}.${XXHASH_PATCH_VER})
set(XXHASH_ROOT      ${3RDPARTY_PATH}/xxHash)
set(XXHASH_INC_DIR   ${XXHASH_ROOT}/src/xxHash-${XXHASH_VER})
set(XXHASH_LIB_DIR   ${XXHASH_ROOT}/src/xxHash-${XXHASH_VER})

set(XXHASH_URL           https://github.com/Cyan4973/xxHash/archive/refs/tags/v${XXHASH_VER}.tar.gz)
set(XXHASH_CONFIGURE     cd ${XXHASH_ROOT}/src/xxHash-${XXHASH_VER} )
set(XXHASH_BUILD         cd ${XXHASH_ROOT}/src/xxHash-${XXHASH_VER} && make CXXFLAGS+='-fPIC')
set(XXHASH_INSTALL       cd ${XXHASH_ROOT}/src/xxHash-${XXHASH_VER} && make install)

ExternalProject_Add(xxHash-${XXHASH_VER}
    URL                   ${XXHASH_URL}
    URL_HASH              ${XXHASH_HASH} 
    DOWNLOAD_NAME         xxHash-${XXHASH_VER}.tar.gz
    PREFIX                ${XXHASH_ROOT}
    CONFIGURE_COMMAND     ${XXHASH_CONFIGURE}
    BUILD_COMMAND         ${XXHASH_BUILD}
    INSTALL_COMMAND       ${XXHASH_INSTALL}
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} xxHash-${XXHASH_VER})

if (NOT EXISTS ${XXHASH_ROOT}/src/xxHash-${XXHASH_VER})
    add_custom_target(rescan-xxHash ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS xxHash-${XXHASH_VER})
else()
    add_custom_target(rescan-xxHash)
endif()

