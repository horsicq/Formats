include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${CMAKE_CURRENT_LIST_DIR}/..)

if (NOT DEFINED XBINARY_SOURCES)
    include(${CMAKE_CURRENT_LIST_DIR}/../xbinary.cmake)
    set(XCLIASSEMBLY_SOURCES ${XCLIASSEMBLY_SOURCES} ${XBINARY_SOURCES})
endif()

set(XCLIASSEMBLY_SOURCES
    ${XCLIASSEMBLY_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/xcliassembly.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xcliassembly.h
    ${CMAKE_CURRENT_LIST_DIR}/xcliassembly_def.h
)
