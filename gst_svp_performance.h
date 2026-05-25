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


#ifndef __GST_SVP_PERFORMANCE_H__
#define __GST_SVP_PERFORMANCE_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>


#include "rdk_perf.h" // RDK Performance metrics library
// Redirect GstPerf to RDKPerf
#define GstPerf RDKPerf

// Feature Control
#define GST_SVP_PERF 1

#if !defined(__OCDM_WRAPPER_H_)
    // Only define for OCDM component, 
    //#define ENABLE_OCDM_PROFILING 1
#endif

#ifdef GST_SVP_PERF

#ifdef ENABLE_OCDM_PROFILING
// OCDM
#define opencdm_session_decrypt(a, b, c, d, e, f, g, h, i, j) GstPerf_opencdm_session_decrypt(a, b, c, d, e, f, g, h, i, j)
#endif // ENABLE_OCDM_PROFILING
// Netflix
#endif // GST_SVP_PERF

#ifdef ENABLE_OCDM_PROFILING
// OCDM Decrypt
#if !defined(__OCDM_WRAPPER_H_) // Is open_cdm.h in the include path already
// Forward declarations
typedef uint32_t OpenCDMError;
struct OpenCDMSession;
#endif

OpenCDMError GstPerf_opencdm_session_decrypt(struct OpenCDMSession* session,
                                                    uint8_t encrypted[],
                                                    const uint32_t encryptedLength,
                                                    const uint8_t* IV, uint16_t IVLength,
                                                    const uint8_t* keyId, const uint16_t keyIdLength,
                                                    uint32_t initWithLast15,
                                                    uint8_t* streamInfo,
                                                    uint16_t streamInfoLength);
#endif //ENABLE_OCDM_PROFILING

#endif // __GST_SVP_PERFORMANCE_H__
