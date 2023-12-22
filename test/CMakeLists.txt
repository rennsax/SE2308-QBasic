add_executable(test_parse
    test_parse.cpp
)

target_link_libraries(test_parse
    basic-parser
    doctest
)