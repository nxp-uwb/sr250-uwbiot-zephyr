# Copyright 2025,2026 NXP
#
# NXP Proprietary. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#

SET(UWBIOT_TML "SPI")
SET(UWBIOT_OS "Zephyr")

SET(UWBIOT_OS_ZEPHYR ON)
SET(UWBIOT_LOG_DEFAULT ON)
SET(UWBIOT_SR1XX_FW_ROW_PROD ON)
SET(UWBIOT_TML_SPI ON)
SET(UWBIOT_Timing_RSTU ON)
SET(SSS_HAVE_APPLET_NONE ON)

IF(CONFIG_NXP_UWB_DEVICE_SR2XXT)
    SET(UWBIOT_UWBD_SR2XXT ON)
    SET(UWBIOT_UWBD_SR250 ON)
    SET(UWB_SUBDIR "SR2XX")
    SET(UWBFTR_UL_TDoA_Anchor ON)
    SET(UWBFTR_TWR ON)
    SET(UWBFTR_Radar ON)
    SET(UWBFTR_DataTransfer ON)
    SET(UWBFTR_DL_TDoA_Anchor ON)
    SET(UWBFTR_UWBS_DEBUG_Dump OFF)
    SET(UWBFTR_CCC ON)
    SET(UWBFTR_CSA ON)
    SET(UWBFTR_TransitProp OFF)
    SET(UWBFTR_BlobParser ON)
    SET(UWBFTR_AoA_FoV ON)
    IF(CONFIG_BOARD_FRDM_RW612)
        SET(UWBIOT_HOST_FRDM_RW612 ON)
        SET(UWBIOT_BOARD_NAME "frdm_rw612")
    ELSE()
        SET(UWBIOT_HOST_VIRGO ON)
        SET(UWBIOT_BOARD_NAME "virgo")
    ENDIF()
    SET(UWBIOT_Host ${UWBIOT_BOARD_NAME})
ENDIF()

IF(CONFIG_NXP_UWB_DEVICE_SR040)
    SET(UWBIOT_UWBD_SR040 ON)
    SET(UWBIOT_HOST_NRF52840_SR040 ON)
    SET(UWBIOT_BOARD_NAME "NRF52840_SR040")
    SET(UWBIOT_Host ${UWBIOT_BOARD_NAME})
    SET(UWB_SUBDIR "SR040")
    SET(UWBFTR_UL_TDoA_Tag ON)
    SET(UWBFTR_TWR ON)
    SET(UWBFTR_DataTransfer ON)
    SET(UWBFTR_BlobParser ON)
ENDIF()

IF(CONFIG_NXP_UWB_DEVICE_SR048M)
    SET(UWBIOT_UWBD_SR048M ON)
    # TODO: SR048 HOST to be added
    SET(UWB_SUBDIR "SR040")
    SET(UWBFTR_TWR ON)
ENDIF()

IF(CONFIG_NXP_UWB_DEVICE_SR1XXT)
    message(FATAL_ERROR "NXP UWB SR1XXT module support not available")
ENDIF()

IF(UWBIOT_UWBD_SR1XXT OR UWBIOT_UWBD_SR2XXT)
    SET(UWBIOT_UWBD_SR1XXT_SR2XXT ON)
ENDIF()

IF(UWBIOT_UWBD_SR040 OR UWBIOT_UWBD_SR048M)
    SET(UWBIOT_UWBD_SR04X ON)
ENDIF()

MACRO(CREATE_BINARY PROJECT_NAME)

ENDMACRO()

MACRO(
    UWBIOT_INSTALL_LIBRARY PROJECT_NAME
)

ENDMACRO()