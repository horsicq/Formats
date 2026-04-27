include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${CMAKE_CURRENT_LIST_DIR}/..)

if (NOT DEFINED XFILEPART_SOURCES)
    include(${CMAKE_CURRENT_LIST_DIR}/../xfilepart.cmake)
    set(XCLIASSEMBLY_SOURCES ${XCLIASSEMBLY_SOURCES} ${XFILEPART_SOURCES})
endif()

set(XCLIASSEMBLY_SOURCES
    ${XCLIASSEMBLY_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/xcliassembly.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xcliassembly.h
    ${CMAKE_CURRENT_LIST_DIR}/xcliassembly_def.h
)
