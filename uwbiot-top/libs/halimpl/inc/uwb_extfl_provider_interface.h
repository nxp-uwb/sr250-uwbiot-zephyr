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

#ifndef __UWB_FWDL_PROVIDER_INTERFACE_H__
#define __UWB_FWDL_PROVIDER_INTERFACE_H__

#if UWBIOT_UWBD_SR1XXT
#include "uwb_board_flash_interface.h"
#include "phUwbStatus.h"

#define DataError (200)
/** @defgroup uwb_uwbs UWBS Specific Transport Interface
 *
 * This layer depends on ``uwb_board_flash_interface.h``
 *
 * The implementation of all these APIs would be UWBD and Transport specific.
 *
 *
 * @{
 */

/** @defgroup uwb_fwdl_provider_ctx_t UWB BUS Context Management
 *
 * @{
 */

/** Command to read/write the Flash.
 *
 */
typedef enum
{
    READ_CMD_SET,
    WRITE_CMD_SET
} uwb_fwdl_provider_cmd_t;

/**
 * @brief Context for the Transport Layer
 *
 * This allows management of data / layer information.
 *
 */
typedef struct
{
    uwb_bus_flash_ctx_t flashCtx;
    /** tml interface read write mode */
    eFirmwareMode mode;
    /*To store Addresses to read/write data from/to external flash*/
    uint32_t flashStartAddr, fwStartAddr, fwLenAddr, fwCrcAddr;

} uwb_fwdl_provider_ctx_t;

/** @} */

/** Initialize the Bus interface and the context
 *
 * bus interface (SPIFI) is initialised.
 *
 * @param      pCtx     The context
 */
UWBStatus_t uwb_fwdl_provider_init(uwb_fwdl_provider_ctx_t *pCtx);

/** De-Initialize the context and free up references
 *
 * @param      pCtx  The context
 */
UWBStatus_t uwb_fwdl_provider_deinit(uwb_fwdl_provider_ctx_t *pCtx);

/** Set the mode to MAINLINE / FACTORY Firmware Download.
 *
 * @param      pCtx  The context
 * @param      mode  See @ref eFirmwareMode
 */
bool uwb_fwdl_provider_setmode(uwb_fwdl_provider_ctx_t *pCtx, eFirmwareMode mode);

/** Query the FW length
 *
 * @param      pCtx    The context
 * @param[out] pfwLen  Firmware length
 *
 */
UWBStatus_t uwb_fwdl_provider_get_fwLength(uwb_fwdl_provider_ctx_t *pCtx, uint32_t *pfwLen);

/** Query the CRC of Flashed FW
 *
 * @param      pCtx    The context
 * @param[out] pFwCrc  Firmware CRC

 */
UWBStatus_t uwb_fwdl_provider_crc_check(uwb_fwdl_provider_ctx_t *pCtx, uint16_t *pFwCrc);

/** Generate the CRC of the FW
 *
 * @param      pCtx    The context
 * @param[in]  pBuf    The data whose CRC to be generated
 * @param[in]  bufLen  Firmware length
 * @param[out] pCrc    the Generated Crc
 */
UWBStatus_t uwb_fwdl_provider_generate_crc(
    uwb_fwdl_provider_ctx_t *pCtx, const uint8_t *pBuf, const uint32_t bufLen, uint16_t *pCrc);

/** Generate the Firmware details
 *
 * To generate the Fw details to be stored in the 1st 6 bytes of the ext flash
 *
 * @param      pCtx    The context
 * @param[out] pBuf    The generated firmware details
 * @param[in]  pCrc    The Generated Crc
 * @param[in]  bufLen  The Firmware length
 */
UWBStatus_t uwb_fwdl_provider_generate_fwDetails(
    uwb_fwdl_provider_ctx_t *pCtx, uint8_t *fwDetails, uint16_t Crc, uint32_t fwLen);

/** Erase required number of blocks of the flash
 *
 * @param      pCtx    The context
 * @param[in]  FwLen   Firmware length

 */
UWBStatus_t uwb_fwdl_provider_flash_erase(uwb_fwdl_provider_ctx_t *pCtx, const uint32_t FwLen);

/** Firmware verification
 *
 * @param           pCtx        The context
 * @param           pBuf        The pointer where we copy received data
 * @param           StartAddr   The address to read data from external flash
 *
 * @return     Status of fw verification
 */
UWBStatus_t uwb_fwdl_provider_verify_fw(
    uwb_fwdl_provider_ctx_t *pCtx, const uint8_t *pBuf, const uint32_t bufLen, uint32_t StartAddr);

/** @} */

/** @defgroup uwb_fwdl_provider_data UWB BUS DATA Interface
 *
 * @{
 */

/** Transmit the data.
 *
 * @param      pCtx    The context
 * @param[in]  pBuf    The data that we want to transmit
 * @param[in]  bufLen  The data length
 *
 * @return     Status of Transmit
 */
UWBStatus_t uwb_fwdl_provider_data_write(uwb_fwdl_provider_ctx_t *pCtx, const uint8_t *pBuf, size_t bufLen);

/** Receive a data frame.
 *
 * @param           pCtx     The context
 * @param[out]      pBuf     The pointer where we copy received data
 * @param[in]       bufLen   Input: The max length that we can copy.
 *
 * @return     Status of Receive
 */
UWBStatus_t uwb_fwdl_provider_data_read(uwb_fwdl_provider_ctx_t *pCtx, uint8_t *pBuf, uint32_t bufLen);

/** Trans-Receive a data frame.
 *
 * Transmit and receive happens at the same time for this frame.
 *
 * @param          pCtx     The context
 * @param[in]      pTxBuf   Buffer to be transmitted
 * @param[in]      txBufLen transmit buffer length
 * @param[out]     pRxBuf   The pointer where we copy received data
 * @param[in,out]  RxBufLen Input: The max length that we can copy.
 *
 * @return     Status of Tx/Rx
 */
UWBStatus_t uwb_fwdl_provider_data_trx(uwb_fwdl_provider_ctx_t *pCtx,
    const uint8_t *pTxBuf,
    size_t txBufLen,
    uint8_t *pRxBuf,
    size_t rxBufLen,
    uint32_t FlashAddr);

/** @} */ // uwb_fwdl_provider_data
#endif    /* UWBIOT_UWBD_SR1XXT */
#endif    /* __UWB_FWDL_PROVIDER_INTERFACE_H__ */
