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

#include "plugins/ets/runtime/interop_js/js_refconvert.h"

#include "libarkbase/utils/utf.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_class_wrapper.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/js_refconvert_array.h"
#include "plugins/ets/runtime/interop_js/js_refconvert_function.h"
#include "plugins/ets/runtime/interop_js/js_refconvert_record.h"
#include "plugins/ets/runtime/interop_js/js_refconvert_tuple.h"
#include "plugins/ets/runtime/interop_js/js_refconvert_union.h"

namespace ark::ets::interop::js {

static bool IsRecord(Class *klass)
{
    return PlatformTypes()->coreRecord->GetRuntimeClass()->IsAssignableFrom(klass);
}

static bool IsTupleClass(Class *klass)
{
    return klass->Implements(PlatformTypes()->coreTuple->GetRuntimeClass());
}

static bool IsTupleNClass(Class *klass)
{
    return PlatformTypes()->coreTupleN->GetRuntimeClass()->IsAssignableFrom(klass);
}

static std::unique_ptr<JSRefConvert> JSRefConvertCreateImpl(InteropCtx *ctx, Class *klass)
{
    INTEROP_FATAL_IF(klass->IsClassClass());

    if (klass->IsArrayClass()) {
        auto type = klass->GetComponentType()->GetType().GetId();
        if (type != panda_file::Type::TypeId::REFERENCE) {
            ctx->Fatal(std::string("Unhandled primitive array: ") + utf::Mutf8AsCString(klass->GetDescriptor()));
        }
        return std::make_unique<JSRefConvertReftypeArray>(klass);
    }

    if (js_proxy::JSProxy::IsProxyClass(klass)) {
        return ets_proxy::EtsClassWrapper::CreateJSRefConvertJSProxy(ctx, klass);
    }

    if (EtsClass::FromRuntimeClass(klass)->IsFunction()) {
        return std::make_unique<JSRefConvertFunction>(klass);
    }

    if (IsRecord(klass)) {
        return std::make_unique<JSRefConvertRecord>(ctx);
    }

    if (IsTupleClass(klass)) {
        // handle Tuple0, Tuple1, ..., Tuple16
        if (!IsTupleNClass(klass)) {
            return std::make_unique<JSRefConvertTuple<false>>(ctx, klass);
        }

        // handle TupleN
        return std::make_unique<JSRefConvertTuple<true>>(ctx, klass);
    }

    if (klass->IsUnionClass()) {
        return std::make_unique<JSRefConvertUnion>(klass);
    }

    if (klass->IsInterface()) {
        return ets_proxy::EtsClassWrapper::CreateJSRefConvertEtsInterface(ctx, klass);
    }

    return ets_proxy::EtsClassWrapper::CreateJSRefConvertEtsProxy(ctx, klass);
}

template <bool ALLOW_INIT>
JSRefConvert *JSRefConvertCreate(InteropCtx *ctx, Class *klass)
{
    INTEROP_TRACE();
    auto coro = EtsCoroutine::GetCurrent();
    if (!CheckClassInitialized<ALLOW_INIT>(klass)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    auto conv = JSRefConvertCreateImpl(ctx, klass);
    if (UNLIKELY(conv == nullptr)) {
        if (!coro->HasPendingException()) {
            ctx->ThrowETSError(coro, std::string("Seamless conversion for class ") +
                                         utf::Mutf8AsCString(klass->GetDescriptor()) +
                                         std::string(" is not supported"));
        }
        return nullptr;
    }
    return ctx->GetRefConvertCache()->Insert(klass, std::move(conv));
}

template JSRefConvert *JSRefConvertCreate<false>(InteropCtx *ctx, Class *klass);
template JSRefConvert *JSRefConvertCreate<true>(InteropCtx *ctx, Class *klass);

}  // namespace ark::ets::interop::js
