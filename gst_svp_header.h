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

#ifndef __GST_SVP_HEADER_H__
#define __GST_SVP_HEADER_H__

#include <glib.h>
#include <gst/gst.h>

#include <cstdint>

G_BEGIN_DECLS

typedef enum TokenType_e {
    InPlace = 0,
    Handle  = 1,
    PreAllocatedHandle = 2
} TokenType;

typedef enum SecAPIVersion_e {
    TWO_THREE,
    THREE
} SecAPIVersion;

typedef enum SvpHeaderField_e {
    Type,
    Version,
    DataSize,
    OutputBuffer
} SvpHeaderFieldName;

gsize    gst_svp_header_size(void * pContext);

gboolean gst_svp_header_set_field(void * pContext, void * const buffer, const SvpHeaderFieldName fieldName, const uint32_t value);

gboolean gst_svp_header_get_field(void * pContext, const void * const buffer, const SvpHeaderFieldName fieldName, uint32_t * const value);

gboolean gst_svp_has_header(void * pContext, const void * const buffer);

void gst_svp_write_header(void * pContext, void * const buffer);

/**
 * Allocate data block for wrapping encrypted/decrypted data for SVP
 * @param ppData Pointer to store pointer to returned data block
 * @param encryptedDataSize Size of the encrypted data, this value will be written to the SVP header (note: This 
 *                          is not the size of the block that will be allocated).
 * @param physicalDataSize Physical Size of the data that will be written to the block. For example, this could be
 *                         the size of the handle to the secure buffer that stores the encrypted data, not the amount of
 *                         encrypted data.
 * @param needSecMemAlloc  If true, secure memory is allocated and the OutputBuffer field is set with the handle.
 * @returns Size of the allocated data
*/
gsize gst_svp_allocate_data_block(void * pContext, void ** ppData, const gsize encryptedDataSize, const gsize physicalDataSize, const gboolean needSecMemAlloc = false);

void gst_svp_free_data_block(void * pContext, void * pData);

void * gst_svp_header_get_start_of_data(void * pContext, void * const header);

void * gst_svp_header_remove_header_if_present(void * pContext, void* encryptedData);

G_END_DECLS
#endif /* __GST_SVP_HEADER_H__ */
