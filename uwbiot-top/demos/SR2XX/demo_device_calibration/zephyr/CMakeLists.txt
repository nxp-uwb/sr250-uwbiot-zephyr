#
# Copyright 2026 NXP
#
# SPDX-License-Identifier: Apache-2.0
#

# ==============================================================================
# DEMO-SPECIFIC CONFIGURATION
# ==============================================================================
set(DEMO_BUILD_MACRO "UWBIOT_APP_BUILD__DEMO_DEVICE_CALIBRATION")
set(DEMO_BUILD_COMMON_MACRO "UWBIOT_APP_BUILD__DEMO_STANDALONE_COMMON")
set(DEMO_SOURCE_FILE "../demo_device_calibration.c")
# ==============================================================================
# STANDARD CMAKE INITIALIZATION
# ==============================================================================
cmake_minimum_required(VERSION 3.20.0)
# ==============================================================================
# CRITICAL: Set feature file include directory BEFORE find_package(Zephyr)
# ==============================================================================
# Set the directory where uwb_iot_ftr.h feature configuration file is located
# This MUST be set as a global include directory BEFORE find_package(Zephyr)
# so that all UWB libraries (halimpl, uwb_core, etc.) can find it during their build
SET(UWB_FTR_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Add as a global include directory so all targets can find uwb_iot_ftr.h
include_directories(${UWB_FTR_INCLUDE_DIR})
message(STATUS "Using uwb_iot_ftr.h from: ${UWB_FTR_INCLUDE_DIR}")

# Define compile-time macros BEFORE find_package(Zephyr)
add_compile_definitions(UWBIOT_USE_FTR_FILE)
add_compile_definitions(${DEMO_BUILD_MACRO})
add_compile_definitions(${DEMO_BUILD_COMMON_MACRO})

# Find and load the Zephyr RTOS package
# After this point, ZEPHYR_NXP_UWBIOT_MODULE_DIR will be available
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Define the project name for CMake
project(nxp-uwb)

# ==============================================================================
# PATH CONFIGURATION
# ==============================================================================
set(UWB_DEMOS_DIR "${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/demos")
set(UWB_DEMO_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

# ==============================================================================
# UWB MODULE CONFIGURATION AND BUILD
# ==============================================================================
if(CONFIG_NXP_UWB_ZEPHYR_MODULE)
    # Include the UWB-specific Zephyr CMake configuration
    include(${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/zephyr/zephyr.cmake)
    # Verify that at least one UWB device type is configured
    # The build cannot proceed without selecting a target UWB device
    if(NOT CONFIG_NXP_UWB_DEVICE_SR2XXT AND NOT CONFIG_NXP_UWB_DEVICE_SR1XXT AND NOT CONFIG_NXP_UWB_DEVICE_SR040)
        MESSAGE(
            FATAL_ERROR "No NXP UWB module is configured
            Hint: select either of the UWB device
            NXP_UWB_DEVICE_SR2XXT
            NXP_UWB_DEVICE_SR1XXT
            NXP_UWB_DEVICE_SR040")
    endif()
    # Log which demo is being built
    message(STATUS "Including NXP UWB Device Calibration")

    # ==============================================================================
    # SOURCE FILES
    # ==============================================================================
    target_sources(app PRIVATE
        # Demo-specific source file
        ${DEMO_SOURCE_FILE}
        # Common demo infrastructure source files
        # These files provide shared functionality used by all standalone demos
        ${UWB_DEMOS_DIR}/common/Standalone_Main.c        # Main entry point for standalone demos
        ${UWB_DEMOS_DIR}/common/Demo_Common_Config.c     # Common configuration utilities
        ${UWB_DEMOS_DIR}/common/demo_app_callbacks.c     # Application callback handlers
        ${UWB_DEMOS_DIR}/common/AppStateManagement.c     # Application state machine
        ${UWB_DEMOS_DIR}/common/Utilities.c              # Utility functions
    )
    # ==============================================================================
    # INCLUDE DIRECTORIES
    # ==============================================================================
    target_include_directories(app PRIVATE
        # Common demo headers
        ${UWB_DEMOS_DIR}/common
        # This demo's specific headers
        ${UWB_DEMO_SOURCE_DIR}
    )
    # ==============================================================================
    # LIBRARY LINKING
    # ==============================================================================
    # Link the application with required UWB libraries
    # PUBLIC visibility means these libraries are also available to anything that links to 'app'
    target_link_libraries(app PUBLIC
        uwb_api      # UWB API library (FiRa and proprietary protocols)
        uwb_core     # UWB core functionality library
        uci_core     # UCI (UWB Command Interface) core library
    )
else()
    # If CONFIG_NXP_UWB_ZEPHYR_MODULE is not enabled, halt the build with an error
    # This configuration option must be set in prj.conf to enable UWB support
    message(FATAL_ERROR
        "NXP UWB module not enabled
        Hint: to support NXP UWB module, configure CONFIG_NXP_UWB_ZEPHYR_MODULE=y in prj.conf" )
endif()