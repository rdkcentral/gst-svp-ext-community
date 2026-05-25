/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
#include "gst_svp_header.h"
#include "gst_svp_meta.h"
#include "gst_svp_logging.h"

#include <gst/base/gstbytereader.h>

#include <cinttypes>

const gsize GUID_SIZE = 16;

typedef struct SvpHeader_t {
    // eb9a1c44-38f3-41b4-8a31-2ed7fb814e15 - generated from https://www.uuidtools.com/generate/bulk
    uint8_t GUID[GUID_SIZE] = {0x44, 0x1c, 0x9a, 0xeb, 0xf3, 0x38, 0xb4, 0x41, 0x8a, 0x31, 0x2e, 0xd7, 0xfb, 0x81, 0x4e, 0x15};
    uint32_t type = 0;
    uint32_t version = 0;
    uint32_t dataSize = 0;
    uint32_t handle = 0;
} SvpHeader;

gsize gst_svp_header_size(void * pContext)
{
    return sizeof(SvpHeader);
}

gboolean gst_svp_header_set_field(void * pContext, void * const buffer, const SvpHeaderFieldName fieldName, const uint32_t value)
{
    SvpHeader * const header = reinterpret_cast<SvpHeader * const>(buffer);

    if (!header)
    {
        LOG(eError, "Cannot set field on null header");
        return false;
    }

    switch (fieldName)
    {
        case SvpHeaderFieldName::Type:
            header->type = value;
            return true;
        case SvpHeaderFieldName::Version:
            header->version = value;
            return true;
        case SvpHeaderFieldName::DataSize:
            header->dataSize = value;
            return true;
        case SvpHeaderFieldName::OutputBuffer:
            header->handle = value;
            return true;
        default:
            LOG(eError, "Unknown SVP Header flag: %d\n", fieldName);
    }
    return false;
}

gboolean gst_svp_header_get_field(void * pContext, const void * const buffer, const SvpHeaderFieldName fieldName, uint32_t * const value)
{
    const SvpHeader * const header = reinterpret_cast<const SvpHeader * const>(buffer);

    if (!header)
    {
        LOG(eError, "Cannot get field from null header");
        return false;
    }

    switch (fieldName)
    {
        case SvpHeaderFieldName::Type:
            *value = header->type;
            return true;
        case SvpHeaderFieldName::Version:
            *value = header->version;
        case SvpHeaderFieldName::DataSize:
            *value = header->dataSize;
            return true;
        case SvpHeaderFieldName::OutputBuffer:
            *value = header->handle;
            return true;
        default:
            LOG(eError, "Unknown SVP Header flag: %d\n", fieldName);
    }
    return false;
}

gboolean gst_svp_has_header(void * pContext, const void * const buffer)
{
    static const SvpHeader defaultHeader;

    return memcmp(buffer, defaultHeader.GUID, GUID_SIZE) == 0;
}

void gst_svp_write_header(void * pContext, void * const buffer)
{
    static const SvpHeader defaultHeader;

    memcpy(buffer, reinterpret_cast<const void * const>(&defaultHeader), gst_svp_header_size(pContext));
}

void * gst_svp_header_get_start_of_data(void * pContext, void * const header)
{
    if (!header)
    {
        LOG(eError, "Could not get start of data as header ptr is null.\n");
        return NULL;
    }

    return header + gst_svp_header_size(pContext);
}

void * gst_svp_header_remove_header_if_present(void * pContext, void* encryptedData)
{
    if (gst_svp_has_header(pContext, encryptedData))
    {
        return gst_svp_header_get_start_of_data(pContext, encryptedData);
    }

    return encryptedData;
}

gsize gst_svp_allocate_data_block(void * pContext, void ** ppData, const gsize encryptedDataSize, const gsize physicalDataSize, const gboolean needSecMemAlloc)
{
    const gsize totalBlockSize = physicalDataSize + svp_token_size() + gst_svp_header_size(pContext);

    if (ppData)
    {
        *ppData = g_malloc(totalBlockSize);

        gst_svp_write_header(pContext, *ppData);
        gst_svp_header_set_field(pContext, *ppData, SvpHeaderFieldName::DataSize, encryptedDataSize);

        if (needSecMemAlloc)
        {
            uint32_t handle = 0;
            if (!gst_svp_allocate_sec_mem(pContext, &handle, encryptedDataSize))
            {
                LOG(eError, "Failed to allocate secure memory\n");
            }
            else
            {
                gst_svp_header_set_field(pContext, *ppData, SvpHeaderFieldName::OutputBuffer, handle);
                gst_svp_header_set_field(pContext, *ppData, SvpHeaderFieldName::Type, TokenType::PreAllocatedHandle);
            }
        }
    }
    else
    {
        LOG(eError, "Failed to allocate SVP data block as ppData is NULL\n");
        return 0;
    }

    return totalBlockSize;
}

void gst_svp_free_data_block(void * pContext, void * pData)
{
    if (!pData)
    {
        return;
    }

    if (gst_svp_has_header(pContext, pData))
    {
        uint32_t handle = 0;
        if (gst_svp_header_get_field(pContext, pData, SvpHeaderFieldName::OutputBuffer, &handle) && handle != 0)
        {
            LOG(eTrace, "Releasing secure memory %" PRIu32 "\n", handle);
            gst_svp_release_sec_mem(pContext, handle);
        }
    }
    else    // This shouldn't ever happen
    {
        LOG(eError, "Freeing data block that doesn't have SVP header");
    }

    g_free(pData);
}