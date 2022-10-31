cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

file(GLOB 3RDPARTY_LIST cmake/*.cmake)
foreach(3RDPARTY_LIB ${3RDPARTY_LIST})
    include (${3RDPARTY_LIB})
endforeach()

project(bqstgeng-cxx-demo C CXX)

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
target_link_options(${PROJECT_NAME} PUBLIC "LINKER:--copy-dt-needed-entries")

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
    PUBLIC "${SOLUTION_ROOT_DIR}/inc/cxx/bqstg/bqstgeng-cxx/inc"
    PUBLIC "${SOLUTION_ROOT_DIR}/inc/cxx/bqpub/inc"
    PUBLIC "${SOLUTION_ROOT_DIR}/inc/cxx/bqipc/inc"
    PUBLIC "${SOLUTION_ROOT_DIR}/inc/cxx/pub/inc"
    PUBLIC "${PROJECT_SOURCE_DIR}/inc"
    PUBLIC "${PROJECT_SOURCE_DIR}/src"
    PUBLIC "${PROJECT_BINARY_DIR}"
    PUBLIC "${MYSQLCPPCONN_INC_DIR}"
    PUBLIC "${SPDLOG_INC_DIR}"
    PUBLIC "${FMT_INC_DIR}"
    PUBLIC "${GFLAGS_INC_DIR}"
    PUBLIC "${MIMALLOC_INC_DIR}"
    )

target_link_directories(${PROJECT_NAME}
    PUBLIC "${SOLUTION_ROOT_DIR}/lib/"
    PUBLIC "${MYSQLCPPCONN_LIB_DIR}"
   )

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(${PROJECT_NAME}
      PUBLIC bqstgeng-cxx-d
      )
else()
    target_link_libraries(${PROJECT_NAME}
      PUBLIC bqstgeng-cxx
      )
endif()

target_link_libraries(${PROJECT_NAME}
    PUBLIC mysqlcppconn-static
    PUBLIC libmysqlclient.a
    PUBLIC crypto
    PUBLIC ssl
    PUBLIC dl
    PUBLIC pthread
    PUBLIC rt
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

execute_process(COMMAND bash -c "mkdir -p ${SOLUTION_ROOT_DIR}/bin/config/${PROJECT_NAME} \
  && rsync -aPc --delete ${PROJECT_SOURCE_DIR}/config/ ${SOLUTION_ROOT_DIR}/bin/config/${PROJECT_NAME}")
