include(ExternalProject)
include(cmake/config.cmake)

set(GTEST_MAJOR_VER 1)
set(GTEST_MINOR_VER 10)
set(GTEST_PATCH_VER 0)
set(GTEST_URL_HASH  SHA256=9dc9157a9a1551ec7a7e43daea9a694a0bb5fb8bec81235d8a1e6ef64c716dcb)

set(GTEST_VER       ${GTEST_MAJOR_VER}.${GTEST_MINOR_VER}.${GTEST_PATCH_VER})
set(GTEST_ROOT      ${3RDPARTY_PATH}/gtest)
set(GTEST_INC_DIR   ${GTEST_ROOT}/src/gtest-${GTEST_VER}/googletest/include/)
set(GTEST_LIB_DIR   ${GTEST_ROOT}/src/gtest-${GTEST_VER}/build/lib/)

set(GTEST_URL       https://github.com/google/googletest/archive/release-${GTEST_VER}.tar.gz)
set(GTEST_CONFIGURE cd ${GTEST_ROOT}/src/gtest-${GTEST_VER} && mkdir -p build && cd build && cmake ..)
set(GTEST_BUILD     cd ${GTEST_ROOT}/src/gtest-${GTEST_VER} && cd build && make)
set(GTEST_INSTALL   echo "install gtest")

ExternalProject_Add(gtest-${GTEST_VER}
    URL               ${GTEST_URL}
    URL_HASH          ${GTEST_URL_HASH} 
    DOWNLOAD_NAME     gtest-${GTEST_VER}.tar.gz
    PREFIX            ${GTEST_ROOT}
    CONFIGURE_COMMAND ${GTEST_CONFIGURE}
    BUILD_COMMAND     ${GTEST_BUILD}
    INSTALL_COMMAND   ${GTEST_INSTALL}
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} gtest-${GTEST_VER})

if (NOT EXISTS ${GTEST_ROOT}/src/gtest-${GTEST_VER})
    add_custom_target(rescan-gtest ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS gtest-${GTEST_VER})
else()
    add_custom_target(rescan-gtest)
endif()

