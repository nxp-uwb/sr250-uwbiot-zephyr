# Copyright 2022-2024 NXP
#
# NXP Proprietary. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#

IF((UWBIOT_HOST_VIRGO OR UWBIOT_HOST_RW61X_QFN OR UWBIOT_HOST_FRDM_RW612)
    AND UWBFTR_BlobParser
)
    ADD_SUBDIRECTORY(demo_nearby_interaction)

ENDIF()

# Not required as of now
# IF(NXPInternal AND (UWBIOT_HOST_VIRGO OR UWBIOT_HOST_FRDM_RW612))
#     ADD_SUBDIRECTORY(demo_wifi_wpa_supplicant)
# ENDIF()

    ADD_SUBDIRECTORY(demo_device_calibration)
    ADD_SUBDIRECTORY(demo_sr2xx_fw_update)
# Below macro provides mutual exclusion between stand alone mode and pnp mode

IF(UWBIOT_UWBD_SR2XXT AND UWBIOT_HOST_PCWINDOWS)
    ADD_SUBDIRECTORY(sr2xx_Fwdnld_app_cli)
ENDIF()
