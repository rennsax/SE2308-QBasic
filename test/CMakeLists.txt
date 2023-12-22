add_executable(test_parse
    test_parse.cpp
)

target_link_libraries(test_parse
    basic-parser
    doctest
)

add_executable(test_editor
    test_editor.cpp
)

target_link_libraries(test_editor
    qbasic-backend
    doctest
)