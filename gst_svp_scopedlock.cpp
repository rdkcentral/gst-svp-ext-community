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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#ifdef USE_LIBC_SCOPED_LOCK
#include <mutex>
#else
#include <pthread.h>
#endif

#include "gst_svp_logging.h"
#include "gst_svp_scopedlock.h"

#ifdef USE_LIBC_SCOPED_LOCK

std::recursive_mutex _lock;

#else // USE_LIBC_SCOPED_LOCK
ScopedMutex::ScopedMutex(const char* strFN) 
: _strFN(strFN)
, _bMutexAcquired(false)
{
    if(!_bMutexInit) {
        InitMutex(&_lock);
    }
    
    if(_bMutexInit) {
        if(pthread_mutex_lock(&_lock) == 0) {
            _bMutexAcquired = true;
        }
    }
    else {
        LOG(eError, "Mutex was not initialized for %s\n", _strFN);
    }
}
ScopedMutex::ScopedMutex(const char* strFN, uint32_t timeoutMS)
: _strFN(strFN)
, _bMutexAcquired(false)
{
    uint32_t nCount = 0;

    if(!_bMutexInit) {
        InitMutex(&_lock);
    }
    
    if(_bMutexInit) {
        while(nCount < timeoutMS) {
            if(pthread_mutex_trylock(&_lock) != 0) {
                // Mutex not acquired
                usleep(1000); //1 ms
                nCount++;
            }
            else {
                // mutex acquired
                _bMutexAcquired = true;
                break;
            }
        }
    }
    else {
        LOG(eError, "Mutex was not initialized for %s\n", _strFN);
    }
}
ScopedMutex::~ScopedMutex() 
{
    if(_bMutexAcquired) {
        pthread_mutex_unlock(&_lock);
        _bMutexAcquired = false;
    }
}
void ScopedMutex::InitMutex(pthread_mutex_t* pLock)
{
    pthread_mutexattr_t Attr;
    pthread_mutexattr_init(&Attr);
    pthread_mutexattr_settype(&Attr, PTHREAD_MUTEX_RECURSIVE);
    if (pthread_mutex_init(pLock, &Attr) != 0) {
        LOG(eError, "\n mutex init failed for %s\n", _strFN);
    }
    else {
        _bMutexInit = true;
    }
    return;
}

pthread_mutex_t ScopedMutex::_lock;
bool ScopedMutex::_bMutexInit = false;
#endif //USE_LIBC_SCOPED_LOCK


