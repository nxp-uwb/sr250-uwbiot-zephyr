/*
 *
 * Copyright 2021-2024, 2026 NXP.
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading,installing, activating and/or otherwise
 * using the software, you are agreeing that you have read,and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install, activate
 * or otherwise use the software.
 *
 */
#ifndef UWB_CORE_UWBAPI_API_UWBAPI_UTILITY_H_
#define UWB_CORE_UWBAPI_API_UWBAPI_UTILITY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "UwbApi_Types.h"
#include "UwbApi_Types_Proprietary.h"
#include "uwb_types.h"
#include "UwbApi_Internal.h"
#include "uwb_int.h"
/**
**
** Function         sendUciCommandAndWait
**
** Description      Send Uci command and wait on semaphore
**
** Returns          UWBAPI_STATUS_OK on success
**                  UWBAPI_STATUS_FAILED otherwise
**
*/
EXTERNC tUWBAPI_STATUS sendUciCommandAndWait(uint16_t event, uint16_t cmdLen, uint8_t *pCmd);

/**
**
** Function         sendUciCommand
**
** Description      Send Uci command
**
** Returns          UWBAPI_STATUS_OK on success
**                  UWBAPI_STATUS_FAILED otherwise
**
*/
EXTERNC tUWBAPI_STATUS sendUciCommand(uint16_t event, uint16_t cmdLen, uint8_t *pCmd, uint8_t pbf);

