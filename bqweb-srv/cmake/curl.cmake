include(ExternalProject)
include(cmake/config.cmake)

set(CURL_MAJOR_VER 7)
set(CURL_MINOR_VER 79)
set(CURL_PATCH_VER 1)
set(CURL_URL_HASH  SHA256=370b11201349816287fb0ccc995e420277fbfcaf76206e309b3f60f0eda090c2)

set(CURL_VER       ${CURL_MAJOR_VER}.${CURL_MINOR_VER}.${CURL_PATCH_VER})
set(CURL_ROOT      ${3RDPARTY_PATH}/curl)
set(CURL_INC_DIR   /usr/local/include)
set(CURL_LIB_DIR   /usr/local/lib)

# wget https://curl.haxx.se/download/curl-7.65.3.tar.gz
set(CURL_URL           https://github.com/curl/curl/releases/download/curl-${CURL_MAJOR_VER}_${CURL_MINOR_VER}_${CURL_PATCH_VER}/curl-${CURL_VER}.tar.gz)
set(CURL_CONFIGURE     cd ${CURL_ROOT}/src/curl-${CURL_VER} && ./configure --with-openssl --disable-shared)
set(CURL_BUILD         cd ${CURL_ROOT}/src/curl-${CURL_VER} && make CXXFLAGS+='-fPIC')
set(CURL_INSTALL       cd ${CURL_ROOT}/src/curl-${CURL_VER} && make install)

ExternalProject_Add(curl-${CURL_VER}
    URL                    ${CURL_URL}
    URL_HASH               ${CURL_URL_HASH} 
    DOWNLOAD_NAME          curl-${CURL_VER}.tar.gz
    PREFIX                 ${CURL_ROOT}
    CONFIGURE_COMMAND      ${CURL_CONFIGURE}
    BUILD_COMMAND          ${CURL_BUILD}
    INSTALL_COMMAND        ${CURL_INSTALL}
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} curl-${CURL_VER})

if (NOT EXISTS ${CURL_ROOT}/src/curl-${CURL_VER})
    add_custom_target(rescan-curl ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS curl-${CURL_VER})
else()
    add_custom_target(rescan-curl)
endif()

