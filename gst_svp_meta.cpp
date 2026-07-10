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
#include "gst_svp_meta.h"
#include "gst_svp_meta_private.h"
#include "gst_svp_logging.h"

static struct {
    svp_allocate_secure_buffers_t   svp_allocate_secure_buffers;
    svp_release_secure_buffers_t    svp_release_secure_buffers;

    svp_allocate_secure_buffer_t   svp_allocate_secure_buffer;
    svp_free_secure_buffer_t    svp_free_secure_buffer;
    svp_release_secure_buffer_t   svp_release_secure_buffer;

    svp_set_context_flag_t svp_context_set_v3;
    svp_get_context_flag_t svp_context_get_v3;

    svp_alloc_sec_mem_t   svp_alloc_sec_mem;
    svp_release_sec_mem_t svp_release_sec_mem;

    svp_support_mem_prealloc_t svp_support_sec_mem_prealloc;

    svp_create_platform_allocator_t svp_create_platform_allocator;

    svp_is_buffer_using_secure_memory_t svp_is_buffer_using_secure_memory;

    gst_svp_ext_transform_caps_clear_t svp_ext_transform_caps_clear;

    svp_header_extract_output_buffer_t svp_header_extract_output_buffer;
}s_gst_svp_context;

void  __attribute__((weak)) gst_svp_init_device_context()
{
    // Devices may implement this function to register
    // its own specific implementations.
    LOG(eTrace, "gst_svp_init_device_context not implemented for this device\n");
}

void svp_context_set_secapi_v3_default(void * pContext, const gboolean wantV3);
gboolean svp_context_get_secapi_v3_default(void * pContext);
void * gst_svp_ext_create_platform_allocator_default();

static void __attribute__((constructor)) gst_svp_init();
static void __attribute__((destructor)) gst_svp_terminate();

// This function is assigned to execute as a library init
//  using __attribute__((constructor))
static void gst_svp_init()
{
    svp_allocate_secure_buffers_set(&svp_allocate_secure_buffers_v2_impl_default);
    svp_release_secure_buffers_set(&svp_release_secure_buffers_v2_impl_default);

    svp_allocate_secure_buffer_set(&svp_allocate_secure_buffer_default);
    svp_free_secure_buffer_set(&svp_free_secure_buffer_default);
    svp_release_secure_buffer_set(&svp_release_secure_buffer_default);

    svp_set_context_v3_set(&svp_context_set_secapi_v3_default);
    svp_get_context_v3_set(&svp_context_get_secapi_v3_default);

    svp_allocate_secure_memory_set(&allocate_sec_mem_default);
    svp_release_secure_memory_set(&release_sec_mem_default);

    svp_support_mem_prealloc_set(&support_sec_mem_prealloc_default);

    svp_create_platform_allocator_set(&gst_svp_ext_create_platform_allocator_default);

    gst_svp_is_buffer_using_secure_memory_set(&gst_svp_is_buffer_using_secure_memory_default);

    gst_svp_ext_transform_caps_clear_set(&gst_svp_ext_transform_caps_clear_default);

    svp_header_extract_output_buffer_set(&svp_header_extract_output_buffer_default);

    gst_svp_init_device_context();
}

gboolean allocate_sec_mem_default(void * pContext, void * handle, gsize * const physicalDataSize)
{
    LOG(eError, "No implementation provided to allocate sec mem");
    return false;
}

gboolean release_sec_mem_default(void * pContext, void * handle)
{
    LOG(eError, "No implementation provided to release sec mem");
    return false;
}

gboolean support_sec_mem_prealloc_default(void * pContext)
{
    /*
        No log as we expect this to be called during normal runtime
        and not all platforms will implement this
    */
    return false;
}

void * gst_svp_ext_create_platform_allocator_default()
{
    LOG(eError, "No platform allocator is implemented\n");
    return nullptr;
}

gboolean gst_svp_is_buffer_using_secure_memory_default(void * pContext, GstBuffer * buffer)
{
    LOG(eError, "No implementation provided\n");
    return false;
}

