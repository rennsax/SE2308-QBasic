# Begin: configure ANTLR
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/lib/antlr/cmake ${CMAKE_MODULE_PATH})

if(MSVC)
    # if true ANTLR4 C++ runtime library should be compiled with /MT flag
    # otherwise will be compiled with /MD flag.
    set(ANTLR4_WITH_STATIC_CRT OFF)
endif(MSVC)

include(FetchContent)
set(antlr_JAR_NAME "antlr-4.13.1-complete.jar")
FetchContent_Declare(
    antlr4
    URL "https://www.antlr.org/download/${antlr_JAR_NAME}"
    DOWNLOAD_NO_EXTRACT TRUE
    URL_MD5 78af96af276609af0bfb3f1e2bfaef89
)
FetchContent_MakeAvailable(antlr4)

include(ExternalAntlr4Cpp)

set(ANTLR_EXECUTABLE ${antlr4_SOURCE_DIR}/${antlr_JAR_NAME})
# ENd: configure ANTLR

find_package(ANTLR REQUIRED)

antlr_target(BasicParser
    Basic.g4
    PARSER LEXER # Mixed lexer and parser
    VISITOR # Also generate a visitor interface
    PACKAGE antlr_basic
)

aux_source_directory(. BASIC_BACKEND_PARSER_SRC)

add_library(basic-parser STATIC
    ${BASIC_BACKEND_PARSER_SRC}
    ${ANTLR_BasicParser_CXX_OUTPUTS}
)

target_include_directories(basic-parser
    PUBLIC
    ${ANTLR_BasicParser_OUTPUT_DIR}
    ${ANTLR4_INCLUDE_DIRS}
)

target_link_libraries(basic-parser
    antlr4_static
)