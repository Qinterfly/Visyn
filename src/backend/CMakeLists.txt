
find_package(Qt6 REQUIRED COMPONENTS Core LinguistTools)

set(BACKEND_HEADERS
    fileutility.h
    project.h
    response.h
)

set(BACKEND_SOURCES
    fileutility.cpp
    project.cpp
    response.cpp
)

qt_add_library(backend STATIC
    ${BACKEND_HEADERS}
    ${BACKEND_SOURCES}
)

target_include_directories(backend PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib
)

target_link_libraries(backend PUBLIC
    Qt::Core
    visom
)
