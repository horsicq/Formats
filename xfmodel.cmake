include_directories(${CMAKE_CURRENT_LIST_DIR})

set(XFMODEL_SOURCES
    ${XFMODEL_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/xfmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xfmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/xfmodel_header.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xfmodel_header.h
    ${CMAKE_CURRENT_LIST_DIR}/xfmodel_table.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xfmodel_table.h
    ${CMAKE_CURRENT_LIST_DIR}/xftree_model.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xftree_model.h
)
