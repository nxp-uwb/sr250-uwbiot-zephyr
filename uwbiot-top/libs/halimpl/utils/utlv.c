/* Copyright 2020,2023 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

/* UWB TLV */

#include <utlv.h>
#include <uwb_types.h>
#include <string.h>
#include <phNxpLogApis_App.h>
#include <nxEnsure.h>

/** Local functions prototypes */
static utlv_status_t utlv_checkLengthAndType(utlv_entry_t *pEntry, uint8_t tag_lenght);

static utlv_status_t utlv_checkLengthAndType(utlv_entry_t *pEntry, uint8_t tag_lenght)
{
    utlv_status_t retStatus = kUTLV_Status_FailedWrongType;
    switch (pEntry->tag_type) {
    case kUTLV_Type_Unknown: {
        retStatus = kUTLV_Status_Found;
        switch (tag_lenght) {
        case 1:
            pEntry->tag_type = kUTLV_Type_u8;
            break;
        case 2:
            pEntry->tag_type = kUTLV_Type_u16;
            break;
        case 4:
            pEntry->tag_type = kUTLV_Type_u32;
            break;
        default:
            pEntry->tag_type = kUTLV_Type_au8;
            break;
        }
    } break;
    case kUTLV_Type_u8:
        if (1 == tag_lenght) {
            retStatus = kUTLV_Status_Found;
        }
        break;
    case kUTLV_Type_u16:
        if (2 == tag_lenght) {
            retStatus = kUTLV_Status_Found;
        }
        break;
    case kUTLV_Type_u32:
        if (4 == tag_lenght) {
            retStatus = kUTLV_Status_Found;
        }
        break;
    case kUTLV_Type_au8:
        retStatus = kUTLV_Status_Found;
        break;
    default:
        break;
    }
    return retStatus;
}

utlv_status_t utlv_parse_entry(const uint8_t *pBuf, size_t ubufLen, utlv_entry_t *pEntry)
{
    utlv_status_t retStatus = kUTLV_Status_Failed;
    size_t bufLen           = ubufLen;
    uint32_t index;
    uint8_t tagPresent;
    uint8_t tag_length;
    ENSURE_OR_GO_CLEANUP(pBuf != NULL);
    ENSURE_OR_GO_CLEANUP(pEntry != NULL);
    ENSURE_OR_GO_CLEANUP(ubufLen >= 2);
    ENSURE_OR_GO_CLEANUP(ubufLen <= 255);

    while (retStatus != kUTLV_Status_Found && bufLen >= 2) {
        index = 0U;
        UWB_STREAM_TO_UINT8(tagPresent, pBuf, index);
        bufLen--;
        // we will get length from here
        UWB_STREAM_TO_UINT8(tag_length, pBuf, index);
        bufLen--;
        if (pEntry->tag == tagPresent) {
            /* we know type, need to match wth lenght information */
            retStatus = utlv_checkLengthAndType(pEntry, tag_length);
            if (retStatus != kUTLV_Status_Found) {
                goto cleanup;
            }

            if ((index + tag_length) > ubufLen) {
                retStatus = kUTLV_Status_Failed;
                goto cleanup;
            }

            switch (pEntry->tag_type) {
            case kUTLV_Type_Unknown:
            default:
                tag_length = 0;
                break;
            case kUTLV_Type_u8:
                UWB_STREAM_TO_UINT8(pEntry->tag_value.vu8, pBuf, index);
                break;
            case kUTLV_Type_u16:
                UWB_STREAM_TO_UINT16((pEntry->tag_value.vu16), pBuf, index);
                break;
            case kUTLV_Type_u32:
                UWB_STREAM_TO_UINT32((pEntry->tag_value.vu32), pBuf, index); // littile to big
                break;
            case kUTLV_Type_au8:
                if (pEntry->tag_value.au8.ptr == NULL) {
                    pEntry->tag_value.au8.ptr = (uint8_t *)&pBuf[index];
                    pEntry->tag_value.au8.outLen = tag_length;
                }
                else if (pEntry->tag_value.au8.inMaxLen >= tag_length && pEntry->tag_value.au8.ptr != NULL) {
                    phOsalUwb_MemCopy(pEntry->tag_value.au8.ptr, &pBuf[index], tag_length);
                    pEntry->tag_value.au8.outLen = tag_length;
                }
                else {
                    retStatus = kUTLV_Status_FailedBufferOverflow;
                }
                index += tag_length;
                break;
            }
        }
        else {
            /* Skip to next tag. Index is already incremented */
            index += tag_length;
        }

        if (bufLen >= tag_length) {
            bufLen -= tag_length;
        }
        else {
            bufLen = 0U;
            retStatus = kUTLV_Status_NotFound;
        }
        pBuf = &pBuf[index];
    }

cleanup:
    return retStatus;
}

// utlv_status_t utlv_parse_entries(const uint8_t *pBuf, size_t bufLen, utlv_entry_t *pEntries, size_t entrySize)
//{
//    return kUTLV_Status_Found;
//}
