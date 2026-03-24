#
# Copyright 2026 NXP
#
# SPDX-License-Identifier: Apache-2.0
#

# ==============================================================================
# DEMO-SPECIFIC CONFIGURATION
# ==============================================================================
set(DEMO_BUILD_MACRO "UWBIOT_APP_BUILD__DEMO_PNP")
set(DEMO_BUILD_COMMON_MACRO "UWBIOT_APP_BUILD__DEMO_STANDALONE_COMMON")

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
set(UWB_PNP_COMMON_DIR "${UWB_DEMOS_DIR}/common/pnp")
set(UWB_PNP_HOST_DIR "${UWB_PNP_COMMON_DIR}/FRDM_RW612")


# ==============================================================================
# UWB MODULE CONFIGURATION AND BUILD
# ==============================================================================
if(CONFIG_NXP_UWB_ZEPHYR_MODULE)
    # Include the UWB-specific Zephyr CMake configuration
    # This builds the UWB libraries (halimpl, uwb_core, uci_core, etc.)
    include(${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/zephyr/zephyr.cmake)

    # Verify that at least one UWB device type is configured
    if(NOT CONFIG_NXP_UWB_DEVICE_SR2XXT AND NOT CONFIG_NXP_UWB_DEVICE_SR1XXT AND NOT CONFIG_NXP_UWB_DEVICE_SR040)
        MESSAGE(
            FATAL_ERROR "No NXP UWB module is configured
            Hint: select either of the UWB device
            NXP_UWB_DEVICE_SR2XXT
            NXP_UWB_DEVICE_SR1XXT
            NXP_UWB_DEVICE_SR040")
    endif()

    message(STATUS "Including NXP UWB Demo Ranging Controlee")

    # ==============================================================================
    # SOURCE FILES
    # ==============================================================================
    # Add only the demo-specific source file
    target_sources(app PRIVATE
        # Demo-specific source file
        ${UWB_PNP_HOST_DIR}/PnP_Main.c
        ${UWB_PNP_COMMON_DIR}/UWBPnP_Common.c
        ${UWB_PNP_COMMON_DIR}/Uwb_Read_task_Zephyr.c
        ${UWB_PNP_COMMON_DIR}/Uwb_Hif_Handler_Zephyr.c
        ${UWB_PNP_COMMON_DIR}/UWB_Spi_Driver_Interface_Sr1xx_Sr2xx.c
        ${UWB_PNP_COMMON_DIR}/Uwb_Usb_Vcom_Pnp_Zephyr.c
    )

    # ==============================================================================
    # INCLUDE DIRECTORIES
    # ==============================================================================
    # Add include directories so the compiler can find header files
    # These paths are added to the 'app' target's include search path

    target_include_directories(app PRIVATE
        # Common PnP headers directory
        ${UWB_PNP_COMMON_DIR}
        # Host-specific PnP headers directory (FRDM_RW612)
        ${UWB_PNP_HOST_DIR}
    )

    # ==============================================================================
    # LIBRARY LINKING
    # ==============================================================================
    # Link with app_common which was built from demos/common/CMakeLists.txt

    target_link_libraries(app PUBLIC
        uwb_api      # UWB API library (FiRa and proprietary protocols)
        uwb_core     # UWB core functionality library
        uci_core     # UCI (UWB Command Interface) core library
    )

else()
    message(FATAL_ERROR
        "NXP UWB module not enabled
        Hint: to support NXP UWB module, configure CONFIG_NXP_UWB_ZEPHYR_MODULE=y in prj.conf" )
endif()
