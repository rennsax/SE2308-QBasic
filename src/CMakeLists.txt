add_subdirectory(backend)

# Qt6 should be installed appropriately by the user.
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
# Check the doc: https://doc.qt.io/Qt-6/qt-standard-project-setup.html#description
qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

aux_source_directory(frontend QBASIC_FRONTEND_SRC)
set(QBASIC_FRONTEND_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/frontend)

qt_add_library(qbasic-frontend STATIC
    ${QBASIC_FRONTEND_SRC}
    ${QBASIC_FRONTEND_INCLUDE_DIR}/qbasic.qrc
)

target_include_directories(qbasic-frontend
    PUBLIC
    ${QBASIC_FRONTEND_INCLUDE_DIR}
    INTERFACE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(qbasic-frontend
    PUBLIC
    Qt6::Widgets
    Qt6::Gui
    qbasic-backend
)

set_target_properties(qbasic-frontend PROPERTIES
    AUTOUIC_SEARCH_PATHS ${QBASIC_FRONTEND_INCLUDE_DIR}/ui
)