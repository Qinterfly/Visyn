
find_package(Qt6 REQUIRED COMPONENTS Core LinguistTools)
find_package(Eigen3 CONFIG REQUIRED)
find_package(matio CONFIG REQUIRED)

set(BACKEND_HEADERS
    fileutility.h
    mathutility.h
    project.h
    response.h
    harmonicsolver.h
)

set(BACKEND_SOURCES
    fileutility.cpp
    mathutility.cpp
    project.cpp
    response.cpp
    harmonicsolver.cpp
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
    Eigen3::Eigen
    MATIO::HDF5
    MATIO::ZLIB
    matio::matio
    visom
)
