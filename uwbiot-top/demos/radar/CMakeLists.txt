# Copyright 2024 NXP
#
# NXP Proprietary. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#

IF(UWBFTR_Radar)
    ADD_SUBDIRECTORY(demo_radar_test_antenna_isolation)
    ADD_SUBDIRECTORY(demo_radar)
    ADD_SUBDIRECTORY(demo_Radar_Twr_Conc_Ranging)
    ADD_SUBDIRECTORY(demo_ocpd)

    IF(NXPInternal AND UWBIOT_HOST_VIRGO)
        # Support is added only for Virgo .
        # Radar support is avalible for Crete and Virgo
        # it will fail when Host-Crete.
        ADD_SUBDIRECTORY(demo_off_chip_pd)
    ENDIF()
ENDIF()
