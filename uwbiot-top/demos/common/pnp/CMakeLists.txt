# Copyright 2020,2022-2026 NXP
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
    IF(UWBIOT_HOST_NRF52840_SR040)
        PROJECT(pnp_${UWBIOT_Host})
    ELSE()
        PROJECT(
            pnp_${UWBIOT_Host}_${UWBIOT_UWBD}
        )
    ENDIF()

    zephyr_include_directories(
    ${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/demos/common/pnp
    ${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/demos/common/pnp/${UWBIOT_Host}/
    )

IF(UWBIOT_UWBD_SR040 AND UWBIOT_HOST_NRF52840_SR040)
    MESSAGE(STATUS "SR040 PnP App")
    target_sources(app PRIVATE
        NRF52840_SR040/app_config.h
        NRF52840_SR040/PnP_Main.c
        NRF52840_SR040/pnp_uart_zephyr.c
        UWB_Spi_Driver_Interface_Sr04x.c
        Uwb_Hif_Handler_Zephyr.c
        Uwb_Read_task.h
        UWB_Spi_Driver_Interface.h
        Uwb_Read_task_Zephyr.c
        Uwb_Vcom_Pnp.h
        UWBPnP_Common.c
        UwbPnpInternal.h
        UWB_DRIVER.h
        )
ENDIF()
    target_link_libraries(app PUBLIC app_common)

ELSE()
    IF(UWBIOT_HOST_NRF52840_SR040)
        PROJECT(pnp_${UWBIOT_Host})
    ELSE()
        PROJECT(
            pnp_${UWBIOT_Host}_${UWBIOT_UWBD}
        )
    ENDIF()

    FILE(
        GLOB
        common_files
        *.c
        *.h
    )

    IF(UWBIOT_OS_NATIVE)
        FILE(
            GLOB
            host_files
            Socket/*.h
            Socket/*.c
        )

        ADD_EXECUTABLE(
            ${PROJECT_NAME} ${host_files}
                            ${common_files}
        )
    ELSE()
        FILE(
            GLOB
            host_files
            ${UWBIOT_Host}/*.h
            ${UWBIOT_Host}/*.c
        )

        ADD_EXECUTABLE(
            ${PROJECT_NAME}
            ${host_files}
            ${common_files}
            ${KSDK_STARTUP_FILE}
        )
    ENDIF()

    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME}
        PRIVATE .
        PRIVATE ${UWBIOT_Host}
        PRIVATE
            ../../ext/boards/ksdk/usb/include
        PRIVATE
            ../../ext/boards/ksdk/usb/osa
        PRIVATE
            ../../ext/boards/ksdk/usb/device
        PRIVATE ../../libs/halimpl/fwd
        PRIVATE ../SR040/demo_swup_update_fw
    )

    IF(UWBIOT_OS_NATIVE)
        TARGET_INCLUDE_DIRECTORIES(
            ${PROJECT_NAME} PRIVATE Socket
        )
    ENDIF()

IF(
    UWBIOT_HOST_FINDERV3
    OR UWBIOT_HOST_QN9090_MK_SHIELD_V4
    OR UWBIOT_HOST_RHODES4
    OR UWBIOT_HOST_CRETE
    OR UWBIOT_HOST_EMBLINUX
    OR UWBIOT_HOST_NRF52840_MK
    OR UWBIOT_HOST_NRF52840_SR040
    OR UWBIOT_HOST_RW61X_QFN
    OR UWBIOT_HOST_VIRGO
    OR UWBIOT_HOST_FRDM_RW612
)
    SET(BUILD_MASK DEMO_PNP)
ELSE()
    MESSAGE(FATAL_ERROR "Invalid combination to build PnP App")
    SET(BUILD_MASK UNKNOWN)
ENDIF()

TARGET_COMPILE_DEFINITIONS(
    ${PROJECT_NAME}
    PRIVATE
        UWBIOT_APP_BUILD__${BUILD_MASK}
)

IF(UWBIOT_SESN_P71)

    IF(SSS_HAVE_SMCOM_T1OI2C)
        MESSAGE(STATUS "T1oI2C interface")
        ADD_DEFINITIONS(-DT1oI2C -DT1oI2C_UM11225)
    ENDIF()

    FILE(
        GLOB
        SRC_SMCOM
        ${SIMW_TOP_DIR}/se_hostlib/hostLib/libCommon/smCom/smCom.c
        ${SIMW_TOP_DIR}/se_hostlib/hostLib/libCommon/smCom/T1oI2C/*.c
        ${SIMW_TOP_DIR}/se_hostlib/hostLib/libCommon/smCom/smComT1oI2C*.c
    )

    TARGET_SOURCES(
        ${PROJECT_NAME}
        PRIVATE
        ${SRC_SMCOM}
    )

    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME}
        PRIVATE
            .
            ${SIMW_TOP_DIR}/se_hostlib/hostLib/inc/
            ${SIMW_TOP_DIR}/se_hostlib/hostLib/libCommon/infra/
            ${SIMW_TOP_DIR}/se_hostlib/hostLib/platform/inc/
            ${SIMW_TOP_DIR}/se_hostlib/hostLib/libCommon/smCom/
            ${SIMW_TOP_DIR}/se_hostlib/hostLib/libCommon/smCom/T1oI2C/
    )
ENDIF()

TARGET_LINK_LIBRARIES(
    ${PROJECT_NAME} app_common
)

CREATE_BINARY(${PROJECT_NAME})

IF(
    CMAKE_BUILD_TYPE
    STREQUAL
    "Release"
)
    IF(UWBIOT_SESN_SNXXX)
        IF(UWBIOT_SESN_P71)
            COPY_TO_SOURCEDIR(
                ${PROJECT_NAME}
                binaries/${UWBIOT_Host}_P71
                "${UWBIOTVER_STR_VER_STR}"
            )
        ELSE()
            COPY_TO_SOURCEDIR(
                ${PROJECT_NAME}
                binaries/${UWBIOT_Host}_SN
                "${UWBIOTVER_STR_VER_STR}"
            )
        ENDIF()
    ELSE()
    IF(UWBFTR_SE_SE051W)
        COPY_TO_SOURCEDIR(
                ${PROJECT_NAME}
                binaries/${UWBIOT_Host}_SE
                "${UWBIOTVER_STR_VER_STR}"
            )
    ENDIF()
        COPY_TO_SOURCEDIR(
                    ${PROJECT_NAME}
                    binaries/${UWBIOT_Host}
                    "${UWBIOTVER_STR_VER_STR}"
                )
    ENDIF(UWBIOT_SESN_SNXXX)
ENDIF()
ENDIF()
