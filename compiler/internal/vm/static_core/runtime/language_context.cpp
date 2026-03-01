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
#include "runtime/include/language_context.h"

#include "libarkbase/macros.h"
#include "runtime/core/core_itable_builder.h"
#include "runtime/core/core_vm.h"
#include "runtime/core/core_vtable_builder.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/language_config.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/stack_walker.h"
#include "runtime/include/thread.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/vm_handle.h"
#include "runtime/tooling/pt_default_lang_extension.h"

namespace ark {
std::pair<Method *, uint32_t> LanguageContextBase::GetCatchMethodAndOffset(Method *method, ManagedThread *thread) const
{
    uint32_t catchOffset = 0;
    Method *catchMethod = method;
    auto stack = StackWalker::Create(thread);
    while (stack.HasFrame()) {
        catchMethod = stack.GetMethod();
        if (catchMethod->GetPandaFile() == nullptr) {
            stack.NextFrame();
            continue;
        }
        if (stack.IsCFrame()) {
            stack.NextFrame();
            continue;
        }
        catchOffset = catchMethod->FindCatchBlock(thread->GetException()->ClassAddr<Class>(), stack.GetBytecodePc());
        if (catchOffset != panda_file::INVALID_OFFSET) {
            break;
        }
        stack.NextFrame();
    }

    return std::make_pair(catchMethod, catchOffset);
}

std::unique_ptr<ClassLinkerExtension> LanguageContextBase::CreateClassLinkerExtension() const
{
    return nullptr;
}

std::unique_ptr<tooling::PtLangExt> LanguageContextBase::CreatePtLangExt() const
{
    std::unique_ptr<tooling::PtLangExt> result;

    if (GetLanguageType() == LangTypeT::LANG_TYPE_STATIC) {
        result = std::make_unique<tooling::PtStaticDefaultExtension>(GetLanguage());
    } else {
        result = std::make_unique<tooling::PtDynamicDefaultExtension>();
    }
    return result;
}

void LanguageContextBase::ThrowException([[maybe_unused]] ManagedThread *thread,
                                         [[maybe_unused]] const uint8_t *mutf8Name,
                                         [[maybe_unused]] const uint8_t *mutf8Msg) const
{
}

void LanguageContextBase::SetExceptionToVReg(
    // CC-OFFNXT(G.FMT.06) false positive
    [[maybe_unused]] interpreter::AccVRegister &vreg,  // NOLINTNEXTLINE(google-runtime-references)
    [[maybe_unused]] ObjectHeader *obj) const
{
}

void LanguageContextBase::WrapClassInitializerException(ClassLinker *classLinker, ManagedThread *thread) const
{
    ASSERT(thread->HasPendingException());

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*thread->GetException()->ClassAddr<Class>());

    auto *errorClass = classLinker->GetExtension(ctx)->GetClass(ctx.GetErrorClassDescriptor(), false);
    ASSERT(errorClass != nullptr);

    auto *cause = thread->GetException();
    if (cause->IsInstanceOf(errorClass)) {
        return;
    }

    ark::ThrowException(ctx, thread, ctx.GetExceptionInInitializerErrorDescriptor(), nullptr);
}

}  // namespace ark
