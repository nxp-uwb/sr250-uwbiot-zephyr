#!/bin/bash

# Copyright 2025 NXP
#
# NXP Proprietary. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#
#
# Run this script from this folder only
#

IF(CONFIG_NXP_UWB_ZEPHYR_MODULE)
    message(STATUS "Including NXP UWB Library")
    INCLUDE(${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/scripts/version_info.cmake)
    include(${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/zephyr/zephyr.cmake)

    set(LOG_SRC_DIR  "${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/libs/halimpl/log")
    set(UWB_LIB_DIR  "${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/libs")
    set(BOARDS_DIR  "${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/boards")

    zephyr_compile_definitions(UWBIOT_USE_FTR_FILE)

    ADD_SUBDIRECTORY(${LOG_SRC_DIR} ${CMAKE_CURRENT_BINARY_DIR}/libs/halimpl/log)
    ADD_SUBDIRECTORY(${BOARDS_DIR} ${CMAKE_CURRENT_BINARY_DIR}/boards)
    ADD_SUBDIRECTORY(${UWB_LIB_DIR} ${CMAKE_CURRENT_BINARY_DIR}/libs)
ENDIF()