void gst_svp_ext_transform_caps_clear_default(void * pContext, GstCaps* caps)
{
    LOG(eError, "No implementation provided\n");
}

void * svp_header_extract_output_buffer_default(void * headerData)
{
    LOG(eError, "No implementation provided\n");
    return nullptr;
}

// This function is assigned to execute as library unload
// using __attribute__((destructor))
static void gst_svp_terminate()
{

}

gboolean gst_svp_ext_get_context(void ** ppContext, context_type type, unsigned int rpcID)
{
    RDKPerf perf(__FUNCTION__);
    return gst_svp_ext_get_context_impl(ppContext, type, rpcID);
}

gboolean gst_svp_ext_context_set_caps(void * pContext, GstCaps *caps)
{
    RDKPerf perf(__FUNCTION__);
    return gst_svp_ext_context_set_caps_impl(pContext, caps);
}

gboolean gst_svp_ext_free_context(void * pContext)
{
    RDKPerf perf(__FUNCTION__);
    return gst_svp_ext_free_context_impl(pContext);
}

gboolean gst_buffer_svp_transform_from_cleardata(void * pContext, GstBuffer* buffer, media_type mediaType)
{
    RDKPerf perf(__FUNCTION__);
    return gst_buffer_svp_transform_from_cleardata_impl(pContext, buffer, mediaType);
}

gboolean gst_svp_ext_transform_caps(GstCaps **caps, gboolean bEncrypted)
{
    RDKPerf perf(__FUNCTION__);
    return gst_svp_ext_transform_caps_impl(caps, bEncrypted);
}

gboolean gst_buffer_append_svp_transform(void * pContext, GstBuffer* buffer, GstBuffer* subSampleBuffer, const guint32 subSampleCount, guint8* encryptedData, const guint32 mappedDataSize)
{
    RDKPerf perf(__FUNCTION__);
    return gst_buffer_append_svp_transform_impl(pContext, buffer, subSampleBuffer, subSampleCount, encryptedData, mappedDataSize);
}

gboolean gst_buffer_append_svp_metadata(GstBuffer* buffer,  svp_meta_data_t* svp_metadata, const guint32 mappedDataSize)
{
    RDKPerf perf(__FUNCTION__);
    return gst_buffer_append_svp_metadata_impl(buffer,svp_metadata,mappedDataSize);
}

gboolean svp_buffer_to_token(void * pContext, void* svp_handle, void* svp_token)
{
    RDKPerf perf(__FUNCTION__);
    return svp_buffer_to_token_impl(pContext, svp_handle, svp_token);
}

gboolean svp_buffer_from_token(void * pContext, void* svp_token, void* svp_handle)
{
    RDKPerf perf(__FUNCTION__);
    return svp_buffer_from_token_impl(pContext, svp_token, svp_handle);
}

guint32 svp_token_size(void)
{
    RDKPerf perf(__FUNCTION__);
    return svp_token_size_impl();
}

gboolean svp_buffer_alloc_token(void **token)
{
    RDKPerf perf(__FUNCTION__);
    return svp_buffer_alloc_token_impl(token);
}

gboolean svp_buffer_free_token(void *token)
{
    RDKPerf perf(__FUNCTION__);
    return  svp_buffer_free_token_impl(token);
}

void svpGetDrmOEMContext(void ** ppdrmOemContext)
{
    RDKPerf perf(__FUNCTION__);
    svpGetDrmOEMContextImpl(ppdrmOemContext);
    return;
}

void svpGetDrmPlatformInitData( void ** ppPlatformInitData)
{
    RDKPerf perf(__FUNCTION__);
    svpGetDrmPlatformInitDataImpl(ppPlatformInitData);
    return;
}

bool svpIsAudioNeedNonSVPContext( void )
{
    RDKPerf perf(__FUNCTION__);
    return svpIsAudioNeedNonSVPContextImpl();
}

bool svpIsVideoResCheckNeed( void )
{
    RDKPerf perf(__FUNCTION__);
    return svpIsVideoResCheckNeedImpl();
}

bool svpIsDynamicSVPEncEnabled( void )
{
    RDKPerf perf(__FUNCTION__);
    return svpIsDynamicSVPEncEnabledImpl();
}

