/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
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
#ifndef __GST_BUFFER_SVP_SECAPI_H__
#define __GST_BUFFER_SVP_SECAPI_H__

#include <stdint.h>

G_BEGIN_DECLS

uint32_t svp_allocate_secure_buffers_v2_secapi_impl(void* pContext, void** ppInBuf, void** ppOutBuf, const uint8_t* pInData, const size_t nDataLen);
uint32_t svp_release_secure_buffers_v2_secapi_impl(void* pContext, void* pInBuf, void* pOutBuf, uint8_t* pDataOut, size_t nDataOutMax);

gboolean svp_free_secure_buffer_v2_secapi_impl(void* pContext, void * pBuf);

#ifdef SECAPI_V3
uint32_t svp_allocate_secure_buffers_v3_impl_default(void* pContext, void** ppInBuf, void** ppOutBuf, const uint8_t* pInData, const size_t nDataLen);
uint32_t svp_release_secure_buffers_v3_impl_default(void* pContext, void* pInBuf, void* pOutBuf, uint8_t* pDataOut, size_t nDataOutMax);
#endif

G_END_DECLS
#endif //__GST_BUFFER_SVP_SECAPI_H__