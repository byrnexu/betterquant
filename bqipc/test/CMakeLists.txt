aux_source_directory(. TEST_SRC_LIST)
set(TEST_SRC_LIST ${TEST_SRC_LIST})
add_executable(${TEST_PROJECT_NAME} ${TEST_SRC_LIST})

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    set_target_properties(${TEST_PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "-d-${PROJ_VER}")
    add_custom_target(link_test_target ALL
        COMMAND ${CMAKE_COMMAND} -E create_symlink "${TEST_PROJECT_NAME}-d-${PROJ_VER}" ${EXECUTABLE_OUTPUT_PATH}/${TEST_PROJECT_NAME}-d)
else()
    set_target_properties(${TEST_PROJECT_NAME} PROPERTIES RELEASE_POSTFIX "-${PROJ_VER}")
    add_custom_target(link_test_target ALL
        COMMAND ${CMAKE_COMMAND} -E create_symlink "${TEST_PROJECT_NAME}-${PROJ_VER}" ${EXECUTABLE_OUTPUT_PATH}/${TEST_PROJECT_NAME})
endif()

target_include_directories(${TEST_PROJECT_NAME}
    PUBLIC "${SOLUTION_ROOT_DIR}/bqipc/inc"
    PUBLIC "${SOLUTION_ROOT_DIR}/pub/inc"
    PUBLIC "${PROJECT_SOURCE_DIR}/inc"
    PUBLIC "${PROJECT_SOURCE_DIR}/src"
    PUBLIC "${ICEORYX_INC_DIR}"
    PUBLIC "${MYSQLCPPCONN_INC_DIR}"
    PUBLIC "${YYJSON_INC_DIR}"
    PUBLIC "${RAPIDJSON_INC_DIR}"
    PUBLIC "${NLOHMANN_JSON_INC_DIR}"
    PUBLIC "${CPR_INC_DIR}"
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
    PUBLIC "${GTEST_INC_DIR}"
    )

target_link_directories(${TEST_PROJECT_NAME}
    PUBLIC "${SOLUTION_ROOT_DIR}/lib/"
    PUBLIC "${ICEORYX_LIB_DIR}"
    PUBLIC "${MYSQLCPPCONN_LIB_DIR}"
    PUBLIC "${YYJSON_LIB_DIR}"
    PUBLIC "${NLOHMANN_JSON_LIB_DIR}"
    PUBLIC "${CPR_LIB_DIR}"
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
    PUBLIC "${GTEST_LIB_DIR}"
    )

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(${TEST_PROJECT_NAME}
      bqipc-d
      pub-d
      )
else()
    target_link_libraries(${TEST_PROJECT_NAME}
      bqipc
      pub
      )
endif()

target_link_libraries(${TEST_PROJECT_NAME}
    iceoryx_posh
    iceoryx_hoofs
    iceoryx_platform
    iceoryx_posh_config
    iceoryx_binding_c
    iceoryx_posh_gateway
    iceoryx_posh_roudi
    libboost_date_time.a
    libyyjson.a
    libfmt.a
    libgflags.a
    libgtest.a
    libgmock.a
    dl
    crypto
    ssl
    pthread
    rt
    )