bool svpIsMultipleOpaqueSupportCTR( void )
{
    RDKPerf perf(__FUNCTION__);
    return svpIsMultipleOpaqueSupportCTRImpl();
}

bool svpSetHandleToTEE( void* pHandle )
{
    RDKPerf perf(__FUNCTION__);
    return svpSetHandleToTEEImpl( pHandle );
}

bool svpLoadRevocationList(void)
{
    RDKPerf perf(__FUNCTION__);
    return svpLoadRevocationListImpl();
}

void svpPlatformInitializePlayready(void)
{
    RDKPerf perf(__FUNCTION__);
    svpPlatformInitializePlayreadyImpl();
    return;
}

void svpPlatformInitializeWidevine(void)
{
    RDKPerf perf(__FUNCTION__);
    svpPlatformInitializeWidevineImpl();
    return;
}

void svpPlatformUninitializePlayready(void)
{
    RDKPerf perf(__FUNCTION__);
    svpPlatformUninitializePlayreadyImpl();
    return;
}

void svpPlatformUninitializeWidevine(void)
{
    RDKPerf perf(__FUNCTION__);
    svpPlatformUninitializeWidevineImpl();
    return;
}

bool svpIsSecureClockInitNeed(void)
{
    RDKPerf perf(__FUNCTION__);
    return svpIsSecureClockInitNeedImpl();
}

void svpGetDrmStoragePath(std::string& readDir, std::string& storagePath, std::string& storeLocation)
{
    RDKPerf perf(__FUNCTION__);
    svpGetDrmStoragePathImpl(readDir, storagePath, storeLocation);
    return;
}

gboolean svp_pipeline_buffers_available(void * pContext, media_type mediaType)
{
    RDKPerf perf(__FUNCTION__);
    return  svp_pipeline_buffers_available_impl(pContext, mediaType);
}

gboolean gst_buffer_append_init_metadata(GstBuffer * buffer)
{
    RDKPerf perf(__FUNCTION__);
    return  gst_buffer_append_init_metadata_impl(buffer);
}

uint32_t svp_allocate_secure_buffers(void* pContext, void** ppInBuf, void** ppOutBuf, const uint8_t* pInData, const size_t nDataLen)
{
    RDKPerf perf(__FUNCTION__);
    return s_gst_svp_context.svp_allocate_secure_buffers(pContext, ppInBuf, ppOutBuf, pInData, nDataLen);
}

uint32_t svp_release_secure_buffers(void* pContext, void* pInBuf, void* pOutBuf, uint8_t* pDataOut, size_t nDataOutMax)
{
    RDKPerf perf(__FUNCTION__);
    return s_gst_svp_context.svp_release_secure_buffers(pContext, pInBuf, pOutBuf, pDataOut, nDataOutMax);
}

gboolean svp_allocate_secure_buffer(void* pContext, void ** ppBuf, const uint8_t* pInData, const size_t nDataLen)
{
    RDKPerf perf(__FUNCTION__);
    return s_gst_svp_context.svp_allocate_secure_buffer(pContext, ppBuf, pInData, nDataLen);
}
gboolean svp_free_secure_buffer(void* pContext, void * pBuf)
{
    RDKPerf perf(__FUNCTION__);
    return s_gst_svp_context.svp_free_secure_buffer(pContext, pBuf);
}
gboolean svp_release_secure_buffer(void* pContext, void * pBuf)
{
    RDKPerf perf(__FUNCTION__);
    return s_gst_svp_context.svp_release_secure_buffer(pContext, pBuf);
}

gboolean gst_svp_context_supports_memory_prealloc(void * pContext)
{
    RDKPerf perf(__FUNCTION__);
    return s_gst_svp_context.svp_support_sec_mem_prealloc(pContext);
}

gboolean gst_svp_allocate_sec_mem(void * pContext, void * handle, gsize * const physicalDataSize)
{
    RDKPerf perf(__FUNCTION__);
    return s_gst_svp_context.svp_alloc_sec_mem(pContext, handle, physicalDataSize);
}

