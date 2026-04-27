include_directories(${CMAKE_CURRENT_LIST_DIR})

if (NOT DEFINED XBINARY_SOURCES)
    include(${CMAKE_CURRENT_LIST_DIR}/xbinary.cmake)
endif()

set(XFILEPART_SOURCES
    ${XFILEPART_SOURCES}
    ${XBINARY_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/xfilepart.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xfilepart.h
)
