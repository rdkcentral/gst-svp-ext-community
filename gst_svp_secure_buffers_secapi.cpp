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

#include "gst_svp_performance_secapi.h"

#include "gst_svp_secure_buffers_secapi.h"
#include "sec_security.h"       // SecAPI
#include "gst_svp_meta.h"
#include "gst_svp_meta_private.h"
#include "gst_svp_logging.h"

#ifdef SECAPI_V3
#include "sa_svp.h"
#include "sec_security_svp.h"
#endif

uint32_t svp_allocate_secure_buffers_v2_secapi_impl(void* pContext, void** ppInBuf, void** ppOutBuf, const uint8_t* pInData, const size_t nDataLen)
{
  SEC_SIZE                  nDataIn   = nDataLen;
  Sec_OpaqueBufferHandle*   pDataIn   = NULL;
  Sec_OpaqueBufferHandle*   pDataOut  = NULL;
  Sec_Result                sec_res   = SEC_RESULT_SUCCESS;

  LOG(eTrace, "AllocateSecureBuffers Creating SVP Buffers");

  if(ppOutBuf != NULL) {
    sec_res = SecOpaqueBuffer_Malloc( nDataIn, &pDataOut);
    if( SEC_RESULT_SUCCESS != sec_res )
    {
      LOG(eError, "SecOpaqueBuffer_Malloc failed for pDataOut!\n");
    }
  }

  if (SEC_RESULT_SUCCESS == sec_res && ppInBuf != NULL) {
    sec_res = SecOpaqueBuffer_Malloc(nDataIn, &pDataIn);
    if( SEC_RESULT_SUCCESS != sec_res )
    {
      if(pDataOut != NULL) SecOpaqueBuffer_Free(pDataOut);
      LOG(eError, "SecOpaqueBuffer_Malloc failed for pDataIn!\n");
    }
  }

  // Buffers allocated, now copy data into inBuf
  if( SEC_RESULT_SUCCESS == sec_res )
  {
    LOG(eTrace, "AllocateSecureBuffers SVP Buffers created");
    if(pDataIn) {
      // Copy input data into the protected memory buffer.
      // For SVP the SecCipher_Process function expects the data to be in
      // the same secure memory buffer as the output
      //LOGW("#CWVMediaKeySession :: AllocateSecureBuffers Copying %d bytes into pDataIn (%p)", nDataIn, pDataIn);
      if (SecOpaqueBuffer_Write(pDataIn, 0, (SEC_BYTE*)pInData, nDataIn)) {
        LOG(eError, "SecOpaqueBuffer_Write failed!!!\n\n");
        if (pDataIn) {
            SecOpaqueBuffer_Free(pDataIn);
        }
        if(pDataOut) {
          SecOpaqueBuffer_Free(pDataOut);
        }
        pDataIn = NULL;
        pDataOut = NULL;
      }
      else {
        LOG(eTrace, "AllocateSecureBuffers Copied %d bytes into pDataIn (%p)", nDataIn, pDataIn);
      }
    }
    else {
      // No pDataIn *** is any of this necessary???
      //LOG(eError, "AllocateSecureBuffers No pInData (%p)", pInData);
      //pDataIn = NULL;
      //pDataOut = NULL;
    }
  }
  else {
    LOG(eError, "OpaqueBuffer Allocation failed");
    if (pDataIn) {
        SecOpaqueBuffer_Free(pDataIn);
    }
    if(pDataOut) {
      SecOpaqueBuffer_Free(pDataOut);
    }
    pDataIn = NULL;
    pDataOut = NULL;
  }

  if(ppInBuf)   *ppInBuf  = pDataIn;
  if(ppOutBuf)  *ppOutBuf = pDataOut;

  return 1;
}

uint32_t svp_release_secure_buffers_v2_secapi_impl(void* pContext, void* pInBuf, void* pOutBuf, uint8_t* pDataOut, size_t nDataOutMax)
{
  uint32_t retVal = 0;

  void* secToken = NULL;
  svp_buffer_alloc_token(&secToken);
  svp_buffer_to_token(pContext, pOutBuf, secToken);

  memcpy((void *)pDataOut, secToken, svp_token_size());
  svp_buffer_free_token(secToken);

  // Sec_ProtectedMemHandle* svpHandle;
  // If we free the pOutBuf here will will crash due to double free (Realtek US).
  // Playback was testes for 2+ hours with out apparent memory leak.
  // SecOpaqueBuffer_Release((Sec_OpaqueBufferHandle*)pOutBuf, &svpHandle);

  if( pInBuf != NULL) {
    SecOpaqueBuffer_Free((Sec_OpaqueBufferHandle*)pInBuf);
  }

  return retVal;
}

