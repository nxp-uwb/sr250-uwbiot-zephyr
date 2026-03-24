# Copyright 2020,2022,2024-2026 NXP
#
# NXP Proprietary. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#

PROJECT(halimpl)

MESSAGE(
    STATUS
        "UWBInterface:${UWBIOT_TML}, UWB_SUBDIR:${UWB_SUBDIR}"
)
FILE(
    GLOB
    source_files
    ${CMAKE_BINARY_DIR}/uwb_iot_ftr.h
    inc/*.h
    inc/*.h.in
    fwd/${UWB_SUBDIR}/*.c
    hal/phNxpUciHal.c
    hal/phNxpUciHal_ext.c
    ${LOG_SRC_DIR}/*.c
    ${LOG_SRC_DIR}/*.h
    tml/phTmlUwb.c
    tml/phTmlUwb_transport.c
    transport/serial/*.c
    utils/phNxpUciHal_utils.c
    utils/utlv.c
    utils/phNxpUwbConfig.c
    transport/${UWBIOT_TML}/common/*.c
)

SET (UWBD_FW_DIR
    binaries/UWBD
)

IF(UWBIOT_UWBD_SR1XXT)
    FILE(
        GLOB
        fwdl_sr1xx_files
        fwdl_provider/*.c
    )
ENDIF()

IF(UWBIOT_OS_FREERTOS)
    FILE(
        GLOB
        plat_files
        osal/phOsalUwb_FreeRTOS.c
        osal/phOsalUwb_Timer_FreeRTOS.c
        osal/phOsalUwb_Queue_FreeRTOS.c
        osal/phOsalUwb_Thread_FreeRTOS.c
    )
ENDIF()

IF(UWBIOT_OS_NATIVE)
    FILE(
        GLOB
        plat_files
        osal/phOsalUwb_posix.c
        osal/phOsalUwb_Timer_posix.c
        osal/phOsalUwb_Queue_posix.c
        osal/phOsalUwb_Thread_posix.c
    )
ENDIF()

IF(UWBIOT_OS_ZEPHYR)
    FILE(
        GLOB
        plat_files
        osal/phOsalUwb_Zephyr.c
        osal/phOsalUwb_Timer_Zephyr.c
        osal/phOsalUwb_Queue_Zephyr.c
        osal/phOsalUwb_Thread_Zephyr.c
    )
ENDIF()

IF(
    UWBIOT_TML_SPI
    OR UWBIOT_TML_LIBUWBD
    OR UWBIOT_TML_I2C
)
    FILE(
        GLOB
        tml_files
        transport/${UWBIOT_TML}/${UWB_SUBDIR}/*.c
        transport/${UWBIOT_TML}/${UWB_SUBDIR}/*.h
    )
ELSE()
    FILE(
        GLOB
        tml_files
        transport/${UWBIOT_TML}/*.c
        transport/${UWBIOT_TML}/*.h
    )
ENDIF()

ADD_LIBRARY(
    ${PROJECT_NAME}
    ${source_files}
    ${tml_files}
    ${plat_files}
    ${fwdl_sr1xx_files}
)

TARGET_INCLUDE_DIRECTORIES(
    ${PROJECT_NAME}
    PUBLIC ../uci-core/inc
    PUBLIC ../uci-core/uci/include
    PUBLIC ../uwb-iot/uwb_core/include
    PUBLIC ../uwb-iot/uwb_api/fira_generic
    PUBLIC
    ../uwb-iot/uwb_api/proprietary/${UWB_SUBDIR}
    PUBLIC
    ../uwb-iot/uwb_api/proprietary/
    PUBLIC
    ../uwb-iot/uwb_api/proprietary/SRXXX
    PUBLIC
    ../uwb-iot/uwb_api/proprietary/SRXXX/${UWB_SUBDIR}
    PUBLIC inc
    PUBLIC fwd/${UWB_SUBDIR}
    PUBLIC transport/${UWBIOT_TML}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/${UWBD_FW_DIR}/${UWBIOT_UWBD}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/libs/halimpl/inc
    PRIVATE transport/serial
)

IF(UWBIOT_UWBD_SR1XXT_SR2XXT)
    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME}
        PUBLIC
            ../uci-core/inc/SRXXX
    )

ELSEIF(UWBIOT_UWBD_SR040 OR UWBIOT_UWBD_SR048M)
    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME}
        PUBLIC
            ../uci-core/inc/SR040
    )

ENDIF()

IF(UWBIOT_UWBD_SR2XXT)
    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME}
        PUBLIC
            ${CMAKE_SOURCE_DIR}/firmware_images/SR2XX
        PUBLIC
            ${CMAKE_SOURCE_DIR}/firmware_images/SR2XX/Prod_ROW
        PUBLIC
            ${CMAKE_SOURCE_DIR}/${UWBD_FW_DIR}/SR2XX
    )
    IF(UWBIOT_OS_ZEPHYR)
        TARGET_INCLUDE_DIRECTORIES(
            ${PROJECT_NAME}
            PUBLIC
            ${UWB_ZEPHYR_PROJECT_DIR}
            PUBLIC
                ${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/${UWBD_FW_DIR}/SR2XX
            PUBLIC
                ${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/firmware_images/SR2XX
            PUBLIC
                ${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/firmware_images/SR2XX/Prod_ROW
        )
    ENDIF()
ENDIF()

IF(UWBIOT_OS_ZEPHYR)
    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME}
        PUBLIC ${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/boards/uwbs_driver
     )
ENDIF()

TARGET_LINK_LIBRARIES(
    ${PROJECT_NAME}
    mwlog
    ${UWBIOT_BOARD_NAME}
)

IF(UWBIOT_OS_FREERTOS)
    TARGET_LINK_LIBRARIES(
        ${PROJECT_NAME} freertos-kernel
    )
ENDIF()

IF(UWBIOT_OS_NATIVE)
    TARGET_LINK_LIBRARIES(
        ${PROJECT_NAME}
        pthread
        rt
    )
ENDIF()

IF(UWBIOT_OS_ZEPHYR)
    TARGET_LINK_LIBRARIES(
        ${PROJECT_NAME}
        kernel
    )
ENDIF()

IF(UWBIOT_HOST_LINUX_LIKE)
    INSTALL(
        FILES
            inc/phUwb_BuildConfig.h
            inc/phOsalUwb.h
            inc/phUwbTypes.h
            inc/phUwbStatus.h
            inc/phUwbCompId.h
            inc/uwbiot_ver.h
            inc/phOsalUwb_Thread.h
            inc/phOsalUwb_Timer.h
            inc/phOsalUwb_Queue.h
            inc/phOsalUwb_Internal.h
            inc/phNxpLogApis_App.h
            inc/uwb_logging.h
            inc/phNxpLogDefault.h
            inc/uwb_board_values.h
            inc/uwb_uwbs_tml_interface.h
        DESTINATION
            ${UWBIOT_INSTALL_INC_DIR}
    )
ENDIF()

UWBIOT_INSTALL_LIBRARY(${PROJECT_NAME})
