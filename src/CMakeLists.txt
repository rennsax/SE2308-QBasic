aux_source_directory(frontend qbasic_frontend_src)

qt_add_library(qbasic-frontend STATIC
    ${qbasic_frontend_src}
)

target_include_directories(qbasic-frontend
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/frontend
    INTERFACE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(qbasic-frontend
    PUBLIC
    Qt6::Widgets
    Qt6::Gui
)