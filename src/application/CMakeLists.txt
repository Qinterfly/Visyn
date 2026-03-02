
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(application
    main.cpp
    ${RESOURCES_FILE}
)

set_target_properties(application PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    OUTPUT_NAME ${APP_NAME}
)

target_link_libraries(application PRIVATE
    Qt::Widgets
    backend
    frontend
)

target_include_directories(application PUBLIC ${CMAKE_BINARY_DIR})

install(TARGETS application
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

