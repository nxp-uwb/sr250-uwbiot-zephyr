# Copyright 2020,2022 NXP
#
# NXP Proprietary. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#
PROJECT(uwb_core)

FILE(
    GLOB
    source_files
    adaptation/*.c
)

IF(UWBIOT_OS_FREERTOS)
    FILE(GLOB plat_files)
ENDIF()

IF(UWBIOT_OS_NATIVE)
    FILE(GLOB plat_files)
ENDIF()

IF(UWBIOT_OS_ZEPHYR)
    FILE(GLOB plat_files)
ENDIF()

ADD_LIBRARY(
    ${PROJECT_NAME} ${source_files}
                    ${plat_files}
)

IF(UWBIOT_OS_NATIVE)
    # TARGET_INCLUDE_DIRECTORIES(
    #     ${PROJECT_NAME}
    # )
ENDIF()

TARGET_LINK_LIBRARIES(
    ${PROJECT_NAME} uci_core
)

IF(UWBIOT_HOST_LINUX_LIKE)
    INSTALL(
        FILES include/uwb_target.h
              include/uwb_hal_api.h
        DESTINATION
            ${UWBIOT_INSTALL_INC_DIR}
    )
ENDIF()

UWBIOT_INSTALL_LIBRARY(${PROJECT_NAME})