gboolean gst_svp_release_sec_mem(void * pContext, void * handle)
{
    RDKPerf perf(__FUNCTION__);
    return s_gst_svp_context.svp_release_sec_mem(pContext, handle);
}

void svp_context_set_secapi_v3(void * pContext, const gboolean wantV3)
{
    s_gst_svp_context.svp_context_set_v3(pContext, wantV3);
}

gboolean svp_context_get_secapi_v3(void * pContext)
{
    return s_gst_svp_context.svp_context_get_v3(pContext);
}

void svp_allocate_secure_buffers_set(svp_allocate_secure_buffers_t pFunc)
{
    s_gst_svp_context.svp_allocate_secure_buffers = pFunc;
}

void svp_release_secure_buffers_set(svp_release_secure_buffers_t pFunc)
{
    s_gst_svp_context.svp_release_secure_buffers = pFunc;
}

void svp_allocate_secure_buffer_set(svp_allocate_secure_buffer_t pFunc)
{
    s_gst_svp_context.svp_allocate_secure_buffer = pFunc;
}

void svp_free_secure_buffer_set(svp_free_secure_buffer_t pFunc)
{
    s_gst_svp_context.svp_free_secure_buffer = pFunc;
}

void svp_release_secure_buffer_set(svp_release_secure_buffer_t pFunc)
{
    s_gst_svp_context.svp_release_secure_buffer = pFunc;
}

void svp_set_context_v3_set(svp_set_context_flag_t pFunc)
{
    s_gst_svp_context.svp_context_set_v3 = pFunc;
}

void svp_get_context_v3_set(svp_get_context_flag_t pFunc)
{
    s_gst_svp_context.svp_context_get_v3 = pFunc;
}

void svp_context_set_secapi_v3_default(void * pContext, const gboolean wantV3)
{
    LOG(eWarning, "svp_context_set_secapi_v3 is not implemented for this platform\n");
}

gboolean svp_context_get_secapi_v3_default(void * pContext)
{
    LOG(eWarning, "svp_context_get_secapi_v3 is not implemented for this platform, returning false\n");
    return false;
}

void svp_allocate_secure_memory_set(svp_alloc_sec_mem_t pFunc)
{
    s_gst_svp_context.svp_alloc_sec_mem = pFunc;
}

void svp_release_secure_memory_set(svp_release_sec_mem_t pFunc)
{
    s_gst_svp_context.svp_release_sec_mem = pFunc;
}

void svp_support_mem_prealloc_set(svp_support_mem_prealloc_t pFunc)
{
    s_gst_svp_context.svp_support_sec_mem_prealloc = pFunc;
}

void svp_header_extract_output_buffer_set(svp_header_extract_output_buffer_t pFunc)
{
    s_gst_svp_context.svp_header_extract_output_buffer = pFunc;
}

void * svp_header_extract_output_buffer(void * headerData)
{
    return s_gst_svp_context.svp_header_extract_output_buffer(headerData);
}

void gst_svp_is_buffer_using_secure_memory_set(svp_is_buffer_using_secure_memory_t pFunc)
{
    s_gst_svp_context.svp_is_buffer_using_secure_memory = pFunc;
}

void gst_svp_ext_transform_caps_clear_set(gst_svp_ext_transform_caps_clear_t pFunc)
{
    s_gst_svp_context.svp_ext_transform_caps_clear = pFunc;
}

gboolean gst_svp_is_buffer_using_secure_memory(void * pContext, GstBuffer * buffer)
{
    return s_gst_svp_context.svp_is_buffer_using_secure_memory(pContext, buffer);
}

void gst_svp_ext_transform_caps_clear(void * pContext, GstCaps* caps)
{
    s_gst_svp_context.svp_ext_transform_caps_clear(pContext, caps);
}
void svp_create_platform_allocator_set(svp_create_platform_allocator_t pFunc)
{
    s_gst_svp_context.svp_create_platform_allocator = pFunc;
}

void * gst_svp_ext_create_platform_allocator()
{
    RDKPerf perf(__FUNCTION__);
    return s_gst_svp_context.svp_create_platform_allocator();
}
