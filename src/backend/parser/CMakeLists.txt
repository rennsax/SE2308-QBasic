if(NOT ${ANTLR_FOUND})
    message(FATAL_ERROR "Cannot find antlr!")
endif()

set(BASIC_BACKEND_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/backend)

antlr_target(BasicParser
    Basic.g4
    PARSER LEXER # Mixed lexer and parser
    PACKAGE antlr_basic
)

aux_source_directory(. BASIC_BACKEND_PARSER_SRC)

add_library(basic-parser STATIC
    ${BASIC_BACKEND_PARSER_SRC}
    ${ANTLR_BasicParser_CXX_OUTPUTS}
)

target_include_directories(basic-parser
    PRIVATE
    ${ANTLR_BasicParser_OUTPUT_DIR}
    PUBLIC
    ${BASIC_BACKEND_INCLUDE_DIR}/parser
)

target_link_libraries(basic-parser
    antlr4_static
)