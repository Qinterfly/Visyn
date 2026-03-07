
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets PrintSupport)

qt_add_library(qcustomplot STATIC
    qcustomplot.h
    qcustomplot.cpp
)

target_link_libraries(qcustomplot PRIVATE
    Qt::Widgets
    Qt::PrintSupport
    qtpropertybrowser
)

target_include_directories(qcustomplot INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
