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


IF(UWBIOT_HOST_VIRGO)
    ADD_SUBDIRECTORY(virgo)
ELSEIF(UWBIOT_HOST_RW61X_QFN)
    ADD_SUBDIRECTORY(rd_rw612_qfn)
ELSEIF(UWBIOT_HOST_FRDM_RW612)
    # message(STATUS ">>> Adding frdm_rw612 subdirectory <<<")
    ADD_SUBDIRECTORY(frdm_rw612)
ENDIF()

