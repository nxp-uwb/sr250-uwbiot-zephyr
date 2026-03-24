# Copyright 2021,2022-2023 NXP
#
# NXP Proprietary. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#

cmake_minimum_required(VERSION 3.20.0)

IF(CONFIG_NXP_UWB_ZEPHYR_MODULE)
    PROJECT(vcom_common)

    zephyr_include_directories(
        ${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/demos/common/vcom_common
    )

    target_sources(app PRIVATE
        UwbHif.c
        UwbHif.h
        Uwb_usb_zephyr.c
        Uwb_usb_zephyr.h
    )
    target_link_libraries(app PUBLIC
        app_common
    )

ELSE()

PROJECT(vcom_common)

FILE(
    GLOB
    files
    UwbHif.c
    UwbHif.h
)

IF(UWBIOT_HOST_LPC55S_MK_SHIELD_V4)
    FILE(
        GLOB
        files_usb
        Uwb_usb.c
        Uwb_usb.h
        ${CMAKE_SOURCE_DIR}/ext/boards/lpc55s/usb/device/class/*.c
        ${CMAKE_SOURCE_DIR}/ext/boards/lpc55s/usb/device/class/cdc/*.c
    )
    ADD_LIBRARY(
        ${PROJECT_NAME} ${files}
                        ${files_usb}
    )
    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME}
        PUBLIC .
        PUBLIC
            ${CMAKE_SOURCE_DIR}/usb/device/class
        PUBLIC
            ${CMAKE_SOURCE_DIR}/usb/device/class/cdc
        PUBLIC
            ${CMAKE_SOURCE_DIR}/usb/device/source
        PUBLIC
            ${CMAKE_SOURCE_DIR}/usb/device/source/lpcip3511/
        PUBLIC
            ${CMAKE_SOURCE_DIR}/usb/device/include
        PUBLIC
            ${CMAKE_SOURCE_DIR}/usb/include
        PUBLIC
            ${CMAKE_SOURCE_DIR}/usb/phy
        PUBLIC
            ${CMAKE_SOURCE_DIR}/libs/halimpl/fwd
    )
ENDIF()

IF(
    UWBIOT_HOST_QN9090_MK_SHIELD_V4
    OR UWBIOT_HOST_RHODES4
    OR UWBIOT_HOST_FINDERV3
)
    FILE(
        GLOB
        files_usart
        usart_vcom_qn9090.c
    )
    ADD_LIBRARY(
        ${PROJECT_NAME} ${files}
                        ${files_usart}
    )
    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME} PUBLIC .
    )
ENDIF()

IF(UWBIOT_HOST_S32K144_SR040)
    FILE(
        GLOB
        files_lpusart
        usart_s32k.h
        usart_s32k.c
        ${CMAKE_SOURCE_DIR}/ext/boards/s32k/devices/drivers/src/lpuart_driver.c
    )

    ADD_LIBRARY(
        ${PROJECT_NAME}
        ${files} ${files_lpusart}
    )

    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME}
        PUBLIC .
        PUBLIC handlers
        PUBLIC
            ${CMAKE_SOURCE_DIR}/demos/common
        PUBLIC
            ${CMAKE_SOURCE_DIR}/ext/boards/s32k/devices/drivers/inc
        PUBLIC
            ${CMAKE_SOURCE_DIR}/demos/common/pnp/
        PRIVATE
            ${CMAKE_SOURCE_DIR}/demos/SR040/demo_swup_update_fw_Engg
    )

    TARGET_LINK_LIBRARIES(
        ${PROJECT_NAME} app_common
    )
ENDIF()

IF(UWBIOT_HOST_NRF52840_SR040
   OR UWBIOT_HOST_NRF52840_MK
)
    FILE(
        GLOB
        files_usb
        usart_nrf52840.c
    )
    ADD_LIBRARY(
        ${PROJECT_NAME} ${files}
                        ${files_usb}
    )
    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME} PUBLIC .
    )
ENDIF()

IF(UWBIOT_HOST_EMBLINUX)
    FILE(
        GLOB
        files_uart
        uart_linux.c
        uwb_socket.c
    )
    ADD_LIBRARY(
        ${PROJECT_NAME} ${files}
                        ${files_uart}
    )
    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME} PUBLIC .
    )
ENDIF()

IF(UWBIOT_HOST_CRETE)
    FILE(
        GLOB
        files_usb
        Uwb_usb.c
        Uwb_usb.h
        ${CMAKE_SOURCE_DIR}/ext/boards/MIMXRT1176/usb/device/*.c
        ${CMAKE_SOURCE_DIR}/ext/boards/MIMXRT1176/usb/osa/usb_osa_freertos.c
    )
    ADD_LIBRARY(
        ${PROJECT_NAME} ${files}
                        ${files_usb}
    )
    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME}
        PUBLIC .
        PUBLIC
            ${CMAKE_SOURCE_DIR}/ext/boards/MIMXRT1176/usb/include
        PUBLIC
            ${CMAKE_SOURCE_DIR}/ext/boards/MIMXRT1176/usb/osa
        PUBLIC
            ${CMAKE_SOURCE_DIR}/ext/boards/MIMXRT1176/usb/device
        PUBLIC
            ${CMAKE_SOURCE_DIR}/libs/halimpl/fwd
    )
ENDIF()

IF(UWBIOT_HOST_RW61X_QFN OR UWBIOT_HOST_VIRGO OR UWBIOT_HOST_FRDM_RW612)
    FILE(
        GLOB
        files_usb
        Uwb_usb.c
        Uwb_usb.h
    )
    ADD_LIBRARY(
        ${PROJECT_NAME} ${files}
                        ${files_usb}
    )
    TARGET_LINK_LIBRARIES(
        ${PROJECT_NAME} ${UWBIOT_BOARD_NAME}
    )
    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME}
        PUBLIC .
        PUBLIC
            ${CMAKE_SOURCE_DIR}/libs/halimpl/fwd
    )
ENDIF()

IF(UWBIOT_OS_NATIVE)
        TARGET_LINK_LIBRARIES(${PROJECT_NAME} uwb_${UWBIOT_UWBD})
ELSE()
        TARGET_LINK_LIBRARIES(${PROJECT_NAME} uwb_api)
ENDIF()
ENDIF()