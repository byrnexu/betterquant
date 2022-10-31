include(ExternalProject)
include(cmake/config.cmake)

set(SPDLOG_MAJOR_VER 1)
set(SPDLOG_MINOR_VER 10)
set(SPDLOG_PATCH_VER 0)
set(SPDLOG_URL_HASH  SHA256=697f91700237dbae2326b90469be32b876b2b44888302afbc7aceb68bcfe8224)

set(SPDLOG_VER     ${SPDLOG_MAJOR_VER}.${SPDLOG_MINOR_VER}.${SPDLOG_PATCH_VER})
set(SPDLOG_ROOT    ${3RDPARTY_PATH}/spdlog)
set(SPDLOG_INC_DIR ${SPDLOG_ROOT}/src/spdlog-${SPDLOG_VER}/include)
set(SPDLOG_LIB_DIR ${SPDLOG_ROOT}/src/spdlog-${SPDLOG_VER}-build)

set(SPDLOG_URL https://github.com/gabime/spdlog/archive/refs/tags/v${SPDLOG_VER}.tar.gz)
set(SPDLOG_CONFIGURE cd ${SPDLOG_ROOT}/src/spdlog-${SPDLOG_VER} && mkdir -p build && cd build && cmake ..)
set(SPDLOG_BUILD     cd ${SPDLOG_ROOT}/src/spdlog-${SPDLOG_VER} && cd build && make CXXFLAGS+='-fPIC' -j4)
set(SPDLOG_INSTALL   echo "install spdlog")

ExternalProject_Add(spdlog-${SPDLOG_VER}
    URL               ${SPDLOG_URL}
    URL_HASH          ${SPDLOG_URL_HASH} 
    DOWNLOAD_NAME     spdlog-${SPDLOG_VER}.tar.gz
    PREFIX            ${SPDLOG_ROOT}
    CONFIGURE_COMMAND #{SPDLOG_CONFIGURE} 
    BUILD_COMMAND     #{SPDLOG_BUILD} 
    INSTALL_COMMAND   #{SPDLOG_INSTALL} 
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} spdlog-${SPDLOG_VER})

if (NOT EXISTS ${SPDLOG_ROOT}/src/spdlog-${SPDLOG_VER})
    add_custom_target(rescan-spdlog ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS spdlog-${SPDLOG_VER})
else()
    add_custom_target(rescan-spdlog)
endif()

