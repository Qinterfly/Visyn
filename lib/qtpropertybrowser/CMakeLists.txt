
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(QTPROPERTYBROWSER_HEADERS
    qtbuttonpropertybrowser.h
    qteditorfactory.h
    qtgroupboxpropertybrowser.h
    qtpropertybrowser.h
    qtpropertymanager.h
    qttreepropertybrowser.h
    qtvariantproperty.h
    customdoublespinbox.h
    custompropertyeditor.h
)

set(QTPROPERTYBROWSER_SOURCES
    qtbuttonpropertybrowser.cpp
    qteditorfactory.cpp
    qtpropertymanager.cpp
    qtgroupboxpropertybrowser.cpp
    qtpropertybrowser.cpp
    qtpropertybrowserutils.cpp
    qttreepropertybrowser.cpp
    qtvariantproperty.cpp
    customdoublespinbox.cpp
    custompropertyeditor.cpp
)

qt_add_library(qtpropertybrowser STATIC
    ${QTPROPERTYBROWSER_HEADERS}
    ${QTPROPERTYBROWSER_SOURCES}
)

target_link_libraries(qtpropertybrowser PRIVATE
    Qt::Widgets
)

target_include_directories(qtpropertybrowser INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
