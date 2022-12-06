include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${CMAKE_CURRENT_LIST_DIR}/images)
include_directories(${CMAKE_CURRENT_LIST_DIR}/video)
include_directories(${CMAKE_CURRENT_LIST_DIR}/audio)
# TODO

set(XFORMATS_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/xbinary.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xcom.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xmsdos.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xne.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xpe.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xelf.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xmach.cpp
    ${CMAKE_CURRENT_LIST_DIR}/subdevice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xformats.cpp
    ${CMAKE_CURRENT_LIST_DIR}/images/xicon.cpp
    ${CMAKE_CURRENT_LIST_DIR}/images/xbmp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/images/xgif.cpp
    ${CMAKE_CURRENT_LIST_DIR}/images/xpng.cpp
    ${CMAKE_CURRENT_LIST_DIR}/images/xjpeg.cpp
    ${CMAKE_CURRENT_LIST_DIR}/images/xtiff.cpp
    ${CMAKE_CURRENT_LIST_DIR}/video/xmp4.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audio/xmp3.cpp
)
