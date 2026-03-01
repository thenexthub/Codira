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

#include "ets_platform_types.h"
#include "libarkfile/file.h"
#include "include/object_header.h"
#include "intrinsics.h"
#include "libarkbase/os/mutex.h"
#include "plugins/ets/runtime/ets_class_linker_context.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/types/ets_abc_file.h"
#include "plugins/ets/runtime/types/ets_abc_runtime_linker.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_string.h"

namespace ark::ets::intrinsics {

void EtsAbcRuntimeLinkerAddNewAbcFiles(EtsAbcRuntimeLinker *runtimeLinker, ObjectHeader *newAbcFilesArray)
{
    auto *coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope hs(coro);
    EtsHandle newAbcFilesHandle(coro, EtsTypedObjectArray<EtsAbcFile>::FromCoreType(newAbcFilesArray));
    EtsHandle linkerHandle(coro, runtimeLinker);
    auto *ctx = EtsClassLinkerContext::FromCoreType(linkerHandle->GetClassLinkerContext());

    os::memory::LockHolder lock(ctx->GetAbcFilesMutex());
    EtsHandle currentAbcFilesHandle(coro, linkerHandle->GetAbcFiles());
    auto currentLength = currentAbcFilesHandle->GetLength();
    auto resultLength = newAbcFilesHandle->GetLength() + currentLength;
    EtsHandle resultAbcFilesHandle(coro, EtsObjectArray::Create(PlatformTypes(coro)->coreAbcFile, resultLength));
    if (UNLIKELY(resultAbcFilesHandle.GetPtr() == nullptr)) {
        ASSERT(coro->HasPendingException());
        return;
    }

    currentAbcFilesHandle->CopyDataTo(resultAbcFilesHandle.GetPtr());
    for (size_t start = currentLength, i = start; i < resultLength; ++i) {
        resultAbcFilesHandle->Set(i, newAbcFilesHandle->Get(i - start));
    }
    linkerHandle->SetAbcFiles(resultAbcFilesHandle.GetPtr());
}

EtsClass *EtsAbcRuntimeLinkerLoadClassFromAbcFiles(EtsAbcRuntimeLinker *runtimeLinker, EtsString *clsName,
                                                   EtsBoolean init)
{
    ark::ets::ClassPublicNameParser parser(clsName->GetMutf8());
    const auto name = parser.Resolve();
    auto *classDescriptor = utf::CStringAsMutf8(name.c_str());
    if (classDescriptor == nullptr) {
        return nullptr;
    }

    auto *coro = EtsCoroutine::GetCurrent();
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    auto *errorHandler = PandaEtsVM::GetCurrent()->GetEtsClassLinkerExtension()->GetErrorHandler();
    auto *ctx = reinterpret_cast<EtsClassLinkerContext *>(runtimeLinker->GetClassLinkerContext());
    os::memory::LockHolder rlock(ctx->GetAbcFilesMutex());

    auto *abcFiles = runtimeLinker->GetAbcFiles();
    for (size_t i = 0, end = abcFiles->GetLength(); i < end; ++i) {
        auto *currentFile = abcFiles->Get(i);
        ASSERT(currentFile != nullptr);
        auto *pf = EtsAbcFile::FromEtsObject(currentFile)->GetPandaFile();
        const auto classId = pf->GetClassId(classDescriptor);
        if (!classId.IsValid() || pf->IsExternal(classId)) {
            continue;
        }

        auto *klass = classLinker->LoadClass(*pf, classId, ctx, errorHandler, true);
        if (UNLIKELY(klass == nullptr)) {
            ASSERT(coro->HasPendingException());
            return nullptr;
        }

        if (UNLIKELY(init != 0 && !klass->IsInitialized())) {
            if (UNLIKELY(!classLinker->InitializeClass(coro, klass))) {
                ASSERT(coro->HasPendingException());
                return nullptr;
            }
        }
        return EtsClass::FromRuntimeClass(klass);
    }
    return nullptr;
}

}  // namespace ark::ets::intrinsics
