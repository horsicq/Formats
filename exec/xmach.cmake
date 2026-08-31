include_directories(${CMAKE_CURRENT_LIST_DIR})

if (NOT DEFINED XBINARY_SOURCES)
    include(${CMAKE_CURRENT_LIST_DIR}/../xbinary.cmake)
    set(XMACH_SOURCES ${XMACH_SOURCES} ${XBINARY_SOURCES})
endif()

set(XMACH_SOURCES
    ${XMACH_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/xmach.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xmach.h
    ${CMAKE_CURRENT_LIST_DIR}/xmach_def.h
)

