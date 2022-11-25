cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

file(GLOB 3RDPARTY_LIST cmake/*.cmake)
foreach(3RDPARTY_LIB ${3RDPARTY_LIST})
    include (${3RDPARTY_LIB})
endforeach()

project(bqweb-srv C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CXX_FLAGS
    -g
    -Wextra
    -Wno-unused-parameter
    -march=native
    )
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_DEBUG   "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -flto")

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

set(EXECUTABLE_OUTPUT_PATH ${SOLUTION_ROOT_DIR}/bin)
set(LIBRARY_OUTPUT_PATH    ${SOLUTION_ROOT_DIR}/lib)

check_if_the_cmd_exists(clang-format)
get_proj_ver(${PROJ_VER})

configure_file (
    "${PROJECT_SOURCE_DIR}/config.hpp.in"
    "${PROJECT_BINARY_DIR}/config-proj.hpp")

aux_source_directory(src SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
    set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "-d-${PROJ_VER}")
    add_custom_target(link_target ALL
        COMMAND ${CMAKE_COMMAND} -E create_symlink "${PROJECT_NAME}-d-${PROJ_VER}" ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}-d)
else()
    message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
    set_target_properties(${PROJECT_NAME} PROPERTIES RELEASE_POSTFIX "-${PROJ_VER}")
    add_custom_target(link_target ALL
        COMMAND ${CMAKE_COMMAND} -E create_symlink "${PROJECT_NAME}-${PROJ_VER}" ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME})
endif()

add_dependencies(${PROJECT_NAME} ${3RDPARTY_DEPENDENCIES})
message(STATUS "3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES}")

target_include_directories(${PROJECT_NAME}
    PUBLIC /usr/include/jsoncpp/
    PUBLIC "${SOLUTION_ROOT_DIR}/bqassetsmgr/inc"
    PUBLIC "${SOLUTION_ROOT_DIR}/bqposmgr/inc"
    PUBLIC "${SOLUTION_ROOT_DIR}/bqordmgr/inc"
    PUBLIC "${SOLUTION_ROOT_DIR}/bqweb/inc"
    PUBLIC "${SOLUTION_ROOT_DIR}/bqpub/inc"
    PUBLIC "${SOLUTION_ROOT_DIR}/bqipc/inc"
    PUBLIC "${SOLUTION_ROOT_DIR}/pub/inc"
    PUBLIC "${PROJECT_SOURCE_DIR}/inc"
    PUBLIC "${PROJECT_SOURCE_DIR}/src"
    PUBLIC "${PROJECT_BINARY_DIR}"
    PUBLIC "${ICEORYX_INC_DIR}"
    PUBLIC "${MYSQLCPPCONN_INC_DIR}"
    PUBLIC "${YYJSON_INC_DIR}"
    PUBLIC "${RAPIDJSON_INC_DIR}"
    PUBLIC "${NLOHMANN_JSON_INC_DIR}"
    PUBLIC "${CPR_INC_DIR}"
    PUBLIC "${CURL_INC_DIR}"
    PUBLIC "${YAMLCPP_INC_DIR}"
    PUBLIC "${WEBSOCKETPP_INC_DIR}"
    PUBLIC "${SPDLOG_INC_DIR}"
    PUBLIC "${BOOST_INC_DIR}"
    PUBLIC "${READERWRITER_QUEUE_INC_DIR}"
    PUBLIC "${CONCURRENT_QUEUE_INC_DIR}"
    PUBLIC "${GFLAGS_INC_DIR}"
    PUBLIC "${MAGIC_ENUM_INC_DIR}"
    PUBLIC "${FMT_INC_DIR}"
    PUBLIC "${XXHASH_INC_DIR}"
    PUBLIC "${MIMALLOC_INC_DIR}"
    )

target_link_directories(${PROJECT_NAME}
    PUBLIC "${SOLUTION_ROOT_DIR}/lib/"
    PUBLIC "${ICEORYX_LIB_DIR}"
    PUBLIC "${MYSQLCPPCONN_LIB_DIR}"
    PUBLIC "${YYJSON_LIB_DIR}"
    PUBLIC "${NLOHMANN_JSON_LIB_DIR}"
    PUBLIC "${CPR_LIB_DIR}"
    PUBLIC "${CURL_LIB_DIR}"
    PUBLIC "${YAMLCPP_LIB_DIR}"
    PUBLIC "${WEBSOCKETPP_LIB_DIR}"
    PUBLIC "${SPDLOG_LIB_DIR}"
    PUBLIC "${BOOST_LIB_DIR}"
    PUBLIC "${READERWRITER_QUEUE_LIB_DIR}"
    PUBLIC "${GFLAGS_LIB_DIR}"
    PUBLIC "${MAGIC_ENUM_LIB_DIR}"
    PUBLIC "${FMT_LIB_DIR}"
    PUBLIC "${XXHASH_LIB_DIR}"
    PUBLIC "${MIMALLOC_LIB_DIR}"
   )

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(${PROJECT_NAME}
      bqposmgr-d
      bqassetsmgr-d
      bqordmgr-d
      bqweb-d
      bqpub-d
      bqipc-d
      pub-d
    )
else()
    target_link_libraries(${PROJECT_NAME}
      bqposmgr
      bqassetsmgr
      bqordmgr
      bqweb
      bqpub
      bqipc
      pub
    )
endif()

target_link_libraries(${PROJECT_NAME}
    libcpr.a
    libcurl.a
    drogon
    libxxhash.a
    iceoryx_posh
    iceoryx_hoofs
    iceoryx_platform
    iceoryx_posh_config
    iceoryx_binding_c
    iceoryx_posh_gateway
    iceoryx_posh_roudi
    libabsl_raw_hash_set.a
    libabsl_flags_reflection.a
    libabsl_hash.a
    libabsl_city.a
    libabsl_low_level_hash.a
    libboost_date_time.a
    libboost_filesystem.a
    libyyjson.a
    libyaml-cpp.a
    libfmt.a
    libgflags.a
    mysqlcppconn-static
    libmysqlclient.a
    libmimalloc.a
    crypto
    ssl
    dl
    pthread
    rt
    )

add_subdirectory(drogon)

option(BUILD_TESTS "Build the tests" ON)
if (BUILD_TESTS)
    set(TEST_PROJECT_NAME ${PROJECT_NAME}-test)
    message(STATUS "Start building test cases.")
    enable_testing()
    add_test(NAME test COMMAND ${TEST_PROJECT_NAME} WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test)
    add_subdirectory(test)
    if(${CMAKE_BUILD_TYPE} MATCHES Debug)
        add_custom_target(tests COMMAND ${EXECUTABLE_OUTPUT_PATH}/${TEST_PROJECT_NAME}-d)
    else()
        add_custom_target(tests COMMAND ${EXECUTABLE_OUTPUT_PATH}/${TEST_PROJECT_NAME})
    endif()
endif()

option(BUILD_BENCH "Build the bench" ON)
if (BUILD_BENCH)
    set(BENCH_PROJECT_NAME ${PROJECT_NAME}-bench)
    message(STATUS "Start building benches.")
    add_subdirectory(bench)
    if(${CMAKE_BUILD_TYPE} MATCHES Debug)
        add_custom_target(bench COMMAND ${EXECUTABLE_OUTPUT_PATH}/${BENCH_PROJECT_NAME}-d)
    else()
        add_custom_target(bench COMMAND ${EXECUTABLE_OUTPUT_PATH}/${BENCH_PROJECT_NAME})
    endif()
endif()

execute_process(COMMAND bash -c "mkdir -p ${SOLUTION_ROOT_DIR}/bin/config/${PROJECT_NAME} \
  && rsync -aPc --delete ${PROJECT_SOURCE_DIR}/config/ ${SOLUTION_ROOT_DIR}/bin/config/${PROJECT_NAME}")
