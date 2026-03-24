# Copyright 2024,2026 NXP
#
# NXP Proprietary. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#


PROJECT(demo_device_calibration)

FILE(
    GLOB
    files
    *.rst
    *.h
    *.md
)

ADD_EXECUTABLE(
    ${PROJECT_NAME}
    ${files} ${KSDK_STARTUP_FILE}
    demo_device_calibration.c
)

TARGET_INCLUDE_DIRECTORIES(
    ${PROJECT_NAME}
    PUBLIC .
)

TARGET_LINK_LIBRARIES(
    ${PROJECT_NAME}
    app_common
)

TARGET_COMPILE_DEFINITIONS(
    ${PROJECT_NAME}
    PUBLIC
        UWBIOT_APP_BUILD__DEMO_DEVICE_CALIBRATION
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
