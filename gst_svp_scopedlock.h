/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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

#ifndef __GST_SVP_SCOPEDLOCK_H__
#define __GST_SVP_SCOPEDLOCK_H__

#include <unistd.h>     // For usleep()
#include "gst_svp_logging.h"

#ifdef USE_LIBC_SCOPED_LOCK
#include <mutex>
#else
#include <pthread.h>
#endif

#ifdef USE_LIBC_SCOPED_LOCK
extern std::recursive_mutex _lock;
#define SCOPED_LOCK()     std::lock_guard<std::recursive_mutex> lock(_lock)
#define SCOPED_TRY_LOCK(timeoutMS, failRetVal)  \
    int     nTryCount = 0;                      \
    bool    bLockAcquired = false;              \
    while(nTryCount < timeoutMS) {              \
        bLockAcquired = _lock.try_lock();       \
        if(bLockAcquired == false) {            \
            nTryCount++;                        \
            usleep(1000); /* 1ms */             \
        }                                       \
        else {                                  \
            break;                              \
        }                                       \
    }                                           \
    if(bLockAcquired != true) {                 \
        LOG(eError, "%s failed to acquire mutex after %d ms\n", \
                    __FUNCTION__, timeoutMS);   \
        return failRetVal;                      \
    }                                           \
    SCOPED_LOCK();                              \
    _lock.unlock()
#else
class ScopedMutex
{
public:
    ScopedMutex(const char* strFN);
    ScopedMutex(const char* strFN, uint32_t timeoutMS);
    ~ScopedMutex();

    bool IsMutexLocked() { return _bMutexAcquired; };

private:
    void InitMutex(pthread_mutex_t* pLock);

    const char*             _strFN;
    bool                    _bMutexAcquired;

    static bool             _bMutexInit;
    static pthread_mutex_t  _lock;
};

#define SCOPED_LOCK()                               ScopedMutex lock(__FUNCTION__)
#define SCOPED_TRY_LOCK(timeoutMS, failRetVal)                  \
    ScopedMutex lock(__FUNCTION__, timeoutMS);                  \
    if(!lock.IsMutexLocked()) {                                 \
        LOG(eError, "%s failed to acquire mutex after %d ms\n", \
                    __FUNCTION__, timeoutMS);                   \
        return failRetVal;                                      \
    }
#endif

#endif //__GST_SVP_SCOPEDLOCK_H__
