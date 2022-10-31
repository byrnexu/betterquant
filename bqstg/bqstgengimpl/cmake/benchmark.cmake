include(ExternalProject)
include(cmake/config.cmake)

set(BENCHMARK_MAJOR_VER 1)
set(BENCHMARK_MINOR_VER 5)
set(BENCHMARK_PATCH_VER 1)
set(BENCHMARK_URL_HASH  SHA256=23082937d1663a53b90cb5b61df4bcc312f6dee7018da78ba00dd6bd669dfef2)

set(BENCHMARK_VER       ${BENCHMARK_MAJOR_VER}.${BENCHMARK_MINOR_VER}.${BENCHMARK_PATCH_VER})
set(BENCHMARK_ROOT      ${3RDPARTY_PATH}/benchmark)
set(BENCHMARK_INC_DIR   ${BENCHMARK_ROOT}/src/benchmark-${BENCHMARK_VER}/include/)
set(BENCHMARK_LIB_DIR   ${BENCHMARK_ROOT}/src/benchmark-${BENCHMARK_VER}/build/src/)

set(BENCHMARK_URL       https://github.com/google/benchmark/archive/v${BENCHMARK_VER}.tar.gz)
set(BENCHMARK_CONFIGURE cd ${BENCHMARK_ROOT}/src/benchmark-${BENCHMARK_VER} && mkdir -p build && cd build && cmake .. -DCMAKE_BUILD_TYPE=release -DBENCHMARK_ENABLE_TESTING=OFF)
set(BENCHMARK_BUILD     cd ${BENCHMARK_ROOT}/src/benchmark-${BENCHMARK_VER} && cd build && make)
set(BENCHMARK_INSTALL   echo "install benchmark")

ExternalProject_Add(benchmark-${BENCHMARK_VER}
    URL               ${BENCHMARK_URL}
    URL_HASH          ${BENCHMARK_URL_HASH} 
    DOWNLOAD_NAME     benchmark-${BENCHMARK_VER}.tar.gz
    PREFIX            ${BENCHMARK_ROOT}
    CONFIGURE_COMMAND ${BENCHMARK_CONFIGURE}
    BUILD_COMMAND     ${BENCHMARK_BUILD}
    INSTALL_COMMAND   ${BENCHMARK_INSTALL}
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} benchmark-${BENCHMARK_VER})

if (NOT EXISTS ${BENCHMARK_ROOT}/src/benchmark-${BENCHMARK_VER})
    add_custom_target(rescan-benchmark ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS benchmark-${BENCHMARK_VER})
else()
    add_custom_target(rescan-benchmark)
endif()

