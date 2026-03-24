# Copyright 2020,2025,2026 NXP
#
# NXP Proprietary. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#

MESSAGE(
    STATUS "UWBIOT_Host=${UWBIOT_Host}"
)

IF(UWBIOT_OS_ZEPHYR)
    IF(UWBIOT_HOST_VIRGO OR UWBIOT_HOST_RW61X_QFN OR UWBIOT_HOST_FRDM_RW612)
        ADD_SUBDIRECTORY(Host/nxp)
    ELSEIF(CONFIG_BOARD_NRF52840DK)
        ADD_SUBDIRECTORY(Host/nordic/nrf52840dk)
    ENDIF()
ELSE()
    ADD_SUBDIRECTORY(Host/${UWBIOT_Host})
ENDIF()
