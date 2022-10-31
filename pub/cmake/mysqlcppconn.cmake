include(ExternalProject)
include(cmake/config.cmake)

set(MYSQLCPPCONN_MAJOR_VER 1)
set(MYSQLCPPCONN_MINOR_VER 1)
set(MYSQLCPPCONN_PATCH_VER 13)
set(MYSQLCPPCONN_URL_HASH  SHA256=ff9c3274b0c1340750b318457dc3d870b0cc917374f7ddce90d6566f276276c5)

set(MYSQLCPPCONN_VER       ${MYSQLCPPCONN_MAJOR_VER}.${MYSQLCPPCONN_MINOR_VER}.${MYSQLCPPCONN_PATCH_VER})
set(MYSQLCPPCONN_ROOT      ${3RDPARTY_PATH}/mysqlcppconn)
set(MYSQLCPPCONN_INC_DIR   /user/local/include/)
set(MYSQLCPPCONN_LIB_DIR   /user/local/lib/)

set(MYSQLCPPCONN_URL       https://github.com/mysql/mysql-connector-cpp/archive/${MYSQLCPPCONN_VER}.tar.gz)
set(MYSQLCPPCONN_CONFIGURE cd ${MYSQLCPPCONN_ROOT}/src/mysqlcppconn-${MYSQLCPPCONN_VER} && mkdir -p build && cd build && cmake .. -DMYSQLCLIENT_STATIC_BINDING:BOOL=1)
set(MYSQLCPPCONN_BUILD     cd ${MYSQLCPPCONN_ROOT}/src/mysqlcppconn-${MYSQLCPPCONN_VER} && cd build && make && make install)
set(MYSQLCPPCONN_INSTALL   echo "install mysqlcppconn")

ExternalProject_Add(mysqlcppconn-${MYSQLCPPCONN_VER}
    URL               ${MYSQLCPPCONN_URL}
    URL_HASH          ${MYSQLCPPCONN_URL_HASH} 
    DOWNLOAD_NAME     mysqlcppconn-${MYSQLCPPCONN_VER}.tar.gz
    PREFIX            ${MYSQLCPPCONN_ROOT}
    CONFIGURE_COMMAND ${MYSQLCPPCONN_CONFIGURE}
    BUILD_COMMAND     ${MYSQLCPPCONN_BUILD}
    INSTALL_COMMAND   ${MYSQLCPPCONN_INSTALL}
    )

set(3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES} mysqlcppconn-${MYSQLCPPCONN_VER})

if (NOT EXISTS ${MYSQLCPPCONN_ROOT}/src/mysqlcppconn-${MYSQLCPPCONN_VER})
    add_custom_target(rescan-mysqlcppconn ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS mysqlcppconn-${MYSQLCPPCONN_VER})
else()
    add_custom_target(rescan-mysqlcppconn)
endif()

add_dependencies(mysqlcppconn-${MYSQLCPPCONN_VER} boost-${BOOST_VER})
