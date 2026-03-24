/*
 * Copyright 2019,2020,2023-2026 NXP
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

#ifndef UWBPNPINTERNAL_H_
#define UWBPNPINTERNAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "UWB_Evt_Pnp.h"
#include "phUwbTypes.h"
#include "phOsalUwb.h"
#include "uwb_board.h"
#if UWBIOT_OS_ZEPHYR
#define WRITER_QUEUE_SIZE configTML_QUEUE_LENGTH
#else
#define WRITER_QUEUE_SIZE 100
#endif // UWBIOT_OS_ZEPHYR
extern void *mHifWriteMutex;
extern intptr_t mHifWriteQueue;
extern void *mHifIsr_Sem;
extern intptr_t mHifCommandQueue;
extern UWBOSAL_TASK_HANDLE mHifTask;
extern UWBOSAL_TASK_HANDLE mPnpAppTask;
extern UWBOSAL_TASK_HANDLE mUciReaderTask;
extern UWBOSAL_TASK_HANDLE mHifWriterTask;
extern void *mHifSyncMutex;
uint32_t UWB_Hif_UciSendNtfn(uint8_t *pData, uint16_t size);
uint32_t UWB_Hif_SendUCIRsp(uint8_t *pData, uint16_t size);
uint32_t UWB_Hif_SendRsp(uint8_t *pData, uint16_t size);
void Uwb_Hif_ReadDataCb(uint8_t *pData, uint32_t *pLen);
uint32_t UWB_Hif_internal_SendRsp(uint8_t *pData, uint16_t size);
bool Uwb_Is_Hif_Active();
void Uwb_Reset_Hif_State(bool state);

extern void UCI_ReaderTaskSuspend(void);
extern void UCI_ReaderTaskResume(void);

#if (UWBFTR_SE_SE051W || UWBIOT_SESN_SNXXX)
void Se_Comm_Init(void);
void Se_Transceive(uint8_t *cmd, uint16_t cmdLen, uint8_t *resp, size_t *respLen);
#endif

/* HIF header + max UCI pkt size + UCI header= 4132 + 8 byte extra buffer
 * For HDLL based firmware download, max packet size = 4165 (HDLL header + HDLL Group Operation + HDLL Max Payload +
 * CRC) Size increased to 5*1024 bytes of data
 */
#define HIF_MAX_PKT_SIZE (5 * 1024)

#define UWB_MAX_HELIOS_RSP_SIZE 64

#if UWBIOT_UWBD_SR1XXT
void UWB_Handle_SR1XXT_TLV(tlv_t *tlv);
#endif
#if UWBIOT_UWBD_SR2XXT
void UWB_Handle_SR2XXT_TLV(tlv_t *tlv);
#endif
#if (UWBIOT_UWBD_SR04X)
void UWB_Handle_SR040_TLV(tlv_t *tlv); /* TODO: SR048 Update Required */
#endif
// void UCI_ReaderTask(void *args);
// void UWB_USBSetResponse(uint8_t type, uint8_t *data, uint16_t data_size);

// EXTERNC bool UWB_HeliosCE(bool set);

#if UWBIOT_UWBD_SR150
#define UWBIOT_UWBS_NAME "SR150"
#elif UWBIOT_UWBD_SR040
#define UWBIOT_UWBS_NAME "SR040"
#elif UWBIOT_UWBD_SR100T
#define UWBIOT_UWBS_NAME "SR100T"
#elif UWBIOT_UWBD_SR100S
#define UWBIOT_UWBS_NAME "SR100S"
#elif UWBIOT_UWBD_SR200T
#define UWBIOT_UWBS_NAME "SR200T"
#elif UWBIOT_UWBD_SR250
#define UWBIOT_UWBS_NAME "SR250"
#elif UWBIOT_UWBD_SR200S
#define UWBIOT_UWBS_NAME "SR200S"
#elif UWBIOT_UWBD_SR048M
#define UWBIOT_UWBS_NAME "SR048M"
#else
#define UWBIOT_UWBS_NAME "Unknown"
#endif

#define PRINT_APP_NAME(szAPP_NAME)                                   \
    PRINTF("#################################################\r\n"); \
    PRINTF("## " szAPP_NAME " : " UWBIOT_UWBS_NAME "\r\n");          \
    PRINTF("## " UWBIOTVER_STR_PROD_NAME_VER_FULL "\r\n");           \
    PRINTF("#################################################\r\n")

#if UWBIOT_OS_ZEPHYR
#define HIF_TASK_SIZE        2048
#define PNP_APP_TASK_SIZE    2048
#define UCI_READER_TASK_SIZE 2048
#define HIF_WRITER_TASK_SIZE 2048
#else
#define HIF_TASK_SIZE        512
#define PNP_APP_TASK_SIZE    512
#define UCI_READER_TASK_SIZE 512
#define HIF_WRITER_TASK_SIZE 512
#endif // UWBIOT_OS_ZEPHYR
#define TRACE_UCI  PRINTF
#define TRACE_HBCI PRINTF

#define HDR_GID_MASK 0xF0
#define HDR_MTS_RSP  0x40
#define HDR_MTS_NTF  0x60
#define HDR_MTS_PBF  0x70

#define HDR_DPF_RCV_MASK 0x0F
#define HDR_DPF_RCV      0x02

#define MT_UCI_DATA        0
#define MT_UCI_CMD         1
#define MT_UCI_RSP         2
#define MT_UCI_NTF         3
#define MT_ESE_CTRL_CMD    4
#define MT_ESE_CTRL_RSP    5
#define MT_ESE_END         6
#define MCTT_ESE_CMD       1
#define UCI_HDR_LEN        4
#define UCI_EXT_LEN_IND    1
#define UCI_LEN_OFFSET     3
#define UCI_EXT_LEN_OFFSET 2
#define UCI_EXT_LEN_MASK   0x80
#define UCI_MT_MASK        0xE0
#define UCI_MT_SHIFT       0x05

#define UCI_MTS_CHECK(X) ((X)&UCI_MT_MASK) >> UCI_MT_SHIFT

/** Check weather message type is response*/
#define RSP_MTS_CHECK(X) (((X & HDR_GID_MASK) == HDR_MTS_RSP) ? 1 : 0)

/** Check weather message type is ntf*/
#define NTF_MTS_CHECK(X) (((X & HDR_GID_MASK) == HDR_MTS_NTF) ? 1 : 0)

/** Check weather message type is pbf is enabled*/
#define PBF_MTS_CHECK(X) (((X & HDR_GID_MASK) == HDR_MTS_PBF) ? 1 : 0)

/** Check weather DATA PACKET or NOT*/
#define DPF_RCV_CHECK(X) (((X & HDR_DPF_RCV_MASK) == HDR_DPF_RCV) ? 1 : 0)

/** Handle protocol error, board specific */
void uwb_pnp_board_protocol_error_handler(void);

/** In order to handle timeouts, reload the timers at start of new valid packet */
void uwb_pnp_reload_timer(void);
extern void PRINTF_WITH_TIME(const char *fmt, ...);

void PRINTF_WITH_TIME(const char *fmt, ...);
#endif /* UWBPNPINTERNAL_H_ */