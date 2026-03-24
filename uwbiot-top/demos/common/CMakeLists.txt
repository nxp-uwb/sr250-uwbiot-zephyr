# Copyright 2020,2023-2026 NXP
#
# NXP Proprietary. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#

IF(NOT(UWBIOT_UWBD_SR04X))
    # CSA demos
    IF(UWBFTR_CSA AND NOT(UWBIOT_HOST_FRDM_RW612))
        IF((UWBIOT_UWBD_SR150 AND UWBFTR_SE_SE051W) OR UWBIOT_UWBD_SR250)
            ADD_SUBDIRECTORY(demo_csa_controlee)
        ENDIF()

        # for internal testing - as CSA Controller has a mobile solution.
        IF(NXPInternal AND ((UWBIOT_UWBD_SR100S OR UWBIOT_UWBD_SR200S) AND (UWBIOT_SESN_SNXXX AND (NOT UWBIOT_SESN_P71))))
            ADD_SUBDIRECTORY(demo_csa_controller)
        ENDIF()
    ENDIF()

    # CCC demos
    IF(UWBFTR_CCC AND NOT(UWBIOT_HOST_FRDM_RW612))

        # for internal testing - as CCC Controlee has an automotive solution.
        IF(NXPInternal AND UWBIOT_UWBD_SR150 AND UWBFTR_SE_SE051W)
            ADD_SUBDIRECTORY(demo_ccc_controlee)
        ENDIF()

        # CCC_Controller is only applicable for SR100S and SR200S.
        # For internal testing FW has enabled CCC Controller support for SR150 as well.
        IF(((UWBIOT_UWBD_SR100S OR UWBIOT_UWBD_SR200S) AND (UWBIOT_SESN_SNXXX AND (NOT UWBIOT_SESN_P71))) OR
            (UWBIOT_UWBD_SR150 AND UWBFTR_SE_SE051W AND NXPInternal)
        )
            ADD_SUBDIRECTORY(demo_ccc_controller)
        ENDIF()

    ENDIF()

    IF(UWBFTR_DL_TDoA_Anchor)
        # DL-TDoA Anchor demos
        ADD_SUBDIRECTORY(demo_dltdoa_initiator)
        ADD_SUBDIRECTORY(demo_dltdoa_responder)
    ENDIF()

    IF(UWBFTR_DL_TDoA_Tag)
        # DL-TDoA Tag demos
        ADD_SUBDIRECTORY(demo_dltdoa_tag)
    ENDIF()

    # Demos which includes DataTransfer
    IF(UWBFTR_DataTransfer)
        # Hybrid Ranging demos
        ADD_SUBDIRECTORY(demo_hybrid_ranging_controlee)
        ADD_SUBDIRECTORY(demo_hybrid_ranging_controller)
        # DataTransfer demos
        ADD_SUBDIRECTORY(demo_inband_data_transfer_rx)
        ADD_SUBDIRECTORY(demo_inband_data_transfer_tx)
        IF(UWBIOT_UWBD_SR150 OR UWBIOT_UWBD_SR250)
            # OWR Advertiser demo
            ADD_SUBDIRECTORY(demo_owr_aoa_advertiser)
        ENDIF()
        IF(UWBIOT_UWBD_SR100T OR UWBIOT_UWBD_SR100S)
            # OWR Observer demo
            ADD_SUBDIRECTORY(demo_owr_aoa_observer)
        ENDIF()
    ENDIF()

    IF(UWBIOT_UWBD_SR150 AND (UWBFTR_SE_SE051W OR UWBIOT_TRANSIT_AUTH_MIFARE))
        IF(UWBIOT_HOST_RHODES4 AND UWBFTR_TransitProp AND UWBFTR_DataTransfer)
            ADD_SUBDIRECTORY(demo_transit)
        ENDIF()
    ENDIF()

    # TWR Demos
    IF(UWBFTR_TWR)
        ADD_SUBDIRECTORY(demo_ranging_controller)
        ADD_SUBDIRECTORY(demo_ranging_controlee)
    ENDIF()

    # Test Mode Demos
    ADD_SUBDIRECTORY(demo_test_rx)
    ADD_SUBDIRECTORY(demo_test_tx)

    IF(UWBFTR_UL_TDoA_Anchor)
        # UL-TDoA Anchor Demos
        ADD_SUBDIRECTORY(demo_ultdoa_anchor)
        ADD_SUBDIRECTORY(demo_ultdoa_sync_anchor)
    ENDIF()

    IF(UWBFTR_UL_TDoA_Tag)
        # UL-TDoA Tag Demos
        ADD_SUBDIRECTORY(demo_ultdoa_tag)
    ENDIF()

    IF((UWBIOT_HOST_MCTT_PCTT_LIKE OR UWBIOT_OS_ZEPHYR) AND (NOT UWBIOT_SESN_P71))
        # MCTT-PCTT Specific demos
        ADD_SUBDIRECTORY(mctt_pctt)
    ENDIF()

    IF(UWBIOT_HOST_PNP_LIKE OR UWBIOT_OS_ZEPHYR)
        # PNP Specific files
        ADD_SUBDIRECTORY(pnp)
    ENDIF()

    # VCOM Specific files
    IF(UWBIOT_HOST_PCWINDOWS OR UWBIOT_OS_ZEPHYR)
        # vcom_common not for PC
        MESSAGE(STATUS "vcom_common not for PC")
    ELSE()
        ADD_SUBDIRECTORY(vcom_common)
    ENDIF()

    # SE Specific Files
    IF (SSS_HAVE_APPLET_SE051_UWB OR UWBIOT_SESN_SNXXX)
        # se vecom Specific files for APPLET_SE051_UWB or UWBIOT_SESN_SNXXX
        IF (UWBIOT_HOST_QN9090_MK_SHIELD_V4 OR
            UWBIOT_HOST_RHODES4 OR
            UWBIOT_HOST_NRF52840_MK OR
            UWBIOT_HOST_CRETE OR
            UWBIOT_HOST_VIRGO
           )
            ADD_SUBDIRECTORY(se_vcom)
        ENDIF()
    ENDIF()

