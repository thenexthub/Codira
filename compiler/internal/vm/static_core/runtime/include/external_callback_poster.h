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

#ifndef PANDA_RUNTIME_EXTERNAL_CALLBACK_POSTER_H
#define PANDA_RUNTIME_EXTERNAL_CALLBACK_POSTER_H

#include <functional>

#include "libarkbase/macros.h"
#include "runtime/include/mem/panda_smart_pointers.h"

namespace ark {

class Coroutine;

/// @brief Interface of class that should post a callback to remote side
class CallbackPoster {
public:
    CallbackPoster() = default;
    virtual ~CallbackPoster() = default;
    NO_COPY_SEMANTIC(CallbackPoster);
    NO_MOVE_SEMANTIC(CallbackPoster);

    template <typename... Args>
    void Post(int64_t delayMs, Args... args)
    {
        static_assert(sizeof...(args) == 0);
        PostImpl(delayMs);
    }

    template <class Callback, class... Args>
    void Post(Callback callback, Args... args)
    {
        static_assert(std::is_invocable_v<Callback, Args...>);
        static_assert(std::is_void_v<std::result_of_t<Callback(Args...)>>);
        PostImpl([callback = std::move(callback), targs = std::tuple<Args...>(std::move(args)...)]() {
            std::apply(std::move(callback), std::move(targs));
        });
    }

    void SetDestroyInPlace()
    {
        destroyInPlace_ = true;
    }

    bool NeedDestroyInPlace() const
    {
        return destroyInPlace_;
    }

protected:
    using WrappedCallback = std::function<void()>;

    virtual void PostImpl(WrappedCallback &&callback) = 0;

    virtual void PostImpl([[maybe_unused]] int64_t delayMs) {}

private:
    bool destroyInPlace_ = false;
};

class CallbackPosterFactoryIface {
public:
    CallbackPosterFactoryIface() = default;
    virtual ~CallbackPosterFactoryIface() = default;
    NO_COPY_SEMANTIC(CallbackPosterFactoryIface);
    NO_MOVE_SEMANTIC(CallbackPosterFactoryIface);

    virtual PandaUniquePtr<CallbackPoster> CreatePoster() = 0;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_EXTERNAL_CALLBACK_POSTER_H
