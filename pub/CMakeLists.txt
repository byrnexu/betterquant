cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

file(GLOB 3RDPARTY_LIST cmake/*.cmake)
foreach(3RDPARTY_LIB ${3RDPARTY_LIST})
    include (${3RDPARTY_LIB})
endforeach()

project(pub C CXX)

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

set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -fPIC")

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

set(EXECUTABLE_OUTPUT_PATH ${SOLUTION_ROOT_DIR}/bin)
set(LIBRARY_OUTPUT_PATH    ${SOLUTION_ROOT_DIR}/lib)

check_if_the_cmd_exists(clang-format)
get_proj_ver(${PROJ_VER} ${MAJOR_VER})

configure_file (
    "${PROJECT_SOURCE_DIR}/config.hpp.in"
    "${PROJECT_BINARY_DIR}/config-proj.hpp")

aux_source_directory(src/def SRC_LIST)
aux_source_directory(src/util SRC_LIST)
aux_source_directory(src/db SRC_LIST)
add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
target_link_options(${PROJECT_NAME} PUBLIC "LINKER:--copy-dt-needed-entries")

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
    set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "-d")
    set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJ_VER} SOVERSION ${MAJOR_VER})
else()
    message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
    set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJ_VER} SOVERSION ${MAJOR_VER})
endif()

add_dependencies(${PROJECT_NAME} ${3RDPARTY_DEPENDENCIES})
message(STATUS "3RDPARTY_DEPENDENCIES ${3RDPARTY_DEPENDENCIES}")

target_include_directories(${PROJECT_NAME}
    PUBLIC "${PROJECT_SOURCE_DIR}/inc"
    PUBLIC "${PROJECT_SOURCE_DIR}/src"
    PUBLIC "${PROJECT_BINARY_DIR}"
    PUBLIC "${ABSEIL_INC_DIR}"
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
    PUBLIC "${ABSEIL_LIB_DIR}"
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

target_link_libraries(${PROJECT_NAME}
    PUBLIC mysqlcppconn-static
    PUBLIC libmysqlclient.a
    PUBLIC libcpr.a
    PUBLIC libcurl.a
    PUBLIC libxxhash.a
    PUBLIC libabsl_flags_reflection.a
    PUBLIC libabsl_hash.a
    PUBLIC libabsl_raw_hash_set.a
    PUBLIC libabsl_city.a
    PUBLIC libabsl_low_level_hash.a
    PUBLIC libyyjson.a
    PUBLIC libyaml-cpp.a
    PUBLIC libfmt.a
    PUBLIC libgflags.a
    PUBLIC libmimalloc.a
    PUBLIC dl
    PUBLIC pthread
    )

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
