
set(VISOM_HEADERS
    vaufxfile.h
)

set(VISOM_SOURCES
    vaufxfile.cpp
)

add_library(visom STATIC
    ${VISOM_HEADERS}
    ${VISOM_SOURCES}
)

target_include_directories(visom INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

