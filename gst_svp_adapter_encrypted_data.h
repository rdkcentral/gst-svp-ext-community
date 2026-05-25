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

#ifndef __GST_SVP_ADAPTER_ENCRYPTED_DATA_HEADER_H__
#define __GST_SVP_ADAPTER_ENCRYPTED_DATA_HEADER_H__

#ifndef __cplusplus
#include <stdint.h>
#include <string.h>
#else
#include <cstdint>
#include <cstring>
#endif

/**
 * The purpose of this struct is to add a layer of communication to
 * the secapi2-adapter. Specifically this is used during PlayReady 
 * decryption to allow for use of pre-allocated secure buffers
 * rather than allocating a secure buffer during decryption.
*/
#ifdef __cplusplus
extern "C" {
#endif
typedef struct AdapterEncryptedData_t {
  uint8_t GUID[16];
  void * encryptedData;
  uint32_t encryptedDataSize;
  uint32_t preallocatedHandle;
  const uint32_t * subsampleMap;
  uint32_t subsampleMapCount;
} AdapterEncryptedData;
#ifdef __cplusplus
};
#endif

static const uint8_t ADAPTER_ENCRYPTED_DATA_HEADER[16] = {0x32, 0x92, 0xaa, 0x2c, 0x9e, 0x58, 0x4e, 0xa4, 0xa4, 0x64, 0xfd, 0xb7, 0xa1, 0xc2, 0x62, 0xe3};

static inline void gst_svp_init_adapter_encrypted_data(AdapterEncryptedData* data)
{
    if (!data)
    {
      return;
    }

    memcpy(data->GUID, ADAPTER_ENCRYPTED_DATA_HEADER, 16);

    data->encryptedData = NULL;
    data->encryptedDataSize = 0;
    data->preallocatedHandle = 0;
    data->subsampleMap = NULL;
    data->subsampleMapCount = 0;
}

static inline int gst_svp_is_adapter_encrypted_data(void * data, size_t dataSize)
{
  if (dataSize >= sizeof(AdapterEncryptedData) && memcmp(data, ADAPTER_ENCRYPTED_DATA_HEADER, 16) == 0)
  {
    return 0;
  }

  return 1;
}

#endif /* __GST_SVP_ADAPTER_ENCRYPTED_DATA_HEADER_H__ */