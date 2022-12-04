include(ExternalProject)
include(cmake/config.cmake)

set(BOOST_MAJOR_VER 1)
set(BOOST_MINOR_VER 72)
set(BOOST_PATCH_VER 0)
set(BOOST_URL_HASH  SHA256=59c9b274bc451cf91a9ba1dd2c7fdcaf5d60b1b3aa83f2c9fa143417cc660722)

set(BOOST_VER           ${BOOST_MAJOR_VER}.${BOOST_MINOR_VER}.${BOOST_PATCH_VER})
set(BOOST_DOWNLOAD_NAME boost_${BOOST_MAJOR_VER}_${BOOST_MINOR_VER}_${BOOST_PATCH_VER}.tar.bz2)

set(BOOST_ROOT      ${3RDPARTY_PATH}/boost)
set(BOOST_INC_DIR   /usr/local/include)
set(BOOST_LIB_DIR   /usr/local/lib)

set(BOOST_URL           https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VER}/source/${BOOST_DOWNLOAD_NAME})
set(BOOST_CONFIGURE     cd ${BOOST_ROOT}/src/boost-${BOOST_VER} && ./bootstrap.sh)
set(BOOST_BUILD         cd ${BOOST_ROOT}/src/boost-${BOOST_VER} && ./b2 variant=release link=static threading=multi runtime-link=shared address-model=64 cxxflags=-fPIC --without-python install -j4)
set(BOOST_INSTALL       echo "install boost")

ExternalProject_Add(boost-${BOOST_VER}
    URL                   ${BOOST_URL}
    URL_HASH              ${BOOST_URL_HASH} 
    DOWNLOAD_NAME         ${BOOST_DOWNLOAD_NAME}
    PREFIX                ${BOOST_ROOT}
    CONFIGURE_COMMAND     ${BOOST_CONFIGURE}
    BUILD_COMMAND         ${BOOST_BUILD}
    INSTALL_COMMAND       ${BOOST_INSTALL}
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} boost-${BOOST_VER})

if (NOT EXISTS ${BOOST_ROOT}/src/boost-${BOOST_VER})
    add_custom_target(rescan-boost ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS boost-${BOOST_VER})
else()
    add_custom_target(rescan-boost)
endif()

