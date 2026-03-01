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

#ifndef PANDA_VERIFIER_SYNCHRONIZED_HPP_
#define PANDA_VERIFIER_SYNCHRONIZED_HPP_

#include "verification/util/callable.h"

#include "libarkbase/os/mutex.h"

#include "libarkbase/macros.h"

#include <utility>

namespace ark::verifier {
template <class C, class Friend1 = C, class Friend2 = C>
class Synchronized {
    struct ConstProxy {
        ConstProxy() = delete;
        ConstProxy(const ConstProxy &) = delete;
        ConstProxy(ConstProxy &&other)
        {
            obj = other.obj;
            other.obj = nullptr;
        }
        const Synchronized *obj;  // NOLINT(misc-non-private-member-variables-in-classes)
        explicit ConstProxy(const Synchronized *paramObj) ACQUIRE_SHARED(obj->rwLock_) : obj {paramObj}
        {
            obj->rwLock_.ReadLock();
        }
        const C *operator->() const
        {
            ASSERT(obj != nullptr);
            return &obj->c_;
        }
        ~ConstProxy() RELEASE_SHARED(obj->rwLock_)
        {
            if (obj != nullptr) {
                obj->rwLock_.Unlock();
            }
        }
        NO_COPY_OPERATOR(ConstProxy);
        NO_MOVE_OPERATOR(ConstProxy);
    };

    struct Proxy {
        Proxy() = delete;
        Proxy(const Proxy &) = delete;
        Proxy(Proxy &&other)
        {
            obj = other.obj;
            other.obj = nullptr;
        }
        Synchronized *obj;  // NOLINT(misc-non-private-member-variables-in-classes)
        explicit Proxy(Synchronized *paramObj) ACQUIRE(obj->rwLock_) : obj {paramObj}
        {
            obj->rwLock_.WriteLock();
        }
        C *operator->()
        {
            ASSERT(obj != nullptr);
            return &obj->c_;
        }
        ~Proxy() RELEASE(obj->rwLock_)
        {
            if (obj != nullptr) {
                obj->rwLock_.Unlock();
            }
        }
        NO_COPY_OPERATOR(Proxy);
        NO_MOVE_OPERATOR(Proxy);
    };

    C c_;

    // GetObj() should be ideally annotated with REQUIRES/REQUIRES_SHARED and c with GUARDED_BY, but then the current
    // design of LibCache::FastAPI can't be annotated correctly
    C &GetObj()
    {
        return c_;
    }

    const C &GetObj() const
    {
        return c_;
    }

    void WriteLock() ACQUIRE(rwLock_)
    {
        rwLock_.WriteLock();
    }

    void ReadLock() ACQUIRE_SHARED(rwLock_)
    {
        rwLock_.ReadLock();
    }

    void Unlock() RELEASE_GENERIC(rwLock_)
    {
        rwLock_.Unlock();
    }

    friend Friend1;
    friend Friend2;

public:
    template <typename... Args>
    explicit Synchronized(Args &&...args) : c_(std::forward<Args>(args)...)
    {
    }

    ConstProxy operator->() const
    {
        return ConstProxy(this);
    }

    Proxy operator->()
    {
        return Proxy(this);
    }

    template <typename Handler>
    auto Apply(Handler &&handler)
    {
        os::memory::WriteLockHolder lockHolder {rwLock_};
        return handler(c_);
    }

    template <typename Handler>
    auto Apply(Handler &&handler) const
    {
        os::memory::ReadLockHolder lockHolder {rwLock_};
        return handler(c_);
    }

private:
    mutable ark::os::memory::RWLock rwLock_;
};
}  // namespace ark::verifier

#endif  // !PANDA_VERIFIER_SYNCHRONIZED_HPP_
