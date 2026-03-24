# Copyright 2020, 2025,2026 NXP
#
# NXP Proprietary. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#

PROJECT(uci_core)

IF(
    UWBIOT_UWBD_SR100T
    OR UWBIOT_UWBD_SR150
    OR UWBIOT_UWBD_SR100S
    OR UWBIOT_UWBD_SR200T
    OR UWBIOT_UWBD_SR250
    OR UWBIOT_UWBD_SR200S
)
    SET(UWB_SUBDIR "SRXXX")
ELSEIF(
     UWBIOT_UWBD_SR040
     OR UWBIOT_UWBD_SR048M
)
    SET(UWB_SUBDIR "SR040")
ELSE()
    SET(UWB_SUBDIR ${UWBIOT_UWBD})
ENDIF()

FILE(
    GLOB
    source_files
    inc/${UWB_SUBDIR}/*.h
    inc/*.h
    src/*.c
)

ADD_LIBRARY(
    ${PROJECT_NAME} ${source_files}
)

TARGET_INCLUDE_DIRECTORIES(
    ${PROJECT_NAME}
    PUBLIC inc/${UWB_SUBDIR}
)

IF(UWBIOT_OS_FREERTOS)
    TARGET_LINK_LIBRARIES(
        ${PROJECT_NAME}
        freertos-kernel
        halimpl
        uwb_core
    )
    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME}
        PUBLIC
            ../uwb-iot/uwb_core/gki/freertos
    )
ENDIF()

IF(UWBIOT_OS_NATIVE)
    TARGET_LINK_LIBRARIES(
        ${PROJECT_NAME}
        halimpl
        uwb_core
    )
ENDIF()

IF(UWBIOT_OS_ZEPHYR)
    TARGET_LINK_LIBRARIES(
        ${PROJECT_NAME}
        kernel
        halimpl
        uwb_core
    )
ENDIF()

IF(UWBIOT_HOST_LINUX_LIKE)
    INSTALL(
        FILES
            inc/uci_defs.h
            inc/uwa_api.h
            inc/uwb_types.h
        DESTINATION
            ${UWBIOT_INSTALL_INC_DIR}
    )
ENDIF()

UWBIOT_INSTALL_LIBRARY(${PROJECT_NAME})
