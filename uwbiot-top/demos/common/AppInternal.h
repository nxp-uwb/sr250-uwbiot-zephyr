/* Copyright 2019,2020,2023-2026 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef APP_INTERNAL_H_
#define APP_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#if defined(CPU_QN9090)
#include <fsl_wwdt.h>
#endif

#include "phUwbTypes.h"
#include "UwbApi_Types.h"
#include <uwb_board.h>
#include "AppStateManagement.h"
#include "PrintUtility.h"
#include "UwbApi.h"
#include <stdlib.h>
#include "phNxpLogApis_App.h"
#include "uwbiot_ver.h"
#include "Demo_Common_Config.h"

#if (UWBIOT_UWBD_SR04X)
#include "Demo_SR040_Swup_Update.h"
#include "Demo_SR040_RadioConfigAndGroupDelay.h"
#endif // UWBIOT_UWBD_SR04X

#define DEMO_NTF_HANDLE_TASK_NAME "demo_ntf_handling_task"
#if UWBIOT_OS_ZEPHYR
#define DEMO_NTF_HANDLE_TASK_SIZE CONFIG_NXP_UWB_NTF_STACK_SIZE
#else
#define DEMO_NTF_HANDLE_TASK_SIZE 1800
#endif
#define DEMO_NTF_HANDLE_TASK_PRIO 5

/* 0 ==> Use BPRF, 1 ==> Use HPRF; for this demo.  */
#define APP_INTERNAL_USE_HPRF 0
/* In HPRF, whether to use 7.8 or 6.8M
 *
 * 1 == kUWB_PsduDataRate_7_80Mbps
 * 0 == kUWB_PsduDataRate_6_81Mbps */
#define APP_INTERNAL_USE_HPRF_PSDU_DATARATE_78M 0

#if APP_INTERNAL_USE_HPRF
/* Data size to be sent and expected to receive over RF*/
/* PSDU Data[0:N] bytes : 0 <= N <= 4095 for HPRF */
#define PSDU_DATA_SIZE 1023
/* Either 1 or 2. Only valid if using HPRF */
#define APP_INTERNAL_USE_NUMBER_OF_STS_SEGMENTS 1
#else
/* PSDU Data[0:N] bytes : 0 <= N <= 127 for BPRF*/
#define PSDU_DATA_SIZE 127
#endif

#define TEST_TASK_PRIORITY 4
/* 2021.04.19. Reduced from 2048 to 1536 */
#define TEST_TASK_STACK_SIZE 1536
#define TEST_TASK_NAME       "TestTask"

#define RECOVERY_TASK_PRIORITY   5
#define RECOVERY_TASK_STACK_SIZE 300
#define RECOVERY_TASK_NAME       "RecoveryTask"
#define MAX_PKT_SIZE             2031

#define P1_VALUE_DEFAULT        (0x00)
#define P1_VALUE_OPTIMIZED_AUTH (0x80)

#define Log LOG_I

extern void *perSem;
extern void *rangingDataSem;
extern void *inBandterminationSem;
extern void *datatransferNtfSemRx;
extern void *datatransferNtfSemTx;
extern void *dataRcvNtfSemTx;
extern void *RadarTstAntIsoNtfSemTx;
extern void *testLoopBackNtfSem;
extern void *llCreateStatusSem;
extern void *llUwbsCloseNtf;
extern uint8_t data_buff[];

#if !(UWBIOT_UWBD_SR04X)
#define GENERATE_SEND_DATA(x, y)           \
    {                                      \
        for (uint16_t i = 0; i < y; i++) { \
            x[i] = (i + 1);                \
        }                                  \
    }

extern uint8_t dataToSend[UWBS_MAX_UCI_PACKET_SIZE];
extern uint8_t dataReceived[UWBS_MAX_UCI_PACKET_SIZE];

/**
 * \brief  Function used to validate whether PSDU/APP data received is same as that of data sent
 */
/*@{ */
EXTERNC tUWBAPI_STATUS validateReceivedData();
/* @}*/
#endif

EXTERNC void AppCallback(eNotificationType opType, void *pData, size_t len);
EXTERNC void AppCallback_cdc(eNotificationType opType, void *pData, size_t len);

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

/**
 * @brief      End of the example.
 *
 * @param      status MW status from tUWBAPI_STATUS
 *
 * @return     NA
 *
 * If running on MSVC, this program will exit.
 */

#if UWBIOT_OS_FREERTOS || UWBIOT_OS_ZEPHYR
#define RET_VALUE return
#else
#define RET_VALUE return 0
#endif

#if defined(_MSC_VER) || defined(__linux__) || defined(__MINGW32__) || defined(__MINGW64__) || \
    defined(__apple_build_version__) || defined(__ZEPHYR__)

#if UWBIOT_OS_FREERTOS || UWBIOT_OS_ZEPHYR

#define UWBIOT_EXAMPLE_END(status)                         \
    if (status == UWBAPI_STATUS_OK) {                      \
        LOG_I("OK\r\nFinished %s : Success!\n", __FILE__); \
        exit(0);                                           \
    }                                                      \
    else {                                                 \
        LOG_E("\nFinished %s : Failed!\n", __FILE__);      \
        exit(1);                                           \
    }                                                      \
    RET_VALUE;

#else
#define UWBIOT_EXAMPLE_END(status)                         \
    if (status == UWBAPI_STATUS_OK) {                      \
        LOG_I("OK\r\nFinished %s : Success!\n", __FILE__); \
    }                                                      \
    else {                                                 \
        LOG_E("\nFinished %s : Failed!\n", __FILE__);      \
    }                                                      \
    return NULL;
#endif // UWBIOT_OS_FREERTOS
#elif defined(CPU_QN9090)

#define UWBIOT_EXAMPLE_END(status)                 \
    if (status == UWBAPI_STATUS_OK) {              \
        LOG_I("Finished %s : Success!", __FILE__); \
    }                                              \
    else {                                         \
        LOG_E("Finished %s : Failed!", __FILE__);  \
    }                                              \
    __disable_irq();                               \
    while (1) {                                    \
        WWDT_Refresh(WWDT);                        \
        __WFI();                                   \
    }                                              \
    RET_VALUE
#else /* Non windows linux */

#define UWBIOT_EXAMPLE_END(status)                 \
    if (status == UWBAPI_STATUS_OK) {              \
        LOG_I("Finished %s : Success!", __FILE__); \
    }                                              \
    else {                                         \
        LOG_E("Finished %s : Failed!", __FILE__);  \
    }                                              \
    __disable_irq();                               \
    while (1) {                                    \
        __WFI();                                   \
    }                                              \
    RET_VALUE
#endif // ! _MSC_VER

EXTERNC void UWBDemo_Init(void);
EXTERNC void UWBDemo_DeInit(void);

/* Ntf Handling related functions*/
void demo_handle_ntf(phLibUwb_Message_t *evt);
EXTERNC void demo_ntf_handling_task_start(void);
EXTERNC void demo_ntf_handling_task_stop(void);
EXTERNC void demo_handle_error_scenario(eNotificationType opType);
#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif // APP_INTERNAL_H_
