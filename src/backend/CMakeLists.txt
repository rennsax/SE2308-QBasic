# libbasic-parser
add_subdirectory(parser)

set(QBASIC_BACKEND_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/backend)

aux_source_directory(. QBASIC_BACKEND_SRC)
message("${QBASIC_BACKEND_SRC}")

add_library(qbasic-backend STATIC
    ${QBASIC_BACKEND_SRC}
)

target_link_libraries(qbasic-backend
    PRIVATE
    basic-parser
)

target_include_directories(qbasic-backend
    PUBLIC
    ${QBASIC_BACKEND_INCLUDE_DIR}
)
