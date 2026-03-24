/*
 * Copyright 2012-2023, 2025,2026 NXP.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Basic type definitions */
#ifndef __PH_TML_UWB_TRANSPORT_H__
#define __PH_TML_UWB_TRANSPORT_H__

#include "phTmlUwb.h"
#include "phUwbTypes.h"
#include "uwb_uwbs_tml_io.h"

#define SR100_MAGIC          0xEA
#define SR100_SET_PWR        _IOW(SR100_MAGIC, 0x01, long)
#define SR100_SET_DBG        _IOW(SR100_MAGIC, 0x02, long)
#define SR100_GET_THROUGHPUT _IOW(SR100_MAGIC, 0x05, long)

#define SR200_MAGIC          0xEA
#define SR200_SET_PWR        _IOW(SR200_MAGIC, 0x01, long)
#define SR200_SET_DBG        _IOW(SR200_MAGIC, 0x02, long)
#define SR200_GET_THROUGHPUT _IOW(SR200_MAGIC, 0x05, long)

#define PWR_DISABLE        0
#define PWR_ENABLE         1
#define ABORT_READ_PENDING 2

#define NORMAL_MODE_HEADER_LEN 4
#define NORMAL_MODE_LEN_OFFSET 3

#define EXTENDED_SIZE_LEN_OFFSET 1
#define UCI_EXTENDED_PKT_MASK    0xC0
#define UCI_EXTENDED_SIZE_SHIFT  6
#define UCI_NORMAL_PKT_SIZE      0
#define UCI_EXT_PKT_SIZE_512B    1
#define UCI_EXT_PKT_SIZE_1K      2
#define UCI_EXT_PKT_SIZE_2K      3

#define UCI_PKT_SIZE_512B 512
#define UCI_PKT_SIZE_1K   1024
#define UCI_PKT_SIZE_2K   2048

/* TML UWB Context */
extern phTmlUwb_Context_t *gpphTmlUwb_Context;
extern uwb_uwbs_tml_ctx_t gUwbsTmlCtx;

/*Global Function declarations */

/**
**
** Function         phTmlUwb_open_and_configure
**
** Description      Open and configure helios device
**
** Parameters       pConfig     - hardware information
**                  pLinkHandle - device handle
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - open_and_configure operation success
**                  UWBSTATUS_INVALID_DEVICE - device open operation failure
**
*/
UWBSTATUS phTmlUwb_open_and_configure(pphTmlUwb_Config_t pConfig, void **pLinkHandle);

/**
** Function         phTmlUwb_io_init
**
** Description      initialise uwbs io
**
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_io_init operation success
**                  UWBSTATUS_INVALID_DEVICE - device open operation failure
**
*/
UWBSTATUS phTmlUwb_io_init();

/**
** Function         phTmlUwb_io_deinit
**
** Description      De initialise uwbs io
**
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_io_init operation success
**                  UWBSTATUS_INVALID_DEVICE - device open operation failure
**
*/
UWBSTATUS phTmlUwb_io_deinit();

/**
** Function         phTmlUwb_io_set
**
** Description      set uwbs io
**
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_io_init operation success
**                  UWBSTATUS_INVALID_DEVICE - device open operation failure
**
*/
UWBSTATUS phTmlUwb_io_set(uwbs_io_t ioPin, bool_t value);

/* TODO: To Be Cleaned-Up*/
#if 0
/**
** Function         phTmlUwb_io_enable_irq
**
** Description      enable uwbs irq
**
** Parameters       irqPin     - irq to enable
**                  cb         - callback
**                  args       - callback args
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_io_init operation success
**                  UWBSTATUS_INVALID_DEVICE - device open operation failure
**
*/
void phTmlUwb_io_enable_irq(uwbs_io_t irqPin, void (*cb)(void *args), void *args);
#endif // 0

// TODO: Temporary for sn110 and sr1xx simultaneous irq enblement
#if UWBIOT_SESN_SNXXX
void phTmlUwb_io_enable_uwb_irq();
#endif

/* TODO: To Be Cleaned-Up*/
#if 0
/**
** Function         phTmlUwb_io_disable_irq
**
** Description      disable uwbs irq
**
** Parameters       irqPin     - irq to disable
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_io_init operation success
**                  UWBSTATUS_INVALID_DEVICE - device open operation failure
**
*/
void phTmlUwb_io_disable_irq(uwbs_io_t irqPin);
#endif // 0

/**
** Function         phTmlUwb_close
**
** Description      SPI Cleanup
**
**
** Returns          None
**
*/
void phTmlUwb_close();

/**
** Function         phTmlUwb_uci_read
**
** Description      Reads requested number of bytes from SR100 device into given
**                  buffer
**
** Parameters       pBuffer          - buffer for read data
**                  nNbBytesToRead   - number of bytes requested to be read
**
** Returns          numRead   - number of successfully read bytes
**                  -1        - read operation failure
**
*/
int phTmlUwb_uci_read(uint8_t *pBuffer, int nNbBytesToRead);

/**
** Function         phTmlUwb_uci_write
**
** Description      Writes requested number of bytes from given buffer into
**                  SR100 device
**
** Parameters       pBuffer          - buffer for read data
**                  nNbBytesToWrite  - number of bytes requested to be written
**
** Returns          numWrote   - number of successfully written bytes
**                  -1         - write operation failure
**
*/
int phTmlUwb_uci_write(uint8_t *pBuffer, uint16_t nNbBytesToWrite);

