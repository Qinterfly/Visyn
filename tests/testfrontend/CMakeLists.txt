
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Test)

qt_add_executable(testfrontend
    testfrontend.h
    testfrontend.cpp
    ${RESOURCES_FILE}
)

set_target_properties(testfrontend PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(testfrontend PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Test
    backend
    frontend
)

target_include_directories(testfrontend PUBLIC ${CMAKE_BINARY_DIR})