#ifdef SECAPI_V3
static gboolean allocate_v3_memory(void** outBuffer, const size_t length)
{
  if (outBuffer == NULL) {
    LOG(eError, "Cannot alocate secure buffer, outBuffer is null\n");
    return false;
  }

  Sec_OpaqueBufferHandle* bufferInfo = (Sec_OpaqueBufferHandle*) malloc(sizeof(Sec_OpaqueBufferHandle));
  bufferInfo->size = length;

  sa_svp_buffer buffer;
  const auto res = sa_svp_memory_alloc(&bufferInfo->svp_memory, length);

  if (SA_STATUS_OK != res) {
    LOG(eError, "Failed to allocate secure out buffer %d\n", res);
    free(bufferInfo);
    return false;
  }

  *outBuffer = reinterpret_cast<void*>(bufferInfo);

  return true;
}

static void free_v3_memory(void** buffer)
{
  if (buffer == NULL || *buffer == NULL)
    return;

  Sec_OpaqueBufferHandle* buf = reinterpret_cast<Sec_OpaqueBufferHandle*>(*buffer);

  if (sa_svp_memory_free(buf->svp_memory) != SA_STATUS_OK) {
    LOG(eError, "Failed to free v3 buffer\n");
  }

  free(buf);

  *buffer = NULL;
}

uint32_t svp_allocate_secure_buffers_v3_impl_default(void* pContext, void** ppInBuf, void** ppOutBuf, const uint8_t* pInData, const size_t nDataLen)
{
  LOG(eTrace, "Allocating secapi v3 native secure buffers, context: %p, ppInBuf: %p, ppOutBuf: %p, pIndata: %p, nDataLen: %zu\n",
      pContext, ppInBuf, ppOutBuf, pInData, nDataLen);

  // For the Amlogic platform the order of the IN and OUT buffers is important.  There is limited
  // heap space for SecAPI buffers so when the IN and OUT are allocated it takes most of the heap
  // space for large frames.  The IN buffer is freed after the decrypt and a new tranfser buffer
  // is allocated for full frame decryption.  Since the full frame buffer is slightly larger
  // (it includes the CENC clear data as well) it will not fit in the space vacated by the IN buffer
  // The solution it so allocate the IN buffer last so that when it is freed the space available
  // will include the IN buffer and the remaining free space

  if (ppOutBuf != NULL) {
    if (!allocate_v3_memory(ppOutBuf, nDataLen)) {
      LOG(eError, "Failed to allocate output buffer\n");
      return 0;
    }
  }

  if (ppInBuf != NULL) {
    if (!allocate_v3_memory(ppInBuf, nDataLen)) {
      LOG(eError, "Failed to allocate input buffer\n");

      free_v3_memory(ppOutBuf);

      return 0;
    }

    if (pInData != NULL) {
      sa_svp_offset offset {0, 0, nDataLen};
      sa_svp_buffer dest;

      Sec_OpaqueBufferHandle* buf = reinterpret_cast<Sec_OpaqueBufferHandle*>(*ppInBuf);

      const auto res = sa_svp_buffer_create(&dest, buf->svp_memory, buf->size);
      if (SA_STATUS_OK != res)
      {
        LOG(eError, "Failed to create secapi3 buffer %d\n", res);

        free_v3_memory(ppOutBuf);
        free_v3_memory(ppInBuf);

        return 0;
      }
      else
      {
        const auto copyRes = sa_svp_buffer_write(dest, pInData, nDataLen, &offset, 1);

        if (SA_STATUS_OK != copyRes) {
          LOG(eError, "Failed to copy input data into allocated input buffer: %d\n", copyRes);

          free_v3_memory(ppOutBuf);
          free_v3_memory(ppInBuf);

          return 0;
        }
      }
    }
  }

  return 1;
}

uint32_t svp_release_secure_buffers_v3_impl_default(void* pContext, void* pInBuf, void* pOutBuf, uint8_t* pDataOut, size_t nDataOutMax)
{
  uint32_t retVal = 0;

  void* secToken = NULL;

  svp_buffer_alloc_token(&secToken);
  svp_buffer_to_token(pContext, pOutBuf, secToken);

  free(pOutBuf);

  if (pDataOut != NULL) {
    memcpy((void*) pDataOut, secToken, svp_token_size());
  }
  svp_buffer_free_token(secToken);

  if( pInBuf != NULL) {
    free_v3_memory(&pInBuf);
  }

  return retVal;
}
#endif // SECAPI_V3