/* Copyright 2022-2026 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef MAINLINE_FIRMWARE_H
#define MAINLINE_FIRMWARE_H

#include <stdint.h>

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

/* Depending on the selection, header file would be chosen
 *
 * Steps for the header file generation:
 *
 * 1) Use xxd -i ${file_name}.bin > ${file_name}.h (or similar utility)
 * 2) in the generated header file, replace ``unsigned char`` to ``const uint8_t``
 */

#if UWBIOT_UWBD_SR250
/*Select one of the firmware depending upon the host */
/* Selection For SR250 FW Starts Here */

#if UWBIOT_SR1XX_FW_ROW_PROD
#define SR200A1V2_IOT_01_41_04_Prod_bin                     heliosEncryptedMainlineFwImage
#define SR200A1V2_IOT_01_41_04_Prod_bin_len                 heliosEncryptedMainlineFwImageLen
#define SR200A1V2_IOT_01_41_04_Prod_bin_FwPatchVer          heliosEncryptedMainlineFwImagePatchVer
#define UWB_SR2XX_MAJOR_NUMBER                              0x01
#define UWB_SR2XX_MINOR_NUMBER                              0x41
#define UWB_SR2XX_PATCH_NUMBER                              0x04
#elif UWBIOT_SR1XX_FW_ROW_DEV
#define SR200A1V2_IOT_01_41_04_Dev_bin                  heliosEncryptedMainlineFwImage
#define SR200A1V2_IOT_01_41_04_Dev_bin_len              heliosEncryptedMainlineFwImageLen
#define SR200A1V2_IOT_01_41_04_Dev_bin_FwPatchVer       heliosEncryptedMainlineFwImagePatchVer
#define UWB_SR2XX_MAJOR_NUMBER                              0x01
#define UWB_SR2XX_MINOR_NUMBER                              0x41
#define UWB_SR2XX_PATCH_NUMBER                              0x04
#else
#error "Select anyone of the FW"
#endif
#endif // UWBIOT_UWBD_SR250

#if UWBIOT_UWBD_SR200T || UWBIOT_UWBD_SR200S
/*Select one of the firmware depending upon the host */
/* Selection For SR200T FW Starts Here */
/* Selection For SR200S FW Starts Here */

#if UWBIOT_SR1XX_FW_ROW_PROD
#define SR200_A1V2_Mobile_Prod_01_E3_01_bin                     heliosEncryptedMainlineFwImage
#define SR200_A1V2_Mobile_Prod_01_E3_01_bin_len                 heliosEncryptedMainlineFwImageLen
#define SR200_A1V2_Mobile_Prod_01_E3_01_bin_FwPatchVer          heliosEncryptedMainlineFwImagePatchVer
#define UWB_SR2XX_MAJOR_NUMBER                              0x01
#define UWB_SR2XX_MINOR_NUMBER                              0xE3
#define UWB_SR2XX_PATCH_NUMBER                              0x01
#elif UWBIOT_SR1XX_FW_ROW_DEV
#define SR200_A1V2_Mobile_Dev_01_E3_01_bin                  heliosEncryptedMainlineFwImage
#define SR200_A1V2_Mobile_Dev_01_E3_01_bin_len              heliosEncryptedMainlineFwImageLen
#define SR200_A1V2_Mobile_Dev_01_E3_01_bin_FwPatchVer       heliosEncryptedMainlineFwImagePatchVer
#define UWB_SR2XX_MAJOR_NUMBER                              0x01
#define UWB_SR2XX_MINOR_NUMBER                              0xE3
#define UWB_SR2XX_PATCH_NUMBER                              0x01
#else
#error "Select anyone of the FW"
#endif
#endif // UWBIOT_UWBD_SR200 || UWBIOT_UWBD_SR200S

/* FW selection End */

/* Depending on the selection, header file would be chosen
 *
 * Steps for the header file generation:
 *
 * 1) Use xxd -i ${file_name}.bin > ${file_name}.h (or similar utility)
 * 2) in the generated header file, replace ``unsigned char`` to ``const uint8_t``
 */

#ifdef SR200_A1V2_Mobile_Prod_01_E3_01_bin
#   include <SR200_A1V2_Mobile_Prod_01.E3.01.h>
#endif

