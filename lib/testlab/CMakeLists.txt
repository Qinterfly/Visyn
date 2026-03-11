
# Determine the library path
set(TESTLAB_NAME "testlab-api")
if (${CMAKE_BUILD_TYPE} MATCHES Debug)
    set(TESTLAB_DIR "Debug")
else()
    set(TESTLAB_DIR "Release")
endif()

# Load the library
set(TESTLAB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${TESTLAB_DIR} CACHE INTERNAL "")
list(APPEND CMAKE_PREFIX_PATH ${TESTLAB_DIR})
find_library(TESTLAB_LIB ${TESTLAB_NAME} REQUIRED)
if(NOT TESTLAB_LIB)
    message(FATAL_ERROR "Testlab library is not found")
endif()

