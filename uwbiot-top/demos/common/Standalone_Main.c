/* Copyright 2024-2025 NXP
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

#if !defined(UWBIOT_APP_BUILD__DEMO_STANDALONE_COMMON)
#include "UWBIOT_APP_BUILD.h"
#endif

#if !defined(UWBIOT_APP_BUILD__DEMO_PNP)
#include <stdio.h>
#include "uwb_board.h"

/* App realted includes*/
#include "AppInternal.h"

#include "phOsalUwb.h"

#if UWBFTR_SE_SE051W
#define HAVE_KSDK
#include "ex_sss_main_inc_ksdk.h"
#endif

UWBOSAL_TASK_HANDLE testTaskHandle;
void UWBDemo_Init();
UWBOSAL_TASK_HANDLE uwb_demo_start(void);

#if UWBIOT_OS_NATIVE || defined(_MSC_VER)
extern uint8_t isRecovery;
int gapp_argc;
const char **gapp_argv;
#else
int gapp_argc              = 1;
const char gapp_argv[2][4] = {"NA", ""};
#endif

#if UWBIOT_SESN_SNXXX
int8_t GeneralState = 0;
#endif // UWBIOT_SESN_SNXXX

#if defined(QN9090_SERIES)
/* Allocate the memory for the heap. */
uint8_t __attribute__((section(".bss.$SRAM1"))) ucHeap[configTOTAL_HEAP_SIZE];
#endif // defined(QN9090_SERIES)

#if defined(CPU_S32K144)
extern void trmrpc_client_init(void);
/* Allocate the memory for the heap. */
uint8_t __attribute__((section(".customSection"))) ucHeap[configTOTAL_HEAP_SIZE];
#endif // defined(CPU_S32K144)

/*
 * @brief   Application entry point.
 */
int main(int argc, const char *argv[])
{
    /* Init board hardware. */
    board_init();
    UWBDemo_Init();
    /* starting ntf handling task*/
    demo_ntf_handling_task_start();

#if UWBIOT_OS_NATIVE || defined(_MSC_VER)
    gapp_argc = argc;
    gapp_argv = argv;
#if UWBIOT_OS_NATIVE
restart:
#endif
#endif // UWBIOT_OS_NATIVE
    testTaskHandle = uwb_demo_start();
#if UWBIOT_OS_NATIVE
    void *retval;
    pthread_join(testTaskHandle, &retval);
    if (isRecovery) {
        isRecovery = false;
        /* restart the demo task */
        goto restart;
    }
    demo_ntf_handling_task_stop();
#elif UWBIOT_OS_ZEPHYR
    phOsalUwb_Thread_Join(testTaskHandle);
#else  // Freertos
    phOsalUwb_TaskStartScheduler();
#endif // UWBIOT_OS_NATIVE
    return 0;
}

#endif /* !defined(UWBIOT_APP_BUILD__DEMO_PNP) */
