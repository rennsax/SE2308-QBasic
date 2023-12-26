add_executable(test_editor
    test_editor.cpp
)

target_link_libraries(test_editor
    qbasic-backend
    doctest
)

add_executable(test_interpreter
    test_interpreter.cpp
)

target_link_libraries(test_interpreter
    qbasic-backend
    doctest
)