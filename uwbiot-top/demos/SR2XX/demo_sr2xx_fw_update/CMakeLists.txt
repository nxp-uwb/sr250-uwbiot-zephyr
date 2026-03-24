# Copyright 2025 NXP
#
# NXP Proprietary. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#

PROJECT(demo_sr2xx_fw_update)

FILE(
    GLOB
    files
    *.h
)

ADD_EXECUTABLE(
    ${PROJECT_NAME}
    ${files}
    ${KSDK_STARTUP_FILE}
    demo_sr2xx_fw_update.c
)

TARGET_INCLUDE_DIRECTORIES(
    ${PROJECT_NAME}
    PUBLIC .
)

TARGET_LINK_LIBRARIES(
    ${PROJECT_NAME} app_common
)

TARGET_COMPILE_DEFINITIONS(
    ${PROJECT_NAME}
    PUBLIC
        UWBIOT_APP_BUILD__DEMO_SR2XX_FW_UPDATE
)

CREATE_BINARY(${PROJECT_NAME})

IF(
    CMAKE_BUILD_TYPE
    STREQUAL
    "Release"
)
    COPY_TO_SOURCEDIR(
        ${PROJECT_NAME}
        binaries/${UWBIOT_Host}
        "${UWBIOT_UWBD}-${UWBIOTVER_STR_VER_STR}"
    )
ENDIF()