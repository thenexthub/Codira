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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_EXTERNAL_IFACE_TABLE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_EXTERNAL_IFACE_TABLE_H_

#include <functional>

#include "runtime/interpreter/frame.h"
#include "plugins/ets/runtime/job_queue.h"

namespace ark {
class Coroutine;
}  // namespace ark

namespace ark::ets {

class ExternalIfaceTable {
public:
    using JSEnv = void *;
    using ClearInteropHandleScopesFunction = std::function<void(Frame *)>;
    using CreateJSRuntimeFunction = std::function<JSEnv()>;
    using CleanUpJSEnvFunction = std::function<void(JSEnv)>;
    using GetJSEnvFunction = std::function<JSEnv()>;
    using CreateInteropCtxFunction = std::function<void(Coroutine *, JSEnv)>;

    NO_COPY_SEMANTIC(ExternalIfaceTable);
    NO_MOVE_SEMANTIC(ExternalIfaceTable);

    ExternalIfaceTable() = default;
    virtual ~ExternalIfaceTable() = default;

    JobQueue *GetJobQueue() const
    {
        return jobQueue_.get();
    }

    void SetJobQueue(PandaUniquePtr<JobQueue> jobQueue)
    {
        jobQueue_ = std::move(jobQueue);
    }

    const ClearInteropHandleScopesFunction &GetClearInteropHandleScopesFunction() const
    {
        return clearInteropHandleScopes_;
    }

    void SetClearInteropHandleScopesFunction(const ClearInteropHandleScopesFunction &func)
    {
        clearInteropHandleScopes_ = func;
    }

    void SetCreateJSRuntimeFunction(CreateJSRuntimeFunction &&cb)
    {
        ASSERT(!createJSRuntime_);
        createJSRuntime_ = std::move(cb);
    }

    void SetCleanUpJSEnvFunction(CleanUpJSEnvFunction &&cb)
    {
        ASSERT(!cleanUpJSEnv_);
        cleanUpJSEnv_ = std::move(cb);
    }

    void SetGetJSEnvFunction(GetJSEnvFunction &&cb)
    {
        ASSERT(!getJSEnv_);
        getJSEnv_ = std::move(cb);
    }

    void *CreateJSRuntime()
    {
        void *jsEnv = nullptr;
        if (createJSRuntime_) {
            jsEnv = createJSRuntime_();
        }
        return jsEnv;
    }

    void CleanUpJSEnv(JSEnv env)
    {
        if (cleanUpJSEnv_) {
            cleanUpJSEnv_(env);
        }
    }

    JSEnv GetJSEnv()
    {
        void *jsEnv = nullptr;
        if (getJSEnv_) {
            jsEnv = getJSEnv_();
        }
        return jsEnv;
    }

    void SetCreateInteropCtxFunction(CreateInteropCtxFunction &&cb)
    {
        ASSERT(!createInteropCtx_);
        createInteropCtx_ = std::move(cb);
    }

    void CreateInteropCtx(Coroutine *coro, JSEnv jsEnv)
    {
        if (createInteropCtx_) {
            createInteropCtx_(coro, jsEnv);
        }
    }

private:
    PandaUniquePtr<JobQueue> jobQueue_ = nullptr;
    ClearInteropHandleScopesFunction clearInteropHandleScopes_ = nullptr;
    CreateJSRuntimeFunction createJSRuntime_ = nullptr;
    CleanUpJSEnvFunction cleanUpJSEnv_ = nullptr;
    GetJSEnvFunction getJSEnv_ = nullptr;
    CreateInteropCtxFunction createInteropCtx_ = nullptr;
};

}  // namespace ark::ets
#endif  // #ifndef PANDA_PLUGINS_ETS_RUNTIME_EXTERNAL_IFACE_TABLE_H_
