# Copyright 2021,2025 NXP
#
# NXP Proprietary. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#

PROJECT(demo_ranging_controlee)

FILE(
    GLOB
    files
    *.rst
    *.h
    *.md
)

ADD_EXECUTABLE(
    ${PROJECT_NAME}
    ${files}
    ${KSDK_STARTUP_FILE}
    demo_ranging_controlee.c
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
        UWBIOT_APP_BUILD__DEMO_RANGING_CONTROLEE
)

CREATE_BINARY(${PROJECT_NAME})

IF(
    CMAKE_BUILD_TYPE
    STREQUAL
    "Release"
)
    IF(SSS_HAVE_APPLET_SE051_UWB)
        COPY_TO_SOURCEDIR(
            ${PROJECT_NAME}
            binaries/${UWBIOT_Host}_SE
            "${UWBIOT_UWBD}-${UWBIOTVER_STR_VER_STR}"
        )
    ENDIF()
    IF(UWBIOT_SESN_SNXXX)
        IF(UWBIOT_SESN_P71)
            COPY_TO_SOURCEDIR(
                ${PROJECT_NAME}
                binaries/${UWBIOT_Host}_P71
                "${UWBIOT_SESN}-${UWBIOTVER_STR_VER_STR}"
            )
        ELSE()
            COPY_TO_SOURCEDIR(
                ${PROJECT_NAME}
                binaries/${UWBIOT_Host}_SN
                "${UWBIOT_SESN}-${UWBIOTVER_STR_VER_STR}"
            )
        ENDIF(UWBIOT_SESN_P71)
    ENDIF(UWBIOT_SESN_SNXXX)
ENDIF()