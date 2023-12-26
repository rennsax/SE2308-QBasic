add_executable(test_editor
    test_editor.cpp
)

target_link_libraries(test_editor
    qbasic-backend
    doctest
)

add_custom_command(
    TARGET test_editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/test_cases
        ${CMAKE_CURRENT_BINARY_DIR}/test_cases
    COMMENT "Copying test cases"
)

add_executable(test_interpreter
    test_interpreter.cpp
)

target_link_libraries(test_interpreter
    qbasic-backend
    doctest
)