# Copyright 2026 NXP
#
# NXP Proprietary. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#

PROJECT(${UWBIOT_BOARD_NAME})

SET(
    UWB_VERSION
    ${UWBIOTVER_STR_MAJOR}.${UWBIOTVER_STR_MINOR}.${UWBIOTVER_STR_DEV}
)

FILE(
    GLOB
    source_files
    *.c
    *.h
)

ADD_LIBRARY(
    ${PROJECT_NAME} ${source_files}
)

SET_TARGET_PROPERTIES(
    ${PROJECT_NAME}
    PROPERTIES
        VERSION ${UWB_VERSION}
        SOVERSION
        ${UWBIOTVER_STR_MAJOR}
)

TARGET_INCLUDE_DIRECTORIES(
    ${PROJECT_NAME}
    PUBLIC .
    PUBLIC include
    PUBLIC ${CMAKE_SOURCE_TOP_DIR}/boards/uwbs_driver
    PUBLIC ${CMAKE_SOURCE_TOP_DIR}/libs/halimpl/inc
    PUBLIC
        ${CMAKE_SOURCE_TOP_DIR}/libs/uwb-iot/uwb_api/proprietary/SRXXX/${UWB_SUBDIR}
)

TARGET_LINK_LIBRARIES(
    ${PROJECT_NAME}
    PUBLIC kernel
    PUBLIC mwlog
)
