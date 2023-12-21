cmake_minimum_required(VERSION 3.24)

project(QBasic VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
# Check the doc: https://doc.qt.io/Qt-6/qt-standard-project-setup.html#description
qt_standard_project_setup()

add_subdirectory(src)
add_subdirectory(app)