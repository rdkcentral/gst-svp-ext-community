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


#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include <string>
#include <map>

#include <gst/gst.h>

extern "C" {
#include <sec_security_datatype.h>
#include <sec_security.h>
}

#include "gst_svp_logging.h"
#include "gst_svp_scopedlock.h"
#include "rdk_perf.h"

Sec_Result GstPerf_SecOpaqueBuffer_Malloc(SEC_SIZE bufLength, Sec_OpaqueBufferHandle **handle)
{
    RDKPerf perf("SecOpaqueBuffer_Malloc");
    return SecOpaqueBuffer_Malloc(bufLength, handle);
}

Sec_Result GstPerf_SecOpaqueBuffer_Write(Sec_OpaqueBufferHandle *handle, SEC_SIZE offset, SEC_BYTE *data, SEC_SIZE length)
{
    RDKPerf perf("SecOpaqueBuffer_Write");
    return SecOpaqueBuffer_Write(handle, offset, data, length);
}

Sec_Result GstPerf_SecOpaqueBuffer_Free(Sec_OpaqueBufferHandle *handle)
{
    RDKPerf perf("SecOpaqueBuffer_Free");
    return SecOpaqueBuffer_Free(handle);
}

Sec_Result GstPerf_SecOpaqueBuffer_Release(Sec_OpaqueBufferHandle *handle, Sec_ProtectedMemHandle **svpHandle)
{
    RDKPerf perf("SecOpaqueBuffer_Release");
    return SecOpaqueBuffer_Release(handle, svpHandle);
}

Sec_Result GstPerf_SecOpaqueBuffer_Copy(Sec_OpaqueBufferHandle *out, SEC_SIZE out_offset, Sec_OpaqueBufferHandle *in, SEC_SIZE in_offset, SEC_SIZE num_to_copy)
{
    RDKPerf perf("SecOpaqueBuffer_Copy");
    return SecOpaqueBuffer_Copy(out, out_offset, in, in_offset, num_to_copy);
}