/**
** Function         phTmlUwb_rci_read
**
** Description      Reads requested number of bytes from SR040 device into given
**                  buffer using SWUP protocol
**
** Parameters       pBuffer          - buffer for read data
**
** Returns          nNbBytesToRead   - number of successfully read bytes
**                  -1        - read operation failure
**
*/
int phTmlUwb_rci_read(uint8_t *pBuffer, int nNbBytesToRead);

/**
** Function         phTmlUwb_rci_write
**
** Description      Writes requested number of bytes from given buffer into
**                  SR040 device in SWUP mode
**
** Parameters       pBuffer          - buffer to write
**                  nNbBytesToWrite  - number of to write
**
** Returns          numWrote   - number of successfully written bytes
**                  -1         - write operation failure
**
*/
int phTmlUwb_rci_write(uint8_t *pBuffer, uint16_t nNbBytesToWrite);

#if UWBIOT_UWBD_SR2XXT
/**
** Function         phTmlUwb_hdll_read
**
** Description      Reads requested number of bytes from SR200 device into given
**                  buffer using HDLL protocol
**
** Parameters       pBuffer          - buffer for read data
**
** Returns          numRead   - number of successfully read bytes
**                  -1        - read operation failure
**
*/
int phTmlUwb_hdll_read(uint8_t *pBuffer, uint16_t *pRspBufLen);

/**
** Function         phTmlUwb_hdll_transceive
**
** Description      HDLL Write read operation for SR2XXT devices for FW download
**
** Parameters       pWriteBuf        - buffer to write
**                  writeBufLen  - number of bytes to written
**                  pRespBuf     - response buffer
**                  pRspBufLen   - response bufferLen
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_hdll_transceive operation success
**                  UWBSTATUS_FAILED - phTmlUwb_hdll_transceive failure
**
*/
UWBSTATUS phTmlUwb_hdll_transceive(uint8_t *pWriteBuf, size_t writeBufLen, uint8_t *pRespBuf, size_t *pRspBufLen);


#if (UWBIOT_TML_PNP || UWBIOT_TML_SOCKET)
/**
** Function         phTmlUwb_hdll_reset
**
** Description      Send HDLL Reset Command for SR2XXT devices. Resets device back to UCI mode
**
** Parameters       isFWDownloadDone  - true if FW download is in Done
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_hdll_transceive operation success
**                  UWBSTATUS_FAILED - phTmlUwb_hdll_transceive failure
**
*/
UWBSTATUS phTmlUwb_hdll_reset(bool isFWDownloadDone);
#endif //(UWBIOT_TML_PNP || UWBIOT_TML_SOCKET)
#endif // UWBIOT_UWBD_SR2XXT

/**
** Function         phTmlUwb_reset_uwbs
**
** Description      Reset UWBS device
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_reset_uwbs operation success
**                  UWBSTATUS_FAILED - phTmlUwb_reset_uwbs failure
**
*/
UWBSTATUS phTmlUwb_reset_uwbs(void);

/**
** Function         phTmlUwb_reset
**
** Description      Reset SR100 device, using VEN pin
**
** Parameters       level          - reset level
**
** Returns           0   - reset operation success
**                  -1   - reset operation failure
**
*/
int phTmlUwb_reset(long level);
#if (UWBIOT_UWBD_SR04X)
#if UWBIOT_TML_SPI
/**
** Function         phTmlUwb_flush_read_buffer
**
** Description      flush tml read buffer
**
**
**
*/
void phTmlUwb_flush_read_buffer(void);


/**
** Function         phTmlUwb_helios_irq_enable
**
** Description      enable uwbs irq
**
**
**
*/
void phTmlUwb_helios_irq_enable(void);

/**
** Function         phTmlUwb_rdy_read
**
** Description      get uwbs ready pin status
**
**
**
*/
bool phTmlUwb_rdy_read(void);
#endif // UWBIOT_TML_SPI
#endif /* (UWBIOT_UWBD_SR04X) */

#if UWBIOT_TML_S32UART
/**
** Function         phTmlUwb_switch_protocol
**
** Description      switch uwbs protocl
**
** Parameters       protocol       - protocol SWUP or UCI
**
*/
void phTmlUwb_switch_protocol(uint8_t protocol);
#endif // UWBIOT_TML_S32UART

/**
** Function         phTmlUwb_hbci_transceive
**
** Description      HBCI Write read operation for SR1xxT devices for FW download
**
** Parameters       pWriteBuf        - buffer to write
**                  writeBufLen  - number of bytes to written
**                  pRespBuf     - response buffer
**                  pRspBufLen   - response bufferLen
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_hbci_transceive operation success
**                  UWBSTATUS_FAILED - phTmlUwb_hbci_transceive failure
**
*/
UWBSTATUS phTmlUwb_hbci_transceive(uint8_t *pWriteBuf, size_t writeBufLen, uint8_t *pRespBuf, size_t *pRspBufLen);

/**
** Function         phTmlUwb_set_mode_uci
**
** Description      Set TML transfer mode to UCI
**
*/
void phTmlUwb_set_mode_uci(void);

/**
** Function         phTmlUwb_set_mode_fwdld
**
** Description      Set appropriate FW download TML transfer mode.
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - phTmlUwb_set_mode_fwdld operation success
**                  UWBSTATUS_FAILED - phTmlUwb_set_mode_fwdld failure
*/
UWBSTATUS phTmlUwb_set_mode_fwdld(void);
#endif //__PH_TML_UWB_TRANSPORT_H__
