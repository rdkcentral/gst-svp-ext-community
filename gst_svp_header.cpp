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
#include "gst_svp_meta_private.h"
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
    uint8_t handle[1] = {0};    // Dynamic array at end of struct - typically of size svp_token_size()
} SvpHeader;

gsize gst_svp_header_size(void * pContext)
{
    return sizeof(SvpHeader) + svp_token_size();
}

gboolean gst_svp_header_set_field_v2(void * pContext, void * const buffer, const SvpHeaderFieldName fieldName, void * value)
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
            header->type = *((uint32_t*)value);
            return true;
        case SvpHeaderFieldName::Version:
            header->version = *((uint32_t*)value);
            return true;
        case SvpHeaderFieldName::DataSize:
            header->dataSize = *((uint32_t*)value);
            return true;
        case SvpHeaderFieldName::OutputBuffer:
            memcpy(&header->handle, value, svp_token_size());
            return true;
        default:
            LOG(eError, "Unknown SVP Header flag: %d\n", fieldName);
    }
    return false;
}

gboolean gst_svp_header_get_field_v2(void * pContext, const void * const buffer, const SvpHeaderFieldName fieldName, void * value)
{
    const SvpHeader * const header = reinterpret_cast<const SvpHeader * const>(buffer);

    if (!header)
    {
        LOG(eError, "Cannot get field from null header\n");
        return false;
    }

    switch (fieldName)
    {
        case SvpHeaderFieldName::Type:
            *((uint32_t*)value) = header->type;
            return true;
        case SvpHeaderFieldName::Version:
            *((uint32_t*)value) = header->version;
            return true;
        case SvpHeaderFieldName::DataSize:
            *((uint32_t*)value) = header->dataSize;
            return true;
        case SvpHeaderFieldName::OutputBuffer:
            *((void**)value) = svp_header_extract_output_buffer(header->handle);
            return true;
        default:
            LOG(eError, "Unknown SVP Header flag: %d\n", fieldName);
    }
    return false;
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
            *((uint32_t*)&header->handle) = value;
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
        LOG(eError, "Cannot get field from null header\n");
        return false;
    }

    switch (fieldName)
    {
        case SvpHeaderFieldName::Type:
            *value = header->type;
            return true;
        case SvpHeaderFieldName::Version:
            *value = header->version;
            return true;
        case SvpHeaderFieldName::DataSize:
            *value = header->dataSize;
            return true;
        case SvpHeaderFieldName::OutputBuffer:
#ifndef MEDIATEK
            *((uint32_t*)value) = ((uint32_t*) header->handle)[0];
#else
            memcpy((void*) value, header->handle, 400);
#endif
            return true;
        default:
            LOG(eError, "Unknown SVP Header flag: %d\n", fieldName);
    }
    return false;
}

gboolean gst_svp_has_header(void * pContext, const void * const buffer)
{
    static const SvpHeader defaultHeader{};

    return memcmp(buffer, defaultHeader.GUID, GUID_SIZE) == 0;
}

void gst_svp_write_header(void * pContext, void * const buffer)
{
    static const SvpHeader defaultHeader{};

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
        *ppData = g_malloc0(totalBlockSize);

        gst_svp_write_header(pContext, *ppData);
        gst_svp_header_set_field(pContext, *ppData, SvpHeaderFieldName::DataSize, encryptedDataSize);

        if (needSecMemAlloc)
        {
            gsize allocatedDataSize = encryptedDataSize;
            void * handle = (void*) &((SvpHeader*) *ppData)->handle;
            if (!gst_svp_allocate_sec_mem(pContext, handle, &allocatedDataSize))
            {
                LOG(eError, "Failed to allocate secure memory\n");
            }
            else
            {
                if (allocatedDataSize == 0)
                {
                    LOG(eTrace, "Detected dummy handle creation\n");
                    gst_svp_header_set_field(pContext, *ppData, SvpHeaderFieldName::Type, TokenType::Fake);
                }
                else
                {
                    gst_svp_header_set_field(pContext, *ppData, SvpHeaderFieldName::Type, TokenType::PreAllocatedHandle);
                }
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
        uint32_t type = TokenType::InPlace;
        if (!gst_svp_header_get_field_v2(pContext, pData, SvpHeaderFieldName::Type, (void*)&type))
        {
            LOG(eError, "Error getting token type\n");
        }

        if (type == TokenType::PreAllocatedHandle || type == TokenType::Fake)
        {
            void * handle = nullptr;
            if (gst_svp_header_get_field_v2(pContext, pData, SvpHeaderFieldName::OutputBuffer, (void*)&handle))
            {
                gst_svp_release_sec_mem(pContext, handle);
            }
            else
            {
                LOG(eError, "Failed to get pre-allocated handle from the svp header\n");
            }
        }
    }
    else    // This shouldn't ever happen
    {
        LOG(eError, "Freeing data block that doesn't have SVP header");
    }

    g_free(pData);
}