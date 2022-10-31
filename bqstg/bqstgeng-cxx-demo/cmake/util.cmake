function(get_proj_ver ${PROJ_VER})
    execute_process(
        COMMAND bash -c "git describe --tags --always --dirty | tr -d '\r\n';"
        OUTPUT_VARIABLE PROJ_VER)
    set(PROJ_VER ${PROJ_VER} PARENT_SCOPE)
    message(STATUS "Get project version ${PROJ_VER}")

    execute_process(
        COMMAND bash -c "git describe --tags --always --dirty | sed 's/v//g' | awk -F'.' '{print $1}' | tr -d '\r\n';"
        OUTPUT_VARIABLE MAJOR_VER)
    set(MAJOR_VER ${MAJOR_VER} PARENT_SCOPE)
    message(STATUS "Get major version ${MAJOR_VER}")

    execute_process(
        COMMAND bash -c "git describe --tags --always --dirty | sed 's/v//g' | awk -F'.' '{print $2}' | tr -d '\r\n';"
        OUTPUT_VARIABLE MINOR_VER)
    set(MINOR_VER ${MINOR_VER} PARENT_SCOPE)
    message(STATUS "Get minor version ${MINOR_VER}")

    execute_process(
        COMMAND bash -c "git describe --tags --always --dirty | sed 's/v//g' | awk -F'.' '{print $3}' | tr -d '\r\n';"
        OUTPUT_VARIABLE PATCH_VER)
    set(PATCH_VER ${PATCH_VER} PARENT_SCOPE)
    message(STATUS "Get patch version ${PATCH_VER}")
endfunction()

function(check_if_the_cmd_exists ${CMD})
    execute_process(COMMAND bash -c "type ${CMD}" RESULT_VARIABLE CMD_CHECK_RESULT)
    if (NOT ${CMD_CHECK_RESULT} EQUAL 0)
        message(FATAL_ERROR "Please install ${cmd} first")
    endif()
endfunction()
