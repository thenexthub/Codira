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
#ifndef PANDA_RUNTIME_CLASS_LINKER_INL_H_
#define PANDA_RUNTIME_CLASS_LINKER_INL_H_

#include "libarkfile/panda_cache.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/include/mtmanaged_thread.h"

namespace ark {

// CC-OFFNXT(G.FUD.06) perf critical
inline Class *ClassLinker::GetClass(const Method &caller, panda_file::File::EntityId id,
                                    ClassLinkerErrorHandler *errorHandler /* = nullptr */)
{
    ASSERT(!MTManagedThread::ThreadIsMTManagedThread(Thread::GetCurrent()) ||
           !PandaVM::GetCurrent()->GetGC()->IsGCRunning() || PandaVM::GetCurrent()->GetMutatorLock()->HasLock());

    Class *klass = caller.GetPandaFile()->GetPandaCache()->GetClassFromCache(id);
    if (klass != nullptr) {
        return klass;
    }
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(caller);
    auto *ext = GetExtension(ctx);
    ASSERT(ext != nullptr);
    klass = ext->GetClass(*caller.GetPandaFile(), id, caller.GetClass()->GetLoadContext(),
                          (errorHandler == nullptr) ? ext->GetErrorHandler() : errorHandler);
    if (LIKELY(klass != nullptr)) {
        caller.GetPandaFile()->GetPandaCache()->SetClassCache(id, klass);
    }
    return klass;
}

inline void ClassLinker::AddClassRoot(ClassRoot root, Class *klass)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*klass);
    auto *ext = GetExtension(ctx);
    ASSERT(ext != nullptr);
    ASSERT(klass != nullptr);
    ext->SetClassRoot(root, klass);

    RemoveCreatedClassInExtension(klass);
}

// CC-OFFNXT(G.FUD.06) perf critical
inline Class *ClassLinker::GetLoadedClass(const panda_file::File &pf, panda_file::File::EntityId id,
                                          ClassLinkerContext *context)
{
    ASSERT(context != nullptr);

    Class *cls = pf.GetPandaCache()->GetClassFromCache(id);
    if (LIKELY(cls != nullptr)) {
        return cls;
    }

    cls = FindLoadedClass(pf.GetStringData(id).data, context);
    if (LIKELY(cls != nullptr)) {
        pf.GetPandaCache()->SetClassCache(id, cls);
        return cls;
    }
    return nullptr;
}

}  // namespace ark

#endif  // PANDA_RUNTIME_CLASS_LINKER_INL_H_