ENDIF()

PROJECT(app_common)


SET(main_file
    Standalone_Main.c
)

IF(UWBIOT_UWBD_SR040 OR UWBIOT_UWBD_SR048M)
    SET(swup_file
        Demo_SR040_Swup_Update.c
    )
    SET(
        radioConGroupDelay_file
        Demo_SR040_RadioConfigAndGroupDelay.c
    )
ENDIF()

ADD_LIBRARY(
    ${PROJECT_NAME}
    ${main_file}
    ${swup_file}
    ${radioConGroupDelay_file}
    Demo_Common_Config.c
    demo_app_callbacks.c
    AppStateManagement.c
    AppInternal.h
    Utilities.c
)
TARGET_INCLUDE_DIRECTORIES(
    ${PROJECT_NAME}
    PUBLIC .
)

IF(UWBIOT_UWBD_SR04X)
    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME}
        PUBLIC .
        PUBLIC
            ../SR040/demo_swup_update_fw/
        PUBLIC
            ../SR040/demo_swup_update_dsp/
    )
ENDIF()

IF(UWBIOT_UWBD_SR100T)
    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME}
        PUBLIC .
    )
ENDIF()

IF(UWBIOT_OS_NATIVE)
    TARGET_LINK_LIBRARIES(
        ${PROJECT_NAME}
        uwb_${UWBIOT_UWBD}
        uwb_core
        uci_core
    )
ELSE()
    TARGET_LINK_LIBRARIES(
        ${PROJECT_NAME}
        uwb_api
        uwb_core
        uci_core
    )
ENDIF()

IF(UWBIOT_HOST_LINUX_LIKE)
    INSTALL(
        FILES
            AppInternal.h
            AppStateManagement.h
            Demo_Common_Config.h
        DESTINATION
            ${UWBIOT_INSTALL_INC_DIR}
    )
ENDIF()


UWBIOT_INSTALL_LIBRARY(${PROJECT_NAME})
