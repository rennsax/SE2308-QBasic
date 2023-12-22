set(APP_TARGET Qbasic-Interpreter)

add_executable(${APP_TARGET}
    qbasic-app.cpp
)

target_link_libraries(${APP_TARGET}
    PRIVATE
    qbasic-frontend
)

set_target_properties(${APP_TARGET} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.sjtu.rbj.qbasic"
)