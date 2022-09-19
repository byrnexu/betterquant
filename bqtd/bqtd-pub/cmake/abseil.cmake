include(ExternalProject)
include(cmake/config.cmake)

set(ABSEIL_MAJOR_VER 20220623)
set(ABSEIL_MINOR_VER 0)
set(ABSEIL_PATCH_VER 0)
set(ABSEIL_URL_HASH  SHA256=4208129b49006089ba1d6710845a45e31c59b0ab6bff9e5788a87f55c5abd602)

set(ABSEIL_VER       ${ABSEIL_MAJOR_VER}.${ABSEIL_MINOR_VER}.${ABSEIL_PATCH_VER})
set(ABSEIL_ROOT      ${3RDPARTY_PATH}/abseil)
set(ABSEIL_INC_DIR   /usr/local/include)
set(ABSEIL_LIB_DIR   /usr/local/lib)

set(ABSEIL_URL           https://github.com/abseil/abseil-cpp/archive/refs/tags/20220623.0.tar.gz)
set(ABSEIL_CONFIGURE     cd ${ABSEIL_ROOT}/src/abseil-${ABSEIL_VER} && mkdir -p build && cd build && cmake -DCMAKE_CXX_STANDARD=17 ..)
set(ABSEIL_BUILD         cd ${ABSEIL_ROOT}/src/abseil-${ABSEIL_VER} && cd build && cmake --build . --target all)
set(ABSEIL_INSTALL       cd ${ABSEIL_ROOT}/src/abseil-${ABSEIL_VER} && cd build && make install)

ExternalProject_Add(abseil-${ABSEIL_VER}
    URL                   ${ABSEIL_URL}
    DOWNLOAD_NAME         abseil-${ABSEIL_VER}.tar.gz
    URL_HASH              ${ABSEIL_URL_HASH} 
    PREFIX                ${ABSEIL_ROOT}
    CONFIGURE_COMMAND     ${ABSEIL_CONFIGURE}
    BUILD_COMMAND         ${ABSEIL_BUILD}
    INSTALL_COMMAND       ${ABSEIL_INSTALL}
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} abseil-${ABSEIL_VER})

if (NOT EXISTS ${ABSEIL_ROOT}/src/abseil-${ABSEIL_VER})
    add_custom_target(rescan-abseil ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS abseil-${ABSEIL_VER})
else()
    add_custom_target(rescan-abseil)
endif()

