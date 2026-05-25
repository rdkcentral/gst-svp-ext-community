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


#ifndef __GST_SVP_PERFORMANCE_SECAPI_H__
#define __GST_SVP_PERFORMANCE_SECAPI_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <gst/gst.h>

#include "gst_svp_performance.h"

extern "C" {
#include <sec_security_datatype.h>
#include <sec_security.h>
}

// Feature Control
#define GST_SVP_PERF 1

#ifdef GST_SVP_PERF
// Sec API Opaque Buffers
#define SecOpaqueBuffer_Malloc(a, b) GstPerf_SecOpaqueBuffer_Malloc(a, b)
#define SecOpaqueBuffer_Write(a, b, c, d) GstPerf_SecOpaqueBuffer_Write(a, b, c, d)
#define SecOpaqueBuffer_Free(a) GstPerf_SecOpaqueBuffer_Free(a)
#define SecOpaqueBuffer_Release(a, b) GstPerf_SecOpaqueBuffer_Release(a, b)
#define SecOpaqueBuffer_Copy(a, b, c, d, e) GstPerf_SecOpaqueBuffer_Copy(a, b, c, d, e)

#endif // GST_SVP_PERF

// SecAPI Opaque Buffer Functions
inline Sec_Result GstPerf_SecOpaqueBuffer_Malloc(SEC_SIZE bufLength, Sec_OpaqueBufferHandle **handle);
inline Sec_Result GstPerf_SecOpaqueBuffer_Write(Sec_OpaqueBufferHandle *handle, SEC_SIZE offset, SEC_BYTE *data, SEC_SIZE length);
inline Sec_Result GstPerf_SecOpaqueBuffer_Free(Sec_OpaqueBufferHandle *handle);
inline Sec_Result GstPerf_SecOpaqueBuffer_Release(Sec_OpaqueBufferHandle *handle, Sec_ProtectedMemHandle **svpHandle);
inline Sec_Result GstPerf_SecOpaqueBuffer_Copy(Sec_OpaqueBufferHandle *out, SEC_SIZE out_offset, Sec_OpaqueBufferHandle *in, SEC_SIZE in_offset, SEC_SIZE num_to_copy);

#endif // __GST_SVP_PERFORMANCE_SECAPI_H__
