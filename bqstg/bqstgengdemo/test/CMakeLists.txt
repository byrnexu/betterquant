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
    PUBLIC "${PROJECT_SOURCE_DIR}/inc"
    PUBLIC "${PROJECT_SOURCE_DIR}/src"
    PUBLIC "${GTEST_INC_DIR}"
    )

target_link_directories(${TEST_PROJECT_NAME}
    PUBLIC "${SOLUTION_ROOT_DIR}/lib"
    PUBLIC "${GTEST_LIB_DIR}"
    )

target_link_libraries(${TEST_PROJECT_NAME}
    libgtest.a
    libgmock.a
    dl
    pthread
    )
