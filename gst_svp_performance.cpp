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

#include "gst_svp_logging.h"
#include "gst_svp_scopedlock.h"
#include "rdk_perf.h"


#define REPORTING_INITIAL_COUNT 5000
#define REPORTING_INTERVAL_COUNT 20000
#define TIMER_INTERVAL_SECONDS 10
#define MAX_DELAY 60
#define MAX_BUF_SIZE 2048
#define INITIAL_MIN_VALUE 1000000000
#define PROCESS_NAMELEN 80

static void __attribute__((constructor)) PerfModuleInit();
static void __attribute__((destructor)) PerfModuleTerminate();

// This function is assigned to execute as a library init
//  using __attribute__((constructor))
static void PerfModuleInit()
{
    char cmd[80] = { 0 };
    char strProcessName[PROCESS_NAMELEN];

    sprintf(cmd, "/proc/%d/cmdline", getpid());
    FILE* fp = fopen(cmd,"r");
    if(fp != NULL) {
        size_t size = fread(strProcessName, sizeof(char), PROCESS_NAMELEN - 1, fp);
        if(size <= 0) {
            LOG(eError, "Could not read process name\n");
        }
        fclose(fp);
    }

    LOG(eWarning, "GST performance process initialize %X named %s\n", getpid(), strProcessName);

    return;
}
  
// This function is assigned to execute as library unload
// using __attribute__((destructor))
static void PerfModuleTerminate()
{
    return;
}

#if 0
static bool load_library(const char * szLibraryName, void** ppHandle)
{
    dlerror();  // clear error

    void * library_handle = dlopen(szLibraryName, RTLD_LAZY);
    if(library_handle == NULL) {
        char* error = dlerror();
        if (error != NULL) {
            LOG(eError, "Could not open library <%s> error = %s\n", szLibraryName, error);
        }
        LOG(eError, "ERROR: could not open library %s\n", szLibraryName);
        *ppHandle = NULL;

        return false;
    }

    // Success
    *ppHandle = library_handle;
    return true;
}

static bool link_function(void* pLibrary, void** ppFunc, const char* szFuncName)
{
    *ppFunc = dlsym(pLibrary, szFuncName);
    if (!*ppFunc) {
        /* no such symbol */
        LOG(eError, "Error for %s : %s\n", szFuncName, dlerror());
        return false;
    }
    return true;
}
#endif

#ifdef ENABLE_OCDM_PROFILING
#error ENABLE_OCDM_PROFILING
// Forward declarations
typedef uint32_t OpenCDMError;
struct OpenCDMSession;
#ifndef EXTERNAL
    #ifdef _MSVC_LANG
        #ifdef OCDM_EXPORTS
        #define EXTERNAL __declspec(dllexport)
        #else
        #define EXTERNAL __declspec(dllimport)
        #endif
    #else
        #define EXTERNAL __attribute__ ((visibility ("default")))
    #endif
#endif

EXTERNAL OpenCDMError opencdm_session_decrypt(struct OpenCDMSession* session,
                                    uint8_t encrypted[],
                                    const uint32_t encryptedLength,
                                    const uint8_t* IV, uint16_t IVLength,
                                    const uint8_t* keyId, const uint16_t keyIdLength,
                                    uint32_t initWithLast15,
                                    uint8_t* streamInfo,
                                    uint16_t streamInfoLength);

uint32_t (*lcl_opencdm_session_decrypt)(struct OpenCDMSession* session,
                                    uint8_t encrypted[],
                                    const uint32_t encryptedLength,
                                    const uint8_t* IV, uint16_t IVLength,
                                    const uint8_t* keyId, const uint16_t keyIdLength,
                                    uint32_t initWithLast15,
                                    uint8_t* streamInfo,
                                    uint16_t streamInfoLength);

OpenCDMError GstPerf_opencdm_session_decrypt(struct OpenCDMSession* session,
                                                    uint8_t encrypted[],
                                                    const uint32_t encryptedLength,
                                                    const uint8_t* IV, uint16_t IVLength,
                                                    const uint8_t* keyId, const uint16_t keyIdLength,
                                                    uint32_t initWithLast15,
                                                    uint8_t* streamInfo,
                                                    uint16_t streamInfoLength)
{
    static void *library_handle = NULL;
    static bool bInitLibrary = false;
    static const char* szLibraryName = "libocdm.so";
    
    if(bInitLibrary == false) {
        if(library_handle == NULL) {
            load_library(szLibraryName, &library_handle);
            LOG(eWarning, "library_handle = %p\n", library_handle);
        }
        if(library_handle) {
            // Library loaded, link symbol
            if(link_function(library_handle, (void**)&lcl_opencdm_session_decrypt, "opencdm_session_decrypt")) {
                // Success
                bInitLibrary = true;
            }
        }
        else {
            LOG(eError, "Invalid Library handle for %s\n", szLibraryName);
        }
    }

    RDKPerf perf("opencdm_session_decrypt");

    if(lcl_opencdm_session_decrypt) {
        return lcl_opencdm_session_decrypt(session, encrypted, encryptedLength, IV, IVLength, 
                                    keyId, keyIdLength, initWithLast15, 
                                    streamInfo, streamInfoLength);
    }
    else {
        LOG(eError, "Dynamic function not linked\n");
        return 0x80004005;      // ERROR_FAIL
    }
}
#endif // ENABLE_OCDM_PROFILING
