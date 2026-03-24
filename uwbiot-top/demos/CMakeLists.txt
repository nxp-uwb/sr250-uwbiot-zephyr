# Copyright 2020-2024, 2025 NXP
#
# NXP Proprietary. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#

# Below macro provides mutual exclusion between stand alone mode and pnp mode
ADD_DEFINITIONS(
    "-DUWBIOT_APP_BUILD__DEMO_STANDALONE_COMMON"
)

# common demos & Files
ADD_SUBDIRECTORY(common)

# Radar Specific Demos
IF(UWBFTR_Radar)
    ADD_SUBDIRECTORY(radar)
ENDIF()

# rhodes4 Specific Demos
IF(UWBIOT_HOST_RHODES4 AND UWBIOT_UWBD_SR100S AND NOT(UWBIOT_SESN_P71))
    ADD_SUBDIRECTORY(rhodes4)
ENDIF()

# SE Demos
IF(UWBFTR_SE_SE051W)
    ADD_SUBDIRECTORY(SE051W)
ENDIF()

# SR1XX Specific Demos
IF(UWBIOT_UWBD_SR1XXT)
    ADD_SUBDIRECTORY(SR1XX)
ENDIF()

# SR1XX Specific Demos
IF(UWBIOT_UWBD_SR2XXT)
    ADD_SUBDIRECTORY(SR2XX)
ENDIF()

# SNXXX Specific Demos
IF(UWBIOT_SESN_SNXXX)
    ADD_SUBDIRECTORY(SNXXX)
ENDIF()

IF(UWBIOT_UWBD_SR04X)
    # SR040 Specific Demos
    ADD_SUBDIRECTORY(SR040)

    IF(UWBIOT_HOST_PNP_LIKE OR UWBIOT_OS_ZEPHYR)
        # PNP Specific files
        ADD_SUBDIRECTORY(common/pnp)
    ENDIF()

    # VCOM Specific files
    IF(UWBIOT_HOST_PCWINDOWS)
        # vcom_common not for PC
        MESSAGE(STATUS "vcom_common not for PC")
    ELSE()
        ADD_SUBDIRECTORY(common/vcom_common)
    ENDIF()
ENDIF()
