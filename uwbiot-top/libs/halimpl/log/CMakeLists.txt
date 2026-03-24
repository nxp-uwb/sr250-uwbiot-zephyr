# Copyright 2021 NXP
#
# NXP Proprietary. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#

PROJECT(mwlog)

FILE(
    GLOB
    source_files
    *.c
    *.h
)

ADD_LIBRARY(
    ${PROJECT_NAME} ${source_files}
)

TARGET_INCLUDE_DIRECTORIES(
    ${PROJECT_NAME}
    # PUBLIC .
    PUBLIC ../inc
    # PUBLIC ../osal
)
IF(UWBIOT_OS_FREERTOS)
    TARGET_LINK_LIBRARIES(
        ${PROJECT_NAME}
        ${UWBIOT_BOARD_NAME}
        freertos-kernel
        halimpl
    )
ENDIF()

IF(UWBIOT_OS_ZEPHYR)
    TARGET_LINK_LIBRARIES(
        ${PROJECT_NAME}
        ${UWBIOT_BOARD_NAME}
        kernel
        halimpl
    )
ENDIF()

IF(UWBIOT_OS_NATIVE)
    TARGET_LINK_LIBRARIES(
        ${PROJECT_NAME} halimpl
    )
ENDIF()

UWBIOT_INSTALL_LIBRARY(${PROJECT_NAME})
