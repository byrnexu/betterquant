include(ExternalProject)
include(cmake/config.cmake)

set(FMT_MAJOR_VER 6)
set(FMT_MINOR_VER 2)
set(FMT_PATCH_VER 1)
set(FMT_URL_HASH  SHA256=5edf8b0f32135ad5fafb3064de26d063571e95e8ae46829c2f4f4b52696bbff0)

set(FMT_VER       ${FMT_MAJOR_VER}.${FMT_MINOR_VER}.${FMT_PATCH_VER})
set(FMT_ROOT      ${3RDPARTY_PATH}/fmt)
set(FMT_INC_DIR   ${FMT_ROOT}/src/fmt-${FMT_VER}/include)
set(FMT_LIB_DIR   ${FMT_ROOT}/src/fmt-${FMT_VER}/build)

set(FMT_URL  https://github.com/fmtlib/fmt/archive/refs/tags/${FMT_VER}.tar.gz)
set(FMT_CONFIGURE cd ${FMT_ROOT}/src/fmt-${FMT_VER} && mkdir -p build && cd build && cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON)
set(FMT_BUILD     cd ${FMT_ROOT}/src/fmt-${FMT_VER} && cd build && make -j8)
set(FMT_INSTALL   echo "install fmt")

ExternalProject_Add(fmt-${FMT_VER}
    URL                 ${FMT_URL}
    URL_HASH            ${FMT_URL_HASH} 
    DOWNLOAD_NAME       fmt-${FMT_VER}.tar.gz
    PREFIX              ${FMT_ROOT}
    CONFIGURE_COMMAND   ${FMT_CONFIGURE}
    BUILD_COMMAND       ${FMT_BUILD}
    INSTALL_COMMAND     ${FMT_INSTALL}
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} fmt-${FMT_VER})

if (NOT EXISTS ${FMT_ROOT}/src/fmt-${FMT_VER})
    add_custom_target(rescan-fmt ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS fmt-${FMT_VER})
else()
    add_custom_target(rescan-fmt)
endif()

