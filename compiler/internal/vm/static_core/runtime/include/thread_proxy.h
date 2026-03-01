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
#ifndef PANDA_RUNTIME_THREAD_PROXY_H
#define PANDA_RUNTIME_THREAD_PROXY_H

#include "runtime/include/thread_proxy_static.h"
#include "runtime/include/thread_proxy_hybrid.h"

namespace ark {

class MutatorLock;

#ifndef ARK_HYBRID
class ThreadProxy : public ThreadProxyStatic {
public:
    explicit ThreadProxy(MutatorLock *mutatorLock) : ThreadProxyStatic(mutatorLock) {}
};
#else
class ThreadProxy : public ThreadProxyHybrid {
public:
    explicit ThreadProxy(MutatorLock *mutatorLock) : ThreadProxyHybrid(mutatorLock) {}
};
#endif

}  // namespace ark

#endif  // PANDA_RUNTIME_THREAD_PROXY_H
