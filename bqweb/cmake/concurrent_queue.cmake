include(ExternalProject)
include(cmake/config.cmake)

set(CONCURRENT_QUEUE_MAJOR_VER 1)
set(CONCURRENT_QUEUE_MINOR_VER 0)
set(CONCURRENT_QUEUE_PATCH_VER 3)
set(CONCURRENT_QUEUE_URL_HASH  SHA256=eb37336bf9ae59aca7b954db3350d9b30d1cab24b96c7676f36040aa76e915e8)

set(CONCURRENT_QUEUE_VER     ${CONCURRENT_QUEUE_MAJOR_VER}.${CONCURRENT_QUEUE_MINOR_VER}.${CONCURRENT_QUEUE_PATCH_VER})
set(CONCURRENT_QUEUE_ROOT    ${3RDPARTY_PATH}/concurrent_queue)
set(CONCURRENT_QUEUE_INC_DIR ${CONCURRENT_QUEUE_ROOT}/src/concurrent_queue-${CONCURRENT_QUEUE_VER}/)
set(CONCURRENT_INSTALL       echo  "install concurrent queue")

set(CONCURRENT_QUEUE_URL https://github.com/cameron314/concurrentqueue/archive/refs/tags/v${CONCURRENT_QUEUE_VER}.tar.gz)

ExternalProject_Add(concurrent_queue-${CONCURRENT_QUEUE_VER}
    URL               ${CONCURRENT_QUEUE_URL}
    URL_HASH          ${CONCURRENT_QUEUE_URL_HASH} 
    DOWNLOAD_NAME     concurrent_queue-${CONCURRENT_QUEUE_VER}.tar.gz
    PREFIX            ${CONCURRENT_QUEUE_ROOT}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     ""
    INSTALL_COMMAND   ${CONCURRENT_INSTALL}
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} concurrent_queue-${CONCURRENT_QUEUE_VER})

if (NOT EXISTS ${CONCURRENT_QUEUE_ROOT}/src/concurrent_queue-${CONCURRENT_QUEUE_VER})
    add_custom_target(rescan-concurrent_queue ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS concurrent_queue-${CONCURRENT_QUEUE_VER})
else()
    add_custom_target(rescan-concurrent_queue)
endif()

