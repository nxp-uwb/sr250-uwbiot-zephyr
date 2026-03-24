
/* Copyright 2022 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */
#ifndef _UWB_FWDL_PROVIDER_H_
#define _UWB_FWDL_PROVIDER_H_

#if UWBIOT_UWBD_SR1XXT
#include "phUwbStatus.h"
#include "phNxpUciHal_fwd.h"

/** @defgroup uwb_fwdl_provider_t UWB FwDl Context Management
 *
 * @{
 */

/** Mode of operation of the Firmware download
 *
 */
typedef enum uwb_fwdl_mode
{
    UWB_FWDL_FACTORY,
    UWB_FWDL_MAINLINE,
} uwb_fwdl_mode_t;

/**
 * @brief Context for the Firmware Download
 *
 *
 */
typedef struct uwb_fwdl
{
    phHbci_MosiApdu_t uwb_fwdl_MosiApdu;
    phHbci_MisoApdu_t uwb_fwdl_MisoApdu;
    Options_t uwb_fwdl_gOpts;

    const uint8_t *fwImgPtr;
    uint32_t fwSize;
    uwb_fwdl_mode_t uwb_fwdl_mode;

} uwb_fwdl_provider_t;

/** @} */

/** @brief  This function is used to download the firmware on the Helios.
 *
 *
 * @param      pCtx     The context
 */
UWBStatus_t uwb_fwdl_downloadFw(uwb_fwdl_provider_t *pCtx);
#endif // UWBIOT_UWBD_SR1XXT
#if UWBIOT_UWBD_SR1XXT_SR2XXT
/** @brief  This function sets the Firmware Download context with firmware image and the firmware size.
 *
 *
 * @param      pAppfwImageCtx     - Firmware Image Context
 * @return     UWBStatus_t        - 0: success, 1: failure
 */
UWBStatus_t uwb_fwdl_setFwImage(const phUwbFWImageContext_t *const pAppfwImageCtx);
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT
#endif //_UWB_FWDL_PROVIDER_H_