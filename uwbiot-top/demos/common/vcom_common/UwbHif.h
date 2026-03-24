/* Copyright 2021-2026 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#include "phUwb_BuildConfig.h"

#ifndef _UWB_HIF_H_
#define _UWB_HIF_H_

#include "phOsalUwb.h"
#include "uwb_board.h"

#if defined(CPU_LPC55S69JBD100_cm33) || defined(CPU_LPC54628J512ET180) || defined(CPU_MIMXRT1176DVMAA) || \
    defined(CPU_RW612ETA2I)
#include "Uwb_usb.h"
#elif defined(NRF52_SERIES)
#if UWBIOT_OS_FREERTOS
#include "usart_nrf52840.h"
#endif // UWBIOT_OS_FREERTOS
#if UWBIOT_OS_ZEPHYR
#include "usart_nrf52840_zephyr.h"
#endif //
#elif defined(QN9090_SERIES)
#include "usart_vcom_qn9090.h"
#elif defined(CPU_S32K144)
#include "usart_s32k.h"
#elif (__linux__)
#include "uart_linux.h"
#include "uwb_socket.h"
#endif
typedef enum
{
    USB_TLV_EVT,
} UWB_EvtType_t;

typedef struct
{
    UWB_EvtType_t type;
    void *data;
} UWB_Evt_t;

typedef struct
{
    uint8_t uwbmode;
} UWB_Mode_t;

typedef enum
{
    kUWB_MODE_UNKNOWN,
    kUWB_MODE_MCTT
} UWB_MODES_t;

typedef enum __UWB_Hif_Comm_Mode
{
    kUWB_COMM_Serial,
    kUWB_COMM_Socket,
} UWB_Hif_Comm_Mode_t;

typedef enum _UWB_Hif_Error
{
    kUWB_HIF_ERR_Success,
    kUWB_HIF_ERR_Failed,
} UWB_Hif_Error;

#define HIF_TASK_NAME                 "HIF_TASK"
#define MAX_HIF_TASK_TLV_WAIT_TIMEOUT (1000)

#define HIF_TASK_PRIORITY      4
#define HIF_COMMAND_QUEUE_SIZE 10
#if (UWBIOT_UWBD_SR04X)
#if UWBIOT_OS_ZEPHYR
#define HIF_TASK_SIZE 512
#else
#define HIF_TASK_SIZE 256
#endif                         //UWBIOT_OS_ZEPHYR
#define HIF_RESP_BUFF_SIZE 512 // Max CIR payload data: 1024 + 4 byte session Id
#define HIF_MAX_PKT_SIZE   512
#else
#define HIF_TASK_SIZE      1024
#define HIF_RESP_BUFF_SIZE 4104 // In case of the HPRF
/**
 * As per fw max message size is 2039 (MAX_DATA_PACKET_PAYLOAD_SIZE)
 * extra 15 bytes of header
 * some 400 bytes extra allocated, not to crash the MW, if Tool send more Data.
 */
#define HIF_MAX_PKT_SIZE   2560
#endif // UWBIOT_UWBD_SR04X

#define HIF_RSP_HEADER_SIZE 3

#define HEADER_SIZE_MCTT    4
#define HEADER_SIZE_GENERIC 3

/*Macros related to  UCIs */
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

/* TLV Parser Offsets */
#define CMD_TYPE_OFFSET     0x00
#define CMD_SUB_TYPE_OFFSET 0x03

#define LSB_LENGTH_OFFSET 0x02
#define MSB_LENGTH_OFFSET 0x01
#define MSB_LENGTH_MASK   0x08

/* TLV types */
#define SESSION_MANAGEMENT   0x01
#define CONFIG_MANAGEMENT    0x02
#define RANGE_MANAGEMENT     0x03
#define DATA_MANAGEMENT      0x04
#define RF_TEST_MANAGEMENT   0x05
#define UWB_MISC             0x06
#define SE_MANAGEMENT        0x07
#define UWB_NTF_MANAGEMENT   0x08
#define UWB_SETUP_MANAGEMENT 0x09
#define UWB_MCTT_UCI_READY   0x0A // type for MCTT UCI_READY
#define FACTORY_MANAGEMENT   0x0B
#define TLV_TYPE_END         0x0C // Always keep END as last type + 1

#define TLV_TYPE_START    0x0B
#define SE_SELECT_APPLET  0x78
#define SE_DISPATCH       0x79
#define SE_TUNNEL_GETDATA 0x7A
#define SE_TUNNEL_PUTDATA 0x7B
#define SE_START_RANGING  0x7C
#define UART_ECHO         0x7D

/* SESSION Management Sub Commands */
#define SESSION_INIT       0x10
#define SESSION_DE_INIT    0x11
#define GET_SESSION_STATUS 0x12

/* Config Management Sub Commands */
#define SET_RANGING_PARAMS             0x20
#define GET_RANGING_PARAMS             0x21
#define SET_APP_CONFIG                 0x22
#define GET_APP_CONFIG                 0x23
#define SET_DEBUG_PARAMS               0x24
#define GET_DEBUG_PARAMS               0x25
#define SET_RF_TEST_PARAM              0x26
#define GET_PER_PARAMS                 0x27
#define GET_TEST_CONFIG                0x28
#define SET_TEST_CONFIG                0x29
#define SET_STATIC_STS                 0x2A
#define SET_DEVICE_CONFIG              0x2B
#define GET_DEVICE_CONFIG              0x2C
#define SET_APP_CONFIG_MULTIPLE_PARAMS 0x2D
#define GET_DEVICE_CAPABILITIES        0x2E
#define UPDATE_ACTIVE_ROUNDS_ANCHOR    0x2F
#define UPDATE_ACTIVE_ROUNDS_RECEIVER  0x41

/* Range Management Commands */
#define START_RANGING_SESSION            0X30
#define STOP_RANGING_SESSION             0X31
#define ENABLE_RANGING_DATA_NOTIFICATION 0X33
#define SESSION_UPDATE_MULTICAST_LIST    0x35
#define BLINK_DATA_TX                    0x36

/* RF TEST Management Commands */
#define START_PER_TX  0X50
#define START_PER_RX  0X51
#define STOP_RF_TEST  0X52
#define LOOPBACK_TEST 0x53
#define TEST_RX       0x54

/* MISC Commands */
#define GET_STACK_CAPABILITIES   0x60
#define GET_UWB_DEVICE_STATE     0x61
#define QUERY_TIMESTAMP          0x62
#define SEND_RAW_COMMAND         0x63
#define GET_ALL_UWB_SESSIONS     0x64
#define SET_CALIBRATION          0x65
#define GET_CALIBRATION          0x66
#define DO_CALIBRATION           0x67
#define QUERY_TEMPERATURE        0x68
#define GET_SOFTWARE_VERSION     0x69
#define GET_BOARD_ID             0x6A
#define VERIFY_DATA_CALIB_CMD    0x6F
#define WRITE_OTP_DATA_CALIB_CMD 0xA0
#define READ_OTP_DATA_CALIB_CMD  0xA1

/* SE_MANAGEMENT CMDS */
#define SE_TRANSRECEIVE       0x71
#define SE_OPEN_CHANNEL       0x72
#define SE_TEST_LOOP          0x73
#define SE_TEST_CONNECTIVITY  0x74
#define SE_DO_BIND            0x75
#define SE_GET_BINDING_STATUS 0x76
#define SE_GET_BINDING_COUNT  0x77
#define URSK_DELETION_REQ     0x78

/* UWB_NTF_MANAGEMENT */
#define MW_RECOVERY_NTF              0x80
#define RANGING_DATA_NTF             0x81
#define DATA_TRANSMISSION_STATUS_NTF 0x82
#define DATA_RECV_NTF                0x83
#define PER_TX_NTF                   0x84
#define PER_RX_NTF                   0x85
#define CIR_NTF                      0x86
#define DATALOGGER_NTF               0x88
#define DEBUG_LOG_NTF                0x89
#define RFRAME_DATA_NTF              0x8A
#define SCHEDULER_STATUS_NTF         0x8B
#define SESSION_STATUS_NTF           0x8C
#define RF_LOOPBACK_NTF              0x8D
#define MULTICAST_LIST_NTF           0x8E
#define BLINK_DATA_TX_NTF            0x8F
#define TEST_RX_NTF                  0x90

/* Setup Management Commands */
#define UWB_INIT          0x91
#define UWB_SHUTDOWN      0x92
#define UWB_FACTORY_TEST  0x93
#define UWB_MAINLINE_TEST 0x94
#define MCU_RESET         0x95 // MCU nvReset will be called.
#define UWB_NEGATIVE_TEST 0x96 // Negative scenarios will be called.

/* Data Management commands */
#define DATA_SEND_COMMAND 0x40
#define SE_SELECT_APPLET  0x78
#define SE_DISPATCH       0x79
#define SE_TUNNEL_GETDATA 0x7A
#define SE_TUNNEL_PUTDATA 0x7B
#define SE_START_RANGING  0x7C

/* Factory mode management */
#define CONFIGURE_AUTH_TAG_OPTIONS        0xB1
#define CONFIGURE_AUTH_TAG_VERSION        0xB2
#define CALIBRATION_INTEGRITY_PROCTECTION 0xB3
#define GENERATE_TAG_CMD                  0xB4

void HifInit(UWB_Hif_Comm_Mode_t comm_mode);
void HifDeInit();
OSAL_TASK_RETURN_TYPE UWB_HIFTask(void *args);
void UWB_HIFPostQueue(const void *data);
uint32_t UWB_Hif_SendRsp(uint8_t *pData, uint16_t size);
void HifSetMode(UWB_MODES_t state);
UWB_MODES_t HifGetMode();
extern intptr_t mHifCommandQueue;
extern void *mCmdMutex;
#endif //_UWB_HIF_H_
