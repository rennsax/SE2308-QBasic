cmake_minimum_required(VERSION 3.24)

project(QBasic VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

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
include_directories(${ANTLR4_INCLUDE_DIRS})

set(ANTLR_EXECUTABLE ${antlr4_SOURCE_DIR}/${antlr_JAR_NAME})
find_package(ANTLR REQUIRED)

add_subdirectory(src)
add_subdirectory(app)