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

#ifndef JSVM_ENV_H
#define JSVM_ENV_H
#include <functional>
#include <list>
#include <mutex>
#include <vector>

#include "jsvm_dfx.h"
#include "jsvm_inspector_agent.h"
#include "jsvm_reference.h"
#include "jsvm_types.h"
#include "jsvm_util.h"
#include "memory_manager.h"
#include "type_conversion.h"
#include "v8.h"

inline JSVM_Status ClearLastError(JSVM_Env env);

struct JSVM_Env__ final {
public:
    explicit JSVM_Env__(v8::Local<v8::Context> context, int32_t apiVersion)
        : isolate(context->GetIsolate()), contextPersistent(isolate, context), apiVersion(apiVersion)
    {
        ClearLastError(this);
    }

    // Constructor for creating partial env.
    explicit JSVM_Env__(v8::Isolate* isolate, int32_t apiVersion);

    int32_t GetVersion()
    {
        return apiVersion;
    }

    using Callback = std::function<void(JSVM_Env)>;

    inline void RequestInterrupt(Callback cb)
    {
        {
            const std::lock_guard<std::mutex> lock(messageQueueMutex);
            messageQueue.emplace_back(std::move(cb));
        }
        isolate->RequestInterrupt(
            [](v8::Isolate* isolate, void* data) { static_cast<JSVM_Env__*>(data)->RunAndClearInterrupts(); }, this);
    }

    void RunAndClearInterrupts();

    jsvm::InspectorAgent* GetInspectorAgent()
    {
        return inspectorAgent;
    }

    v8::Platform* platform();

    inline v8::Local<v8::Context> context() const
    {
        return v8impl::PersistentToLocal::Strong(contextPersistent);
    }

    bool CanCallIntoJS() const
    {
        return true;
    }

    static inline void HandleThrow(JSVM_Env env, v8::Local<v8::Value> value)
    {
        if (env->IsTerminatedOrTerminating()) {
            return;
        }
        env->isolate->ThrowException(value);
    }

    // i.e. whether v8 exited or is about to exit
    inline bool IsTerminatedOrTerminating()
    {
        return this->isolate->IsExecutionTerminating() || !CanCallIntoJS();
    }

    // v8 uses a special exception to indicate termination, the
    // `handle_exception` callback should identify such case using
    // IsTerminatedOrTerminating() before actually handle the exception
    template<typename T, typename U = decltype(HandleThrow)>
    inline void CallIntoModule(T&& call, U&& handle_exception = HandleThrow)
    {
        int openHandleScopesBefore = openHandleScopes;
        int openCallbackScopesBefore = openCallbackScopes;
        ClearLastError(this);
        call(this);
        CHECK_EQ(openHandleScopes, openHandleScopesBefore);
        CHECK_EQ(openCallbackScopes, openCallbackScopesBefore);
        if (!lastException.IsEmpty()) {
            handle_exception(this, lastException.Get(this->isolate));
            lastException.Reset();
        }
    }

    // Call finalizer immediately.
    void CallFinalizer(JSVM_Finalize cb, void* data, void* hint)
    {
        v8::HandleScope handleScope(isolate);
        CallIntoModule([&](JSVM_Env env) { cb(env, data, hint); });
    }

    void DeleteMe();

    void CheckGCAccess()
    {
        if (inGcFinalizer) {
            jsvm::OnFatalError(nullptr, "Finalizer is calling a function that may affect GC state.\n"
                                        "The finalizers are run directly from GC and must not affect GC "
                                        "state.\n"
                                        "Use `node_api_post_finalizer` from inside of the finalizer to work "
                                        "around this issue.\n"
                                        "It schedules the call as a new task in the event loop.");
        }
    }

    template<typename T>
    JSVM_Script_Data__* NewJsvmData(T srcPtr, JSVM_Script_Data__::DataType type = JSVM_Script_Data__::kJsvmScript)
    {
        if (dataStack.size() == 0 || openHandleScopes != dataStack.back().first) {
            dataStack.emplace_back(openHandleScopes, std::list<JSVM_Script_Data__*>());
        }
        auto newData = new JSVM_Script_Data__(srcPtr, false, type);
        dataStack.back().second.push_back(newData);
        return newData;
    }

    void ReleaseJsvmData()
    {
        if (dataStack.size() == 0 || openHandleScopes != dataStack.back().first) {
            return;
        }
        for (auto data : dataStack.back().second) {
            if (!data->isGlobal) {
                delete data;
            }
        }
        dataStack.pop_back();
    }

    void RetainJsvmData(JSVM_Script_Data__* data)
    {
        for (auto iter = dataStack.rbegin(); iter != dataStack.rend(); ++iter) {
            auto& current = iter->second;
            auto globalData = std::find(current.begin(), current.end(), data);
            if (globalData != current.end()) {
                current.erase(globalData);
                break;
            }
        }
    }

    void CreateScopeTracker()
    {
        scopeTracker = new jsvm::ScopeLifecycleTracker();
    }

    jsvm::ScopeLifecycleTracker* GetScopeTracker()
    {
        if (scopeTracker == nullptr) {
            CreateScopeTracker();
        }
        return scopeTracker;
    }

    // Shortcut for context()->GetIsolate()
    v8::Isolate* const isolate;
    v8impl::Persistent<v8::Context> contextPersistent;

    // Error info and execption
    v8impl::Persistent<v8::Value> lastException;

    // We store references in two different lists, depending on whether they have
    // `JSVM_Finalizer` callbacks, because we must first finalize the ones that
    // have such a callback. See `~JSVM_Env__()` above for details.
    v8impl::RefList userReferenceList;
    v8impl::RefList finalizerList;

    JSVM_ExtendedErrorInfo lastError;

    // Store v8::Data
    std::vector<std::pair<int, std::list<JSVM_Script_Data__*>>> dataStack;

    // Store external instance data
    void* instanceData = nullptr;

    // Store v8::Locker
    v8::Locker* locker = nullptr;

    using ScopeMemoryManager = MemoryChunkList<
        jsvm::MaxSize<v8impl::EscapableHandleScopeWrapper, v8impl::HandleScopeWrapper, v8::Context::Scope>()>;
    ScopeMemoryManager scopeMemoryManager;

    int32_t apiVersion;

    int openHandleScopes = 0;
    int openCallbackScopes = 0;
    bool inGcFinalizer = false;
    uint32_t debugFlags = 0;

private:
    // Used for inspector
    jsvm::InspectorAgent* inspectorAgent;
    std::mutex messageQueueMutex;
    std::vector<Callback> messageQueue;
    // Used for scopeInfo
    jsvm::ScopeLifecycleTracker* scopeTracker = nullptr;

protected:
    // Should not be deleted directly. Delete with `JSVM_Env__::DeleteMe()`
    // instead.
    ~JSVM_Env__() = default;
};

inline JSVM_Status ClearLastError(JSVM_Env env)
{
    env->lastError.errorCode = JSVM_OK;
    env->lastError.engineErrorCode = 0;
    env->lastError.engineReserved = nullptr;
    env->lastError.errorMessage = nullptr;
    return JSVM_OK;
}

#endif