/**
**
** Function         serializeSessionInitPayload
**
** Description      serialize Session Init Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeSessionInitPayload(uint32_t sessionHandle, eSessionType sessionType, uint8_t *pCmdBuf);

/**
**
** Function         serializeGetCoreConfigPayload
**
** Description      serialize Get Core Config Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeGetCoreConfigPayload(
    uint8_t noOfParams, uint8_t paramLen, uint8_t *paramId, uint8_t *pCmdBuf);

/**
**
** Function         serializeSessionHandlePayload
**
** Description      serialize Session Handle Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeSessionHandlePayload(uint32_t sessionHandle, uint8_t *pCmdBuf);

/**
**
** Function         serializeAppConfigPayload
**
** Description      serialize App Config Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeAppConfigPayload(
    uint32_t sessionHandle, uint8_t noOfParams, uint16_t paramLen, uint8_t *pCmdBuf);

/**
**
** Function         serializeUpdateControllerMulticastListPayload
**
** Description      serialize Update Controller MulticastList Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeUpdateControllerMulticastListPayload(
    phMulticastControleeListContext_t *pControleeContext, uint8_t *pCmdBuf);

#if !(UWBIOT_UWBD_SR04X)
/**
**
** Function         serializeTestDataPayload
**
** Description      serialize Test Data Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeTestDataPayload(uint16_t psduLen, uint8_t psduData[], uint8_t *pCmdBuf);
#endif //!(UWBIOT_UWBD_SR04X)

#if !(UWBIOT_UWBD_SR04X)
/**
**
** Function         serializeDoChipCalibPayload
**
** Description      serialize do Calib Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeDoChipCalibPayload(uint8_t channel, uint8_t *pCmdBuf);
#endif // !(UWBIOT_UWBD_SR04X)

#if UWBIOT_UWBD_SR1XXT
/**
**
** Function         serializeWriteOtpCalibDataPayload
**
** Description      serialize Write Otp Calib Data Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeWriteOtpCalibDataPayload(
    uint8_t channel, uint8_t writeOption, uint8_t writeDataLen, uint8_t *writeData, uint8_t *pCmdBuf);

/**
**
** Function         serializeReadOtpCalibDataPayload
**
** Description      serialize Read Otp Calib Data Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeReadOtpCalibDataPayload(
    uint8_t channel, uint8_t readOption, eOtpCalibParam calibParam, uint8_t *pCmdBuf);
#endif // UWBIOT_UWBD_SR1XXT

#if !(UWBIOT_UWBD_SR04X)
/**
**
** Function         serializeSetCalibPayload
**
** Description      serialize Set Calib Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeSetCalibPayload(
    uint8_t channel, eCalibParam paramId, uint8_t *calibrationValue, uint16_t length, uint8_t *pCmdBuf);

/**
**
** Function         serializeGetCalibPayload
**
** Description      serialize Get Calib Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeGetCalibPayload(phGetCalibInputParams_t *calibInput, uint8_t *pCmdBuf);
#endif // !(UWBIOT_UWBD_SR04X)

#if (UWBIOT_SESN_SNXXX)
/**
**
** Function         serializeSeLoopTestPayload
**
** Description      serialize Se Loop Test Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeSeLoopTestPayload(uint16_t loopCnt, uint16_t timeInterval, uint8_t *pCmdBuf);
#endif //(UWBIOT_SESN_SNXXX)

#if (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S)
/**
**
** Function         serializecalibIntegrityProtectionPayload
**
** Description      serialize calib Integrity Protection Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializecalibIntegrityProtectionPayload(
    eCalibTagOption tagOption, uint16_t calibBitMask, uint8_t *pCmdBuf);

/**
**
** Function         serializeVerifyCalibDataPayload
**
** Description      serialize Verify Calib Data Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeVerifyCalibDataPayload(
    uint8_t *pCmacTag, uint8_t tagOption, uint16_t tagVersion, uint8_t *pCmdBuf);

/**
**
** Function         serializeConfigureAuthTagOptionsPayload
**
** Description      serialize Configure Auth Tag Options Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeConfigureAuthTagOptionsPayload(
    uint8_t deviceTag, uint8_t modelTag, uint16_t labelValue, uint8_t *pCmdBuf);

/**
**
** Function         serializeConfigureAuthTagVersionPayload
**
** Description      serialize Configure Auth Tag Version Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeConfigureAuthTagVersionPayload(uint16_t labelValue, uint8_t *pCmdBuf);
#endif //UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S
#if (UWBIOT_SESN_SNXXX)
/**
**
** Function         serializeUrskDeletionRequestPayload
**
** Description      serialize Ursk Deletion Request Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeUrskDeletionRequestPayload(
    uint8_t noOfSessionIds, uint32_t *pSessionIdList, uint8_t *pCmdBuf);
#endif //(UWBIOT_SESN_SNXXX)

#if (UWBIOT_UWBD_SR04X)
/**
**
** Function         serializeSessionNvmPayload
**
** Description      serialize NVM payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeSessionNvmPayload(
    esessionNvmManage sesNvmManageTag, uint32_t sessionHandle, uint8_t *pCmdBuf);

#endif /* (UWBIOT_UWBD_SR04X) */

/**
**
** Function         serializeTrngtPayload
**
** Description      serialize Trng payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeTrngtPayload(uint8_t trng_size, uint8_t *pCmdBuf);

#if (UWBFTR_BlobParser)
/**
**
** Function         serializeSetProfileParamsPayload
**
** Description      serialize set profile params payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeSetProfileParamsPayload(
    phUwbProfileInfo_t *pProfileInfo, uint16_t blobSize, uint8_t *pProfileBlob, uint8_t *pCmdBuf);

/**
**
** Function         serializeUwbDeviceConfigData
**
** Description      serializes dset profile params payload
**
** Returns          Length of the serialized data
**
*/
uint16_t serializeUwbDeviceConfigData(UwbDeviceConfigData_t *pUwbDeviceConfig, uint8_t *pCmdBuf);
/**
**
** Function         serializeUwbPhoneConfigData
**
** Description      deserializes phone configuration data
**
**
*/
void serializeUwbPhoneConfigData(UwbPhoneConfigData_t *pUwbPhoneConfig, uint8_t *pCmdBuf);

#endif // UWBFTR_BlobParser

