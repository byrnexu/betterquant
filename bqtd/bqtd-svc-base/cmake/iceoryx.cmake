include(ExternalProject)
include(cmake/config.cmake)

set(ICEORYX_MAJOR_VER 2)
set(ICEORYX_MINOR_VER 0)
set(ICEORYX_PATCH_VER 2)
set(ICEORYX_URL_HASH  SHA256=99871bcaa8da4361d1baae9cf1507683058de8572ac3080edc41e590ffba06c0)

set(ICEORYX_VER       ${ICEORYX_MAJOR_VER}.${ICEORYX_MINOR_VER}.${ICEORYX_PATCH_VER})
set(ICEORYX_ROOT      ${3RDPARTY_PATH}/iceoryx)
set(ICEORYX_INC_DIR   /usr/local/include/iceoryx/v${ICEORYX_VER}/)
set(ICEORYX_LIB_DIR   /usr/local/lib)

set(ICEORYX_URL           https://github.com/eclipse-iceoryx/iceoryx/archive/refs/tags/v${ICEORYX_VER}.tar.gz)
set(ICEORYX_CONFIGURE     cd ${ICEORYX_ROOT}/src/iceoryx-${ICEORYX_VER} && cmake -Bbuild -Hiceoryx_meta && cmake -Bbuild -Hiceoryx_meta -DCMAKE_PREFIX_PATH=$(PWD)/build/dependencies/)
set(ICEORYX_BUILD         cd ${ICEORYX_ROOT}/src/iceoryx-${ICEORYX_VER} && cmake --build build)
set(ICEORYX_INSTALL       cd ${ICEORYX_ROOT}/src/iceoryx-${ICEORYX_VER} && sudo cmake --build build --target install)

ExternalProject_Add(iceoryx-${ICEORYX_VER}
    URL                   ${ICEORYX_URL}
    DOWNLOAD_NAME         iceoryx-${ICEORYX_VER}.tar.gz
    URL_HASH              ${ICEORYX_URL_HASH} 
    PREFIX                ${ICEORYX_ROOT}
    CONFIGURE_COMMAND     ${ICEORYX_CONFIGURE}
    BUILD_COMMAND         ${ICEORYX_BUILD}
    INSTALL_COMMAND       ${ICEORYX_INSTALL}
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} iceoryx-${ICEORYX_VER})

if (NOT EXISTS ${ICEORYX_ROOT}/src/iceoryx-${ICEORYX_VER})
    add_custom_target(rescan-iceoryx ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS iceoryx-${ICEORYX_VER})
else()
    add_custom_target(rescan-iceoryx)
endif()

