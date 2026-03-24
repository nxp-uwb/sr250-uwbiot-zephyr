/* Copyright 2020,2022,2023 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <phOsalUwb.h>

/* UWB TLV */

typedef uint8_t utlv_max_size_t;

typedef struct utlv_value_au8
{
    /* If inMaxLen = 0, point to destination */
    uint8_t *ptr;
    utlv_max_size_t inMaxLen;
    utlv_max_size_t outLen;
} utlv_value_au8_t;

typedef union utlv_value {
    uint8_t vu8;
    uint16_t vu16;
    uint32_t vu32;
    utlv_value_au8_t au8;
} utlv_value_t;

typedef enum utlv_type_t
{
    /** We don't know the type */
    kUTLV_Type_Unknown = 0,
    /** It's an 8 bit value */
    kUTLV_Type_u8 = 1,
    /** It's a 16 bit value */
    kUTLV_Type_u16 = 2,
    /** It's a 32 bit value */
    kUTLV_Type_u32 = 4,
    /** It's an array of 8 bit values */
    kUTLV_Type_au8 = 5,

} utlv_type_t;

typedef struct utlv_entry
{
    /** Input: search this tag */
    uint8_t tag;
    /** Input/Output: Expected type */
    utlv_type_t tag_type;
    /** Output: Found Value */
    utlv_value_t tag_value;
} utlv_entry_t;

typedef enum utlv_status
{
    kUTLV_Status_Unknown,
    /* Value found */
    kUTLV_Status_Found = 0x1,
    /* Tag is not found found */
    kUTLV_Status_NotFound = 0x2,
    kUTLV_Status_Failed   = 0x70,
    /* Tag is found... but type is not as we expect */
    kUTLV_Status_FailedWrongType = 0x71,
    /* Tag is found... but destination buffer is small */
    kUTLV_Status_FailedBufferOverflow = 0x72,
} utlv_status_t;

/**
 * @brief         Parse the TLV Buffer and find and update pEntry
 *
 * This utility will traverse the pBuf, till bufLen. It will try
 * to search pEntry->tag from the pBuf.
 *
 * If pEntry->tag_type was Unknown, Once, it finds the expected
 * tag, it will update pEntry->tag_type And update
 * pEntry->tag_value.  pEntry->tag_value is a union, so it will
 * update required entry.
 *
 * If pEntry->tag_type was known, and there's a mismatch in the
 * found entry, this API would fail.
 *
 * If pEntry->tag_type was AU8 (Array of uint8_t), there are two
 * possibilites...
 *
 * AU8 No memcopy:
 *
 * - As in input, pEntry->au8.ptr is NULL
 * - Just point pEntry->au8.ptr to the existing location inside
 *   pBuf (No need to memcpy), and update pEntry->au8.outLen to
 *   lenght that we found.
 *
 * AU8 Copy Buffer:
 *
 * - As in input, pEntry->au8.ptr points to a valid buffer
 * - As in input, pEntry->au8.inMaxLen is maximum buffer as
 *   pointed by ptr
 * - Once found, the contents is copied to pEntry->au8.ptr, and
 *   pEntry->au8.outLen is update to the length that we found.
 *
 * \param[in]     pBuf    The buffer that has TLV or sequence of TLVs
 * \param[in]     bufLen  The buffer length
 * \param[in,out] pEntry  The entry.
 *
 * @return        The utlv status. See description of utlv_status_t
 */

utlv_status_t utlv_parse_entry(const uint8_t *pBuf, size_t bufLen, utlv_entry_t *pEntry);

#if 0
utlv_status_t utlv_parse_entries(const uint8_t *pBuf, size_t bufLen, utlv_entry_t *pEntries, size_t entrySize);
#endif