#if !(UWBIOT_UWBD_SR04X)
/**
**
** Function         serializeControllerHusSessionPayload
**
** Description      serialize HUS Session Payload of Controller
**
** Returns          Length of payload
**
*/
uint16_t serializeControllerHusSessionPayload(phControllerHusSessionConfig_t *pHusSessionCfg, uint8_t *pCmdBuf);

/**
**
** Function         serializeControleeHusSessionPayload
**
** Description      serialize HUS Session Payload of Controlee
**
** Returns          Length of payload
**
*/
uint16_t serializeControleeHusSessionPayload(phControleeHusSessionConfig_t *pHusSessionCfg, uint8_t *pCmdBuf);

/**
**
** Function         serializeDtpcmPayload
**
** Description      serialize DTPCM Payload
**
** Returns          Length of payload
**
*/
uint16_t serializeDtpcmPayload(phDataTxPhaseConfig_t *phDataTxPhaseCfg, uint8_t *pCmdBuf);

#endif // !(UWBIOT_UWBD_SR04X)

#if UWBFTR_DL_TDoA_Anchor && !(UWBIOT_UWBD_SR04X)
/**
**
** Function         serializeUpdateActiveRoundsAnchorPayload
**
** Description      serialize update active rounds Anchor Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeUpdateActiveRoundsAnchorPayload(uint32_t sessionHandle,
    uint8_t nActiveRounds,
    UWB_MacAddressMode_t macAddressingMode,
    const phActiveRoundsConfig_t roundConfigList[],
    uint8_t *pCmdBuf);
#endif // UWBFTR_DL_TDoA_Anchor && !(UWBIOT_UWBD_SR04X)

#if UWBFTR_DL_TDoA_Tag
/**
**
** Function         serializeUpdateActiveRoundsReceiverPayload
**
** Description      serialize update active rounds receiver Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeUpdateActiveRoundsReceiverPayload(
    uint32_t sessionHandle, uint8_t nActiveRounds, const uint8_t roundConfigList[], uint8_t *pCmdBuf);
#endif // UWBFTR_DL_TDoA_Tag

#if UWBFTR_DataTransfer
/**
**
** Function         serializeSendDataPayload
**
** Description      serialize Send Data Payload
**
** Returns          Length of payload
**
*/
EXTERNC uint16_t serializeSendDataPayload(phUwbDataPkt_t *pSendData, uint8_t *pCmdBuf);
#endif // UWBFTR_DataTransfer


/** \addtogroup uwb_utils
 *
 * @{ */

/**
 * @brief          Extracts Ranging Params from the given byte array
 *                 and updates the structure phRangingData_t
 *
 * \param p        Pointer to byte array containing range notification
 * \param len      Length of input array \p p
 * \param pRngData Pointer to phRangingData_t structure to be populated
 *
 */
void parseRangingNtf(uint8_t *p, uint16_t len, phRangingData_t *pRngData);

#if UWBFTR_Radar
/**
 * @brief          Extracts Radar Params from the given byte array
 *                 and updates structure phUwbRadarNtf_t and pRadarNtfBuff if passed
 *
 * \param p             Pointer to byte array containing radar notification
 * \param len           Length of input array \p p
 * \param pRadarNtf     Pointer to phUwbRadarNtf_t structure to be populated
 * \param pRadarNtfBuff Pointer to output buffer to be populated with cirdata
 *
 */
void parseRadarNtf(uint8_t *p, uint16_t len, phUwbRadarNtf_t *pRadarNtf, uint8_t *pRadarNtfBuff);
#endif // UWBFTR_Radar

#if UWBFTR_CCC && !(UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250)
/**
 * @brief                   Extracts CCC Ranging Params from the given byte array
 *                          and updates structure phCccRangingData_t
 *
 * \param p                 Pointer to byte array containing CSA ranging notification
 * \param len               Length of input array \p p
 * \param pCccRangingData Pointer to phCccRangingData_t structure to be populated
 *
 */
void parseCccRangingNtf(uint8_t *p, uint16_t len, phCccRangingData_t *pCccRangingData);
#endif // UWBFTR_CCC && !(UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250)

