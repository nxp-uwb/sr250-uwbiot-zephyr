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

#ifndef __UWB_BUS_FLASH_INTERFACE_H__
#define __UWB_BUS_FLASH_INTERFACE_H__

#include "uwb_board_flash.h"

#define MAINLINE_FW_CRC_ADDR MAINLINE_FW_CRC_START_ADDR
#define MAINLINE_FW_LEN_ADDR MAINLINE_FW_LEN_START_ADDR
#define MAINLINE_FW_ADDR     MAINLINE_FW_START_ADDR

#define FACTORY_FW_CRC_ADDR FACTORY_FW_CRC_START_ADDR
#define FACTORY_FW_LEN_ADDR FACTORY_FW_LEN_START_ADDR
#define FACTORY_FW_ADDR     FACTORY_FW_START_ADDR

/** @defgroup uwb_flash UWB FLASH Interface
 *
 * This layer depends on ``uwb_board_flash.h``, which is board specific.
 *
 * ``uwb_board_flash.h`` must implement / define the following
 *
 * - structure ``uwb_bus_flash_ctx``
 *
 * The implementation of all these APIs would be board specific and
 * tightly coupled to the board.
 *
 * @{
 */

/** @defgroup uwb_bus_ctx UWB BUS Context Management
 *
 * @{
 */

/** Initailize Bus interface
 *
 * When this API is called
 *
 * - Bus interface (SPIFI) is initialised.
 *
 * @param      pCtx  The context
 */
uwb_bus_status_t uwb_bus_flash_init(uwb_bus_flash_ctx_t *pCtx);

/** De-Initialize the bus interface and free up references and context
 *
 * When this API is called

 * - Bus interface (SPIFI) is de-Initialized.
 *
 * @param      pCtx  The context
 */
uwb_bus_status_t uwb_bus_flash_deinit(uwb_bus_flash_ctx_t *pCtx);

/** Reset the command field of flash interface
 *
 * @param      pCtx  The context
 */
uwb_bus_status_t uwb_bus_flash_reset(uwb_bus_flash_ctx_t *pCtx);

/** Check the write status of flash interface
 *
 * @param      pCtx  The context
 */
uwb_bus_status_t uwb_bus_flash_status(uwb_bus_flash_ctx_t *pCtx);

/** @} */

/** @defgroup uwb_bus_flash FLASH Interface Transfer
 *
 * @{
 */

/** Transmit a data frame
 *
 * @param      pCtx    The context
 * @param[in]  pBuf    The data that we want to transmit
 *
 * @retval kUWB_bus_Status_OK
 * @retval kUWB_bus_Status_FAILED
 */
uwb_bus_status_t uwb_bus_flash_write(
    uwb_bus_flash_ctx_t *pCtx, const uint8_t *pBuf, uint32_t bufLen, uint32_t StartAddr);
/** Receive a data frame
 *
 * @param      pCtx       The context
 * @param[out] pBuf       The pointer where we copy received data
 * @param[in]  bufLen     The Length of the data to be written.
 * @param[in]  StartAddr  Input: Address from which data to be read.
 *
 * @retval kUWB_bus_Status_OK
 * @retval kUWB_bus_Status_FAILED
 */

uwb_bus_status_t uwb_bus_flash_read(uwb_bus_flash_ctx_t *pCtx, uint8_t *pBuf, uint32_t bufLen, uint32_t StartAddr);

/** Erase the required number of blocks
 *
 * @param      pCtx       The context
 * @param      StartAddr  The address of the page to be erased
 * @param      FwLen      The FW length
 *
 * @retval kUWB_bus_Status_OK
 * @retval kUWB_bus_Status_FAILED
 */
uwb_bus_status_t uwb_bus_flash_erase(uwb_bus_flash_ctx_t *pCtx, uint32_t StartAddr, uint32_t FwLen);

/** @} */

#endif // __UWB_BUS_FLASH_INTERFACE_H__
