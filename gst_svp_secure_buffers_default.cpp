/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
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

#include <assert.h>
#include <iostream>

#include <pthread.h>
#include <sstream>
#include <string>
#include <string.h>
#include <stdint.h>

#include "gst_svp_meta.h"
#include "gst_svp_meta_private.h"
#include "gst_svp_logging.h"

uint32_t svp_allocate_secure_buffers_v2_impl_default(void* pContext, void** ppInBuf, void** ppOutBuf, const uint8_t* pInData, const size_t nDataLen)
{
  LOG(eError, "Not implemented\n");
  return 0;
}

uint32_t svp_release_secure_buffers_v2_impl_default(void* pContext, void* pInBuf, void* pOutBuf, uint8_t* pDataOut, size_t nDataOutMax)
{
  LOG(eError, "Not implemented\n");
  return 0;
}

gboolean svp_allocate_secure_buffer_default(void* pContext, void ** ppBuf, const uint8_t* pInData, const size_t nDataLen)
{
  LOG(eError, "Not implemented\n");
  return FALSE;
}

gboolean svp_free_secure_buffer_default(void* pContext, void * pBuf)
{
  LOG(eError, "Not implemented\n");
  return FALSE;
}

gboolean svp_release_secure_buffer_default(void* pContext, void * pBuf)
{
  LOG(eError, "Not implemented\n");
  return FALSE;
}