#if UWBFTR_CSA
/**
 * @brief                   Extracts CSA Ranging Params from the given byte array
 *                          and updates structure phCsaRangingData_t
 *
 * \param p                 Pointer to byte array containing CSA ranging notification
 * \param len               Length of input array \p p
 * \param pCsaRangingData   Pointer to phCsaRangingData_t structure to be populated
 *
 */
void parseCsaRangingNtf(uint8_t *p, uint16_t len, phCsaRangingData_t *pCsaRangingData);
#endif // UWBFTR_CSA

#if UWBFTR_DataTransfer
/**
 * @brief             Extracts Data notification from the given byte array
 *                    and updates structure phUwbRcvDataPkt_t
 *
 * \param p           Pointer to byte array containing data receive notification
 * \param len         Length of input array \p p
 * \param pRcvDataPkt Pointer to phUwbRcvDataPkt_t structure to be populated
 *
 */
void parseDataRcvNtf(uint8_t *p, uint16_t len, phUwbRcvDataPkt_t *pRcvDataPkt);

/**
 * @brief             Extracts Data notification from the given byte array
 *                    and updates structure phLogicalLinkDataPkt_t
 *
 * \param p           Pointer to byte array containing data receive notification
 * \param len         Length of input array \p p
 * \param pRcvDataPkt Pointer to phLogicalLinkDataPkt_t structure to be populated
 *
 */
void parseLogicalDataRcvNtf(uint8_t *p, uint16_t len, phLogicalLinkDataPkt_t *pRcvDataPkt);
#endif // UWBFTR_DataTransfer
#if !(UWBIOT_UWBD_SR04X)
/**
 * @brief             Extracts session Role Change notification from the given byte array
 *                    and updates structure phNewRoleChangeNtf_t
 *
 * \param p           Pointer to byte array containing data receive notification
 * \param len         Length of input array \p p
 * \param pNewRole Pointer to phNewRoleChangeNtf_t structure to be populated
 *
 */

void parseNewRoleChangeRcvNtf(uint8_t *p, uint16_t len, phNewRoleChangeNtf_t *pNewRole);
#endif // #if !(UWBIOT_UWBD_SR04X)
/** @} */

/** \addtogroup uwb_utils_sr040
 *
 * @{ */

#if (UWBIOT_UWBD_SR04X)
/**
 * @brief                     This function is called to parse msg log
 *
 * \param p                   Pointer to byte array containing log notification
 * \param len                 Length of input array \p p
 * \param pTestPhyLogNtfnData Pointer to phPhyLogNtfnData_t structure to be populated
 *
 */
void parseMsgLogNtf(uint8_t *p, uint16_t len, phPhyLogNtfnData_t *pTestPhyLogNtfnData);
/**
 * @brief                     This function is called to parse ext psdu msg log
 *
 * \param p                   Pointer to byte array containing log notification
 * \param len                 Length of input array \p p
 * \param pTestPhyLogNtfnData Pointer to phPhyLogNtfnData_t structure to be populated
 *
 */
void parseExtPsduLogNtf(uint8_t *p, uint16_t len, phPhyLogNtfnData_t *pTestPhyLogNtfnData);
/**
 * @brief                     This function is called to parse Loopback data NTF
 *
 * \param p                   Pointer to byte array containing loopback data notification
 * \param len                 Length of input array \p p
 * \param pTestLoopbackStatus Pointer to phTestLoopbackData_t structure to be populated
 *
 */
void parseTestLoopbackData(uint8_t *p, uint16_t len, phTestLoopbackData_t *pTestLoopbackStatus);

#endif /* (UWBIOT_UWBD_SR04X) */

/** @} */

#if !(UWBIOT_UWBD_SR04X)
void deserializeRfTestDataNtf(phRfTestData_t *pRfTestData, uint8_t *pRespBuf, uint16_t rspLen);
void deserializeDataFromRxPerNtf(phTestPer_Rx_Ntf_t *ptestrecvdata, uint8_t *pRespBuf);
void deserializeDataFromRxNtf(phTest_Rx_Ntf_t *pRfTestRecvData, uint8_t *pRespBuf, uint8_t *pPsduBuf);
void deserializeDataFromLoopbackNtf(phTest_Loopback_Ntf_t *pRfTestRecvData, uint8_t *pRespBuf, uint8_t *pPsdu);
void deserializeDataFromSrRxNtf(phTest_Test_Sr_Ntf_t *pRfTestSrRxData, uint8_t *pRespBuf, uint8_t *pPsdu,uint16_t data_len);
#endif // !(UWBIOT_UWBD_SR04X)

#if UWBFTR_DataTransfer
/**
**
** Function         serializeCreateLogicalLinkCmd
**
** Description      serialize Create Logical Link Command.
**
** Returns          Length of payload
**
*/
uint16_t serializeCreateLogicalLinkCmd(phLogicalLinkCreateCmd_t *phLogicalLinkCreateCmd, uint8_t *pCmdBuf);

/**
**
** Function         serializeLinkConnectIdPayload
**
** Description      serialize Logical Link Id.
**
** Returns          Length of payload
**
*/
uint16_t serializeLinkConnectIdPayload(uint32_t LogicalLinkConnectId, uint8_t *pCmdBuf);

/**
**
** Function         deserializeLinkGetParamsPayload
**
** Description      de-serialize the Logical Link Get Params Response.
**
**
*/
void deserializeLinkGetParamsPayload(uint8_t *pCmdBuf, phLogicalLinkGetParamsRsp_t *phLogicalLinkGetParamsRsp);

/**
**
** Function         serializeLogicalSendDataPayload
**
** Description      serialize Logical Link Send Data Payload
**
** Returns          Length of payload
**
*/
uint16_t serializeLogicalSendDataPayload(phLogicalLinkDataPkt_t *pSendData, uint8_t *pCmdBuf);

#endif // UWBFTR_DataTransfer

/**
**
** Function         deserializeUpdateControllerMulticastListResp
**
** Description      serialize the response data recieved for the Update Controller Multicast List command
**
*/
void deserializeUpdateControllerMulticastListResp(
    phMulticastControleeListRspContext_t *pControleeListRsp, uint8_t *rsp_data);

#if !(UWBIOT_UWBD_SR04X)

/**
**
** Function         deserializeGetCalibResp
**
** Description      deserialize the response data recieved for Get Calibration command
**
*/
void deserializeGetCalibResp(phCalibRespStatus_t *pCalibResp, uint8_t *respBuf);

#endif // !(UWBIOT_UWBD_SR04X)

#if UWBIOT_UWBD_SR2XXT

/**
**
** Function         serializeSetSecureCalibPayload
**
** Description      serialize Set Secure Calib Payload
**
** Returns          Length of payload
**
*/
uint16_t serializeSetSecureCalibPayload(phSecureCalibParams_t *SecureCalibParams, uint8_t *pCmdBuf);

/**
**
** Function         validateSecureCalibParamId
**
** Description      Validates set secure Calib input parameters
**
** Returns          TRUE if validation is successful.
**
*/
BOOLEAN validateSecureCalibParamId(uint8_t calibParamId);

#endif // UWBIOT_UWBD_SR2XXT

#if UWBFTR_CSA
/*******************************************************************************
**
** Function         serializeSessionSetLocZoneCmd
**
** Description      serialize Session Set Localization Zone command.
**
** Returns          Length of command
**
*******************************************************************************/
EXTERNC uint16_t serializeSessionSetLocZoneCmd(phSessionSetLocZone_t *pSetLocZone, uint8_t *pCmdBuf);
#endif // UWBFTR_CSA

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /* UWB_CORE_UWBAPI_API_UWBAPI_UTILITY_H_ */
