include(ExternalProject)
include(cmake/config.cmake)

set(CPR_MAJOR_VER 1)
set(CPR_MINOR_VER 7)
set(CPR_PATCH_VER 2)
set(CPR_URL_HASH  SHA256=aa38a414fe2ffc49af13a08b6ab34df825fdd2e7a1213d032d835a779e14176f)

set(CPR_VER       ${CPR_MAJOR_VER}.${CPR_MINOR_VER}.${CPR_PATCH_VER})
set(CPR_ROOT      ${3RDPARTY_PATH}/cpr)
set(CPR_INC_DIR   ${CPR_ROOT}/src/cpr-${CPR_VER}/include)
set(CPR_LIB_DIR   ${CPR_ROOT}/src/cpr-${CPR_VER}/build/lib)

set(CPR_URL       https://github.com/libcpr/cpr/archive/refs/tags/${CPR_VER}.tar.gz)
set(CPR_CONFIGURE cd ${CPR_ROOT}/src/cpr-${CPR_VER} && mkdir -p build && cd build && cmake ..)
set(CPR_BUILD     cd ${CPR_ROOT}/src/cpr-${CPR_VER} && cd build && make && make install)
set(CPR_INSTALL   echo  "install cpr")

ExternalProject_Add(cpr-${CPR_VER}
    URL                 ${CPR_URL}
    URL_HASH            ${CPR_URL_HASH} 
    DOWNLOAD_NAME       cpr-${CPR_VER}.tar.gz
    PREFIX              ${CPR_ROOT}
    CONFIGURE_COMMAND   ${CPR_CONFIGURE}
    BUILD_COMMAND       ${CPR_BUILD}
    INSTALL_COMMAND     ${CPR_INSTALL}
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} cpr-${CPR_VER})

if (NOT EXISTS ${CPR_ROOT}/src/cpr-${CPR_VER})
    add_custom_target(rescan-cpr ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS cpr-${CPR_VER})
else()
    add_custom_target(rescan-cpr)
endif()

