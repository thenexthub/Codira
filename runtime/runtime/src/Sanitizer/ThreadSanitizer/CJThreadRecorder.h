/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */


#ifndef CODIRARUNTIME_CODETHREADRECORDER_H
#define CODIRARUNTIME_CODETHREADRECORDER_H

#include <cstdint>
#include <map>

#include "Base/Log.h"
#include "Base/LogFile.h"
#include "Base/RwLock.h"

namespace MapleRuntime {

template<typename T>
class CODEThreadRecorder {
public:
    CODEThreadRecorder() = default;

    void CreateThread(void* thread, T data)
    {
        lock.LockWrite();
        map[thread] = data;
        lock.UnlockWrite();
    }

    T GetDataFromThread(void* thread)
    {
        lock.LockRead();
        auto it = map.find(thread);
        if (it == map.end()) {
            lock.UnlockRead();
            return T();
        }
        T result = it->second;
        lock.UnlockRead();
        return result;
    }

    T DeleteThread(void* thread)
    {
        lock.LockWrite();
        auto it = map.find(thread);
        if (it == map.end()) {
            lock.UnlockWrite();
            return T();
        }
        T result = it->second;
        map.erase(it);
        lock.UnlockWrite();
        return result;
    }

    template<typename Res>
    void Traverse(std::function<bool(void*, T, Res*)> func, Res* result)
    {
        lock.LockRead();
        for (auto kv : this->map) {
            if (func(kv.first, kv.second, result)) {
                lock.UnlockRead();
                return;
            }
        }
        lock.UnlockRead();
    }

private:
    std::map<void*, T> map;
    RwLock lock;
};
}
#endif // CODIRARUNTIME_CODETHREADRECORDER_H