#ifdef SR200_A1V2_Mobile_Dev_01_E3_01_bin
#   include <SR200_A1V2_Mobile_Dev_01_E3_01.h>
#endif

#ifdef SR250_A1V2_01_31_C0_PROD_bin
#   include <SR250_A1V2_01.31.C0_PROD.h>
#endif

#ifdef SR200T_Mobile_Mainline_01_E1_01_PROD_bin
#   include <SR200T_Mobile_Mainline_01.E1.01_PROD.h>
#endif

#ifdef SR250T_IOT_01_E2_01_PROD_bin
#   include <SR250T_IOT_01.E2.01_PROD.h>
#endif

#ifdef SR250_A1V2_01_31_C0_DEV_bin
#   include <SR250_A1V2_01.31.C0_DEV.h>
#endif

#ifdef SR200T_Mobile_Mainline_01_E1_01_DEV_bin
#   include <SR200T_Mobile_Mainline_01.E1.01_DEV.h>
#endif

#ifdef SR250T_IOT_01_E2_01_DEV_bin
#   include <SR250T_IOT_01.E2.01_DEV.h>
#endif

#ifdef SR200A1V2_IOT_01_41_F0_Prod_bin
#   include <SR200A1V2_IOT_01.41.F0_Prod.h>
#endif

#ifdef SR250A1V2_IOT_01_41_01_Prod_bin
#   include <SR250A1V2_IOT_01.41.01_Prod.h>
#endif

#ifdef SR200A1V2_01_41_02_Prod_bin
#   include <SR200A1V2_01_41_02_Prod.h>
#endif

#ifdef SR200A1V2_01_41_03_Prod_bin
#   include <SR200A1V2_01_41_03_Prod.h>
#endif

#ifdef SR200A1V2_IOT_01_41_04_Prod_bin
#   include <SR200A1V2_IOT_01.41.04_Prod.h>
#endif

#ifdef SR250_A1V2_01_E3_01_IOT_Prod_bin
#   include <SR250_A1V2_01.E3.01_IOT_Prod.h>
#endif

#ifdef SR200A1V2_01_31_C1_RC1_customer_bin
#   include <SR200A1V2_01.31.C1_RC1_customer.h>
#endif

#ifdef SR200A1V2_01_31_C3_customer_bin
#   include <SR200A1V2_01.31.C3_customer.h>
#endif

#ifdef SR200A1V2_01_31_C4_customer_bin
#   include <SR200A1V2_01.31.C4_customer.h>
#endif

#ifdef SR200A1V2_01_31_C5_customer_bin
#   include <SR200A1V2_01.31.C5_customer.h>
#endif

#ifdef SR200A1V2_01_31_C3_Development_bin
#   include <SR200A1V2_01_31_C3_Development.h>
#endif

#ifdef SR200A1V2_01_31_C4_development_bin
#   include <SR200A1V2_01.31.C4_development.h>
#endif

#ifdef SR200A1V2_01_31_C5_development_bin
#   include <SR200A1V2_01_31_C5_development.h>
#endif

#ifdef SR200A1V2_01_31_C1_RC1_devlopment_bin
#   include <SR200A1V2_01.31.C1_RC1_devlopment.h>
#endif

#ifdef SR200A1V2_IOT_01_41_F0_Dev_bin
#   include <SR200A1V2_IOT_01.41.F0_Dev.h>
#endif

#ifdef SR250A1V2_IOT_01_41_01_Dev_bin
#   include <SR250A1V2_IOT_01.41.01_Dev.h>
#endif

#ifdef SR200A1V2_01_41_02_Dev_bin
#   include <SR200A1V2_01_41_02_Dev.h>
#endif

#ifdef SR200A1V2_01_41_03_Dev_bin
#   include <SR200A1V2_01_41_03_Dev.h>
#endif

#ifdef SR200A1V2_IOT_01_41_04_Dev_bin
#   include <SR200A1V2_IOT_01.41.04_Dev.h>
#endif

#ifdef SR250_A1V2_01_E3_01_IOT_Dev_bin
#   include <SR250_A1V2_01.E3.01_IOT_Dev.h>
#endif

#endif // MAINLINE_FIRMWARE_H
