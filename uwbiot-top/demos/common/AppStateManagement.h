/* Copyright 2019,2020 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef APP_APPSTATEMANAGEMENT_H_
#define APP_APPSTATEMANAGEMENT_H_

#include "phUwbTypes.h"
#include "UwbApi.h"

#define MAX_SESSIONS 5
typedef struct AppStateInfo
{
    uint32_t session_id;
    uint8_t currentState;
    uint8_t opType;
} AppStateInfo_t;

typedef struct AppContext
{
    AppStateInfo_t appStates[MAX_SESSIONS];
} AppContext_t;

/**
 * brief Initializes Session Info states of Application

 * param pAppContext - [IN] pointer to AppContext_t.
 *
 * \return None
 */
void initAppStateInfo(AppContext_t *pAppContext);
/**
 * brief Adds App Session state info in to App context

 * param pAppContext - [IN] pointer to AppContext_t which holds Array of AppStateInfo_t
 * param pCurrentState -[IN] pointer to AppStateInfo_t.
 *
 * \return TRUE if successfully added
 * \return FALSE otherwise
 */
BOOLEAN addStateInfo(AppContext_t *pAppContext, AppStateInfo_t *pCurrentState);
/**
 * brief Removes App Session state info from App context

 * param pAppContext - [IN] pointer to AppContext_t which holds Array of AppStateInfo_t
 * param sessionId -[IN] sessionId to be removed.
 *
 * \return TRUE if successfully Removed
 * \return FALSE otherwise
 */
BOOLEAN removeStateInfo(AppContext_t *pAppContext, uint32_t sessionId);
/**
 * brief Updates App Session state wrt Session ID

 * param pAppContext - [IN] pointer to AppContext_t which holds Array of AppStateInfo_t
 * param sessionId -[IN] Session ID whose State to be updated
 * param state -[IN] Current State
 *
 * \return TRUE if successfully updated
 * \return FALSE otherwise
 */
BOOLEAN updateState(AppContext_t *pAppContext, uint32_t sessionId, uint8_t state);

/**
 * brief Gets current state for a session ID
 * param pAppContext -[IN] pointer to AppContext_t which holds Array of AppStateInfo_t

 * \return State from Session ID
 */
uint8_t getCurrentState(AppContext_t *pAppContext, uint32_t sessionId);
/**
 * brief Gets active ranging session count and session IDs
 * param pAppContext -[IN] pointer to AppContext_t which holds Array of AppStateInfo_t
 * param sessionIds -[OUT] array of sessionIds which contains session IDs of ongoing Active sessions.
 *                          Array should be able to hold elements upto MAX_SESSIONS
 * param pNoOfActiveSessions -[OUT] Count of active ranging sessions, Zero if no ranging sessions are active
 * \return None
 */
void getActiveRangingSessionCount(AppContext_t *pAppContext, uint32_t sessionIds[], uint8_t *pNoOfActiveSessions);

/**
 * brief Gets active session count and session IDs
 * param pAppContext -[IN] pointer to AppContext_t which holds Array of AppStateInfo_t
 * param sessionIds -[OUT] array of sessionIds which contains session IDs of ongoing Active sessions.
 *                          Array should be able to hold elements upto MAX_SESSIONS
 * param pNoOfActiveSessions -[OUT] Count of active ranging sessions, Zero if no ranging sessions are active
 * \return None
 */
void getActiveSessionIds(AppContext_t *pAppContext, uint32_t sessionIds[], uint8_t *pNoOfActiveSessions);

/**
 * brief Clears the App context with default values.
 * \return None
 */
void cleanUpAppContext();
#endif /*APP_APPSTATEMANAGEMENT_H_*/
