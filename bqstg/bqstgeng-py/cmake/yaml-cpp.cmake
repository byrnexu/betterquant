include(ExternalProject)
include(cmake/config.cmake)

set(YAMLCPP_MAJOR_VER 0)
set(YAMLCPP_MINOR_VER 7)
set(YAMLCPP_PATCH_VER 0)
set(YAMLCPP_URL_HASH  SHA256=43e6a9fcb146ad871515f0d0873947e5d497a1c9c60c58cb102a97b47208b7c3)

set(YAMLCPP_VER       ${YAMLCPP_MAJOR_VER}.${YAMLCPP_MINOR_VER}.${YAMLCPP_PATCH_VER})
set(YAMLCPP_ROOT      ${3RDPARTY_PATH}/yaml-cpp)
set(YAMLCPP_INC_DIR   ${YAMLCPP_ROOT}/src/yaml-cpp-${YAMLCPP_VER}/include)
set(YAMLCPP_LIB_DIR   ${YAMLCPP_ROOT}/src/yaml-cpp-${YAMLCPP_VER}/build)

set(YAMLCPP_URL           https://github.com/jbeder/yaml-cpp/archive/refs/tags/yaml-cpp-${YAMLCPP_VER}.tar.gz)
set(YAMLCPP_CONFIGURE     cd ${YAMLCPP_ROOT}/src/yaml-cpp-${YAMLCPP_VER} && mkdir -p build && cd build && cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON)
set(YAMLCPP_BUILD         cd ${YAMLCPP_ROOT}/src/yaml-cpp-${YAMLCPP_VER} && cd build && make CXXFLAGS+='-fPIC')
set(YAMLCPP_INSTALL       cd ${YAMLCPP_ROOT}/src/yaml-cpp-${YAMLCPP_VER} && cd build && make install)

ExternalProject_Add(yaml-cpp-${YAMLCPP_VER}
    URL                   ${YAMLCPP_URL}
    DOWNLOAD_NAME         yaml-cpp-${YAMLCPP_VER}.tar.gz
    URL_HASH              ${YAMLCPP_URL_HASH} 
    PREFIX                ${YAMLCPP_ROOT}
    CONFIGURE_COMMAND     ${YAMLCPP_CONFIGURE}
    BUILD_COMMAND         ${YAMLCPP_BUILD}
    INSTALL_COMMAND       ${YAMLCPP_INSTALL}
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} yaml-cpp-${YAMLCPP_VER})

if (NOT EXISTS ${YAMLCPP_ROOT}/src/yaml-cpp-${YAMLCPP_VER})
    add_custom_target(rescan-yaml-cpp ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS yaml-cpp-${YAMLCPP_VER})
else()
    add_custom_target(rescan-yaml-cpp)
endif()

