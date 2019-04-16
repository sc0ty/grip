find_package(Git)
if(GIT_FOUND)
    execute_process (
        COMMAND ${GIT_EXECUTABLE} describe --tags
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
        RESULT_VARIABLE GIT_DESCRIBE_RESULT
        ERROR_VARIABLE GIT_DESCRIBE_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(${GIT_DESCRIBE_RESULT} EQUAL 0)
        add_definitions (-DVERSION=${GIT_DESCRIBE_VERSION})
    endif()
endif()

set (CMAKE_CXX_STANDARD 11)
add_subdirectory (external)
add_subdirectory (general)
add_subdirectory (grip)
add_subdirectory (gripgen)
add_subdirectory (test)
