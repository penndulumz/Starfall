# Application-seam checks use real staged resources and disposable package copies.
file(MAKE_DIRECTORY "${QA_DIR}/unrelated")
execute_process(COMMAND "${APP_TEST}" "${QA_DIR}" "${LEGACY_FIXTURE}"
    WORKING_DIRECTORY "${QA_DIR}/unrelated" RESULT_VARIABLE flow_result)
if(NOT flow_result EQUAL 0)
    message(FATAL_ERROR "Application screen-flow checks failed (${flow_result}).")
endif()

set(package "${QA_DIR}/resource-package")
file(MAKE_DIRECTORY "${package}")
file(GLOB runtime_dlls "${RUNTIME_DIR}/*.dll")
file(COPY "${APP_TEST}" ${runtime_dlls} "${RUNTIME_DIR}/resources" DESTINATION "${package}")
get_filename_component(test_name "${APP_TEST}" NAME)

function(expect_resource_failure relative_path corrupt)
    set(resource "${package}/resources/reference/${relative_path}")
    if(corrupt)
        file(WRITE "${resource}" "This is not a valid packaged image or font.\n")
    else()
        file(REMOVE "${resource}")
    endif()
    get_filename_component(expected "${relative_path}" NAME)
    execute_process(COMMAND "${package}/${test_name}" --expect-resource-failure "${expected}"
        WORKING_DIRECTORY "${QA_DIR}/unrelated" RESULT_VARIABLE failure_result)
    configure_file("${RUNTIME_DIR}/resources/reference/${relative_path}" "${resource}" COPYONLY)
    if(NOT failure_result EQUAL 0)
        message(FATAL_ERROR "Missing/corrupt ${expected} did not produce its useful startup diagnostic.")
    endif()
endfunction()

expect_resource_failure("spaceShip/stitchedFiles/spaceships.png" FALSE)
expect_resource_failure("spaceShip/stitchedFiles/spaceships.png" TRUE)
expect_resource_failure("CommodorePixeled.ttf" FALSE)
expect_resource_failure("CommodorePixeled.ttf" TRUE)
