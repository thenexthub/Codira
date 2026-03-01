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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_CLASS_LINKER_CONTEXT_H
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_CLASS_LINKER_CONTEXT_H

#include <libarkfile/include/source_lang_enum.h>

#include "libarkbase/macros.h"
#include "libarkbase/os/mutex.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_runtime_linker.h"
#include "runtime/class_linker_context.h"
#include "runtime/include/class_linker.h"

namespace ark::ets {

class EtsClassLinkerContext final : public ClassLinkerContext {
public:
    explicit EtsClassLinkerContext(EtsRuntimeLinker *runtimeLinker) : ClassLinkerContext(panda_file::SourceLang::ETS)
    {
        auto *objectStorage = PandaEtsVM::GetCurrent()->GetGlobalObjectStorage();
        // Store weak reference in order not to prevent collection of managed RuntimeLinker object.
        refToLinker_ = objectStorage->Add(runtimeLinker->GetCoreType(), mem::Reference::ObjectType::WEAK);
        ASSERT(refToLinker_ != nullptr);
    }

    NO_COPY_SEMANTIC(EtsClassLinkerContext);
    NO_MOVE_SEMANTIC(EtsClassLinkerContext);

    ~EtsClassLinkerContext() override;

    static EtsClassLinkerContext *FromCoreType(ClassLinkerContext *ctx);

    /// @brief Load class according to corresponding RuntimeLinker.
    Class *LoadClass(const uint8_t *descriptor, bool needCopyDescriptor,
                     ClassLinkerErrorHandler *errorHandler) override;

    void EnumeratePandaFiles(const std::function<bool(const panda_file::File &)> &cb) const override;

    void EnumeratePandaFilesInChain(const std::function<bool(const panda_file::File &)> &cb) const override;

    PandaVector<std::string_view> GetPandaFilePaths() const override
    {
        PandaVector<std::string_view> filePaths;
        EnumeratePandaFiles([&filePaths](const auto &pf) {
            filePaths.emplace_back(pf.GetFilename());
            return true;
        });
        return filePaths;
    }

    /// @brief Find and load class within panda files of this context.
    Class *FindAndLoadClass(const uint8_t *descriptor, ClassLinkerErrorHandler *errorHandler);

    EtsRuntimeLinker *GetRuntimeLinker() const
    {
        auto *linker = PandaEtsVM::GetCurrent()->GetGlobalObjectStorage()->Get(refToLinker_);
        ASSERT(linker != nullptr);
        return EtsRuntimeLinker::FromCoreType(linker);
    }

    os::memory::RecursiveMutex &GetAbcFilesMutex()
    {
        return abcFilesMutex_;
    }

private:
    /**
     * @brief Try to find and load class in AbcRuntimeLinker's chain without invoking managed implementations.
     *
     * @param descriptor of the searched class.
     * @param errorHandler to be used during class loading, including CLASS_NOT_FOUND error.
     * @param klass pointer for storing result, must contain nullptr.
     *
     * @returns true if linkers' chain was traversed successfully, false otherwise.
     */
    bool TryLoadingClassFromNative(const uint8_t *descriptor, ClassLinkerErrorHandler *errorHandler, Class **klass);

    void EnumeratePandaFilesImpl(const std::function<bool(const panda_file::File &)> &cb) const;

private:
    os::memory::RecursiveMutex abcFilesMutex_;
    mem::Reference *refToLinker_ {nullptr};
};

}  // namespace ark::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_ETS_CLASS_LINKER_CONTEXT_H
