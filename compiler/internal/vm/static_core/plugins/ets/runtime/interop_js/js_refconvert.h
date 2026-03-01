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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_REFCONVERT_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_REFCONVERT_H_

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "libarkbase/macros.h"
#include <node_api.h>
#include <unordered_map>

namespace ark::ets::interop::js {

class InteropCtx;
class JSRefConvertCache;

// Forward declarations to avoid cyclic deps.
inline JSRefConvertCache *RefConvertCacheFromInteropCtx(InteropCtx *ctx);
inline napi_env JSEnvFromInteropCtx(InteropCtx *ctx);

// Conversion interface for some ark::Class objects
class JSRefConvert {
public:
    // Convert ets->js, returns nullptr if failed, throws JS exceptions
    napi_value Wrap(InteropCtx *ctx, EtsObject *obj)
    {
        ASSERT(obj != nullptr);
        return (this->*(this->wrap_))(ctx, obj);
    }

    // Convert js->ets, returns nullopt if failed, throws ETS/JS exceptions
    EtsObject *Unwrap(InteropCtx *ctx, napi_value jsValue)
    {
        ASSERT(!IsUndefined<true>(JSEnvFromInteropCtx(ctx), jsValue));
        return (this->*(this->unwrap_))(ctx, jsValue);
    }

    JSRefConvert() = delete;
    NO_COPY_SEMANTIC(JSRefConvert);
    NO_MOVE_SEMANTIC(JSRefConvert);
    virtual ~JSRefConvert() = default;

    template <typename D, typename = std::enable_if_t<std::is_base_of_v<JSRefConvert, D>>>
    static D *Cast(JSRefConvert *base)
    {
        ASSERT(base->wrap_ == &D::WrapImpl && base->unwrap_ == &D::UnwrapImpl);
        return static_cast<D *>(base);
    }

protected:
    template <typename D>
    explicit JSRefConvert(D * /*unused*/)
        : wrap_(static_cast<WrapT>(&D::WrapImpl)), unwrap_(static_cast<UnwrapT>(&D::UnwrapImpl))
    {
    }

private:
    using WrapT = decltype(&JSRefConvert::Wrap);
    using UnwrapT = decltype(&JSRefConvert::Unwrap);

    const WrapT wrap_;
    const UnwrapT unwrap_;
};

// Fast cache to find convertor for some ark::Class
class JSRefConvertCache {
public:
    JSRefConvert *Lookup(Class *klass)
    {
        auto *entry = GetDirCacheEntry(klass);
        if (LIKELY(entry->klass == klass)) {
            return entry->value;
        }
        auto value = LookupFull(klass);
        *entry = {klass, value};  // Update dircache_
        return value;
    }

    JSRefConvert *Insert(Class *klass, std::unique_ptr<JSRefConvert> value)
    {
        ASSERT(value != nullptr);
        ASSERT(klass->IsInitialized());
        auto ownedValue = value.get();
        auto [it, inserted] = cache_.insert_or_assign(klass, std::move(value));
        ASSERT(inserted);
        (void)inserted;
        *GetDirCacheEntry(klass) = {klass, ownedValue};
        return ownedValue;
    }

    JSRefConvertCache() : dircache_(new DirCachePair[DIRCACHE_SZ]) {}
    ~JSRefConvertCache() = default;
    NO_COPY_SEMANTIC(JSRefConvertCache);
    NO_MOVE_SEMANTIC(JSRefConvertCache);

private:
    __attribute__((noinline)) JSRefConvert *LookupFull(Class *klass)
    {
        auto it = cache_.find(klass);
        if (UNLIKELY(it == cache_.end())) {
            return nullptr;
        }
        return it->second.get();
    }

    struct DirCachePair {
        Class *klass {};
        JSRefConvert *value {};
    };

    static constexpr uint32_t DIRCACHE_SZ = 1024;

    DirCachePair *GetDirCacheEntry(Class *klass)
    {
        static_assert(helpers::math::IsPowerOfTwo(DIRCACHE_SZ));
        auto hash = helpers::math::PowerOfTwoTableSlot<uint32_t>(ToUintPtr(klass), DIRCACHE_SZ,
                                                                 GetLogAlignment(alignof(Class)));
        return &dircache_[hash];
    }

    std::unique_ptr<DirCachePair[]> dircache_;  // NOLINT(modernize-avoid-c-arrays)
    std::unordered_map<Class *, std::unique_ptr<JSRefConvert>> cache_;
};

void RegisterBuiltinJSRefConvertors(InteropCtx *ctx);

// Try to create JSRefConvert for nonexisting cache entry
template <bool ALLOW_INIT = false>
extern JSRefConvert *JSRefConvertCreate(InteropCtx *ctx, Class *klass);

// Find or create JSRefConvert for some Class
// NOTE(vpukhov): <ALLOW_INIT = false> should never throw?
template <bool ALLOW_INIT = false>
inline JSRefConvert *JSRefConvertResolve(InteropCtx *ctx, Class *klass)
{
    JSRefConvertCache *cache = RefConvertCacheFromInteropCtx(ctx);
    auto conv = cache->Lookup(klass);
    if (LIKELY(conv != nullptr)) {
        return conv;
    }
    return JSRefConvertCreate<ALLOW_INIT>(ctx, klass);
}

template <bool ALLOW_INIT = false>
// CC-OFFNXT(G.FUD.06) perf critical
inline bool CheckClassInitialized(Class *klass)
{
    ASSERT(klass != nullptr);
    if constexpr (ALLOW_INIT) {
        if (UNLIKELY(!klass->IsInitialized())) {
            auto coro = EtsCoroutine::GetCurrent();
            ASSERT(coro != nullptr);
            auto classLinker = coro->GetPandaVM()->GetClassLinker();
            if (!classLinker->InitializeClass(coro, EtsClass::FromRuntimeClass(klass))) {
                INTEROP_LOG(ERROR) << "Class " << klass->GetDescriptor() << " cannot be initialized";
                return false;
            }
        }
    } else {
        INTEROP_FATAL_IF(!klass->IsInitialized());
    }
    return true;
}

inline bool IsStdClass(Class *klass)
{
    const char *desc = utf::Mutf8AsCString(klass->GetDescriptor());
    return strstr(desc, "Lstd/") == desc || strstr(desc, "Lescompat/") == desc;
}

inline bool IsStdClass(EtsClass *klass)
{
    return IsStdClass(klass->GetRuntimeClass());
}

inline bool IsSubClassOfError(EtsClass *klass)
{
    return klass->IsSubClass(PlatformTypes()->escompatError);
}

}  // namespace ark::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_REFCONVERT_H_
