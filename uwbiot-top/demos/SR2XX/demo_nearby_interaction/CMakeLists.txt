# Copyright 2021-2024 NXP
#
# NXP Proprietary. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#
    PROJECT(demo_nearby_interaction)

    FILE(
        GLOB
        files
        *.c
        *.h
        src/*.c
        src/*.h
    )

    ADD_EXECUTABLE(
        ${PROJECT_NAME}
        ${files} ${KSDK_STARTUP_FILE}
    )

    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME}
        PUBLIC .
        PUBLIC inc
    )

    TARGET_LINK_LIBRARIES(
        ${PROJECT_NAME}
        app_common
        ble_lib
    )

    TARGET_COMPILE_DEFINITIONS(
        ${PROJECT_NAME}
        PUBLIC
            UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION
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
