aux_source_directory(. BENCH_SRC_LIST)
set(BENCH_SRC_LIST ${BENCH_SRC_LIST})
add_executable(${BENCH_PROJECT_NAME} ${BENCH_SRC_LIST})

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    set_target_properties(${BENCH_PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "-d-${PROJ_VER}")
    add_custom_target(link_bench_target ALL
        COMMAND ${CMAKE_COMMAND} -E create_symlink "${BENCH_PROJECT_NAME}-d-${PROJ_VER}" ${EXECUTABLE_OUTPUT_PATH}/${BENCH_PROJECT_NAME}-d)
else()
    set_target_properties(${BENCH_PROJECT_NAME} PROPERTIES RELEASE_POSTFIX "-${PROJ_VER}")
    add_custom_target(link_bench_target ALL
        COMMAND ${CMAKE_COMMAND} -E create_symlink "${BENCH_PROJECT_NAME}-${PROJ_VER}" ${EXECUTABLE_OUTPUT_PATH}/${BENCH_PROJECT_NAME})
endif()

target_include_directories(${BENCH_PROJECT_NAME}
    PUBLIC "${PROJECT_SOURCE_DIR}/inc"
    PUBLIC "${PROJECT_SOURCE_DIR}/src"
    PUBLIC "${BENCHMARK_INC_DIR}"
    )

target_link_directories(${BENCH_PROJECT_NAME}
    PUBLIC "${SOLUTION_ROOT_DIR}/lib"
    PUBLIC "${FMT_LIB_DIR}"
    PUBLIC "${BENCHMARK_LIB_DIR}"
    )

target_link_libraries(${BENCH_PROJECT_NAME}
    libfmt.a
    libbenchmark.a
    dl
    pthread
    )
