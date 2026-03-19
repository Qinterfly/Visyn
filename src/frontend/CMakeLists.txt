
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(magic_enum CONFIG REQUIRED)

set(FRONTEND_HEADERS
    mainwindow.h
    uialiasdata.h
    uiconstants.h
    uiutility.h
    customtable.h
    customstatusbar.h
    customtabwidget.h
    customplot.h
    customlineedit.h
    viewmanager.h
    projecteditor.h
    setupeditor.h
)

set(FRONTEND_SOURCES
    mainwindow.cpp
    uiutility.cpp
    customtable.cpp
    customstatusbar.cpp
    customtabwidget.cpp
    customplot.cpp
    customlineedit.cpp
    viewmanager.cpp
    projecteditor.cpp
    setupeditor.cpp
)

qt_add_library(frontend STATIC
    ${FRONTEND_HEADERS}
    ${FRONTEND_SOURCES}
    ${RESOURCES_FILE}
)

target_link_libraries(frontend PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    magic_enum::magic_enum
    ${TESTLAB_LIB}
    qtpropertybrowser
    qcustomplot
    backend
)

target_include_directories(frontend PUBLIC ${CMAKE_BINARY_DIR})
target_include_directories(frontend INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

qt_add_translations(frontend
    TS_FILES "${TRANSLATIONS_DIR}/application_ru.ts" "${TRANSLATIONS_DIR}/application_en.ts"
    RESOURCE_PREFIX "/translations"
    -no-obsolete
)

