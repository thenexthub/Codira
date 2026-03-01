/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTEROP_STACKS_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTEROP_STACKS_H

#include "plugins/ets/runtime/interop_js/interop_common.h"

#include "plugins/ets/runtime/ets_call_stack.h"

namespace ark::ets::interop::js {

class InteropCallStack final : public EtsCallStack {
public:
    InteropCallStack()
    {
        void *pool = ark::os::mem::MapRWAnonymousWithAlignmentRaw(POOL_SIZE, ark::os::mem::GetPageSize());
        if (pool == nullptr) {
            InteropFatal("InteropCallStack alloc failed");
        }
        startAddr_ = reinterpret_cast<Record *>(pool);
        endAddr_ = reinterpret_cast<Record *>(ToUintPtr(pool) + POOL_SIZE);
        curAddr_ = startAddr_;
    }

    ~InteropCallStack()
    {
        ark::os::mem::UnmapRaw(startAddr_, POOL_SIZE);
    }

    template <typename... Args>
    Record *AllocRecord(Args &&...args)
    {
        return new (Alloc()) Record(args...);
    }

    void PopRecord()
    {
        ASSERT(curAddr_ >= startAddr_);
        ASSERT(curAddr_ <= endAddr_);
        if (LIKELY(curAddr_ > startAddr_)) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            (curAddr_--)->~Record();
            return;
        }
        InteropFatal("InteropCallStack is empty");
    }

    Record *Current() override
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return curAddr_ != startAddr_ ? (curAddr_ - 1) : nullptr;
    }

    Span<Record> GetRecords() override
    {
        return {startAddr_, (ToUintPtr(curAddr_) - ToUintPtr(startAddr_)) / sizeof(Record)};
    }

private:
    NO_COPY_SEMANTIC(InteropCallStack);
    NO_MOVE_SEMANTIC(InteropCallStack);

    Record *Alloc()
    {
        ASSERT(curAddr_ >= startAddr_);
        ASSERT(curAddr_ <= endAddr_);
        if (LIKELY(curAddr_ < endAddr_)) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            return curAddr_++;
        }
        InteropFatal("InteropCallStack overflow");
    }

    static constexpr uint32_t MAX_FRAMES = 1024;
    static constexpr size_t POOL_SIZE = MAX_FRAMES * sizeof(Record);

    Record *curAddr_ {};
    Record *startAddr_ {};
    Record *endAddr_ {};
};

class LocalScopesStorage {
public:
    LocalScopesStorage() = default;

    ~LocalScopesStorage()
    {
        ASSERT(localScopesStorage_.empty());
    }

    NO_COPY_SEMANTIC(LocalScopesStorage);
    NO_MOVE_SEMANTIC(LocalScopesStorage);

    void CreateLocalScope(napi_env env, Frame *frame)
    {
        napi_handle_scope scope;
        [[maybe_unused]] auto status = napi_open_handle_scope(env, &scope);
        ASSERT(status == napi_ok);
        localScopesStorage_.emplace_back(frame, scope);
    }

    void DestroyTopLocalScope(napi_env env, [[maybe_unused]] Frame *currFrame)
    {
        ASSERT(!localScopesStorage_.empty());
        auto &[frame, scope] = localScopesStorage_.back();
        ASSERT(currFrame == frame);
        localScopesStorage_.pop_back();
        [[maybe_unused]] auto status = napi_close_handle_scope(env, scope);
        ASSERT(status == napi_ok);
    }

    void DestroyLocalScopeForTopFrame(napi_env env, Frame *currFrame)
    {
        if (localScopesStorage_.empty()) {
            return;
        }
        auto &[frame, scope] = localScopesStorage_.back();
        while (frame == currFrame) {
            localScopesStorage_.pop_back();
            [[maybe_unused]] auto status = napi_close_handle_scope(env, scope);
            ASSERT(status == napi_ok);
            if (localScopesStorage_.empty()) {
                return;
            }
            std::tie(frame, scope) = localScopesStorage_.back();
        }
    }

private:
    std::vector<std::pair<const Frame *, napi_handle_scope>> localScopesStorage_ {};
};

}  // namespace ark::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_STACKS_H
