include(ExternalProject)
include(cmake/config.cmake)

set(GFLAGS_MAJOR_VER 2)
set(GFLAGS_MINOR_VER 2)
set(GFLAGS_PATCH_VER 2)
set(GFLAGS_URL_HASH  SHA256=34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf)

set(GFLAGS_VER       ${GFLAGS_MAJOR_VER}.${GFLAGS_MINOR_VER}.${GFLAGS_PATCH_VER})
set(GFLAGS_ROOT      ${3RDPARTY_PATH}/gflags)
set(GFLAGS_INC_DIR   ${GFLAGS_ROOT}/src/gflags-${GFLAGS_VER}/include)
set(GFLAGS_LIB_DIR   ${GFLAGS_ROOT}/src/gflags-${GFLAGS_VER}/lib)

set(GFLAGS_URL           https://github.com/gflags/gflags/archive/v${GFLAGS_VER}.tar.gz)
set(GFLAGS_CONFIGURE     cd ${GFLAGS_ROOT}/src/gflags-${GFLAGS_VER} && cmake -DCMAKE_CXX_FLAGS=-fPIC -DCMAKE_INSTALL_PREFIX=${GFLAGS_ROOT}/src/gflags-${GFLAGS_VER} .)
set(GFLAGS_BUILD         cd ${GFLAGS_ROOT}/src/gflags-${GFLAGS_VER} && make CXXFLAGS+='-fPIC')
set(GFLAGS_INSTALL       cd ${GFLAGS_ROOT}/src/gflags-${GFLAGS_VER} && make install)

ExternalProject_Add(gflags-${GFLAGS_VER}
    URL                    ${GFLAGS_URL}
    URL_HASH               ${GFLAGS_URL_HASH} 
    DOWNLOAD_NAME          gflags-${GFLAGS_VER}.tar.gz
    PREFIX                 ${GFLAGS_ROOT}
    CONFIGURE_COMMAND      ${GFLAGS_CONFIGURE}
    BUILD_COMMAND          ${GFLAGS_BUILD}
    INSTALL_COMMAND        ${GFLAGS_INSTALL}
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} gflags-${GFLAGS_VER})

if (NOT EXISTS ${GFLAGS_ROOT}/src/gflags-${GFLAGS_VER})
    add_custom_target(rescan-gflags ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS gflags-${GFLAGS_VER})
else()
    add_custom_target(rescan-gflags)
endif()

