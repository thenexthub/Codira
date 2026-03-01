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

#ifndef PANDA_VERIFIER_JOB_QUEUE_JOB_H_
#define PANDA_VERIFIER_JOB_QUEUE_JOB_H_

#include <libarkfile/include/source_lang_enum.h>
#include "verification/cflow/cflow_info.h"
#include "verification/verification_options.h"
#include "verification/plugins.h"
#include "verification/public_internal.h"

#include "runtime/include/class_linker.h"
#include "runtime/include/method.h"

#include <cstdint>
#include <functional>
#include <optional>

namespace ark::verifier {
class Job {
public:
    Job(Service *service, Method const *method, const MethodOptions &options)
        : service_ {service},
          classLinker_ {service->classLinker},
          method_ {method},
          lang_ {method->GetClass()->GetSourceLang()},
          langContext_ {plugins::GetLanguageContextBase(method_->GetClass()->GetSourceLang())},
          classLinkerCtx_ {classLinker_->GetExtension(langContext_)->GetBootContext()},
          options_ {options},
          plugin_ {plugin::GetLanguagePlugin(lang_)}
    {
    }

    ~Job() = default;
    NO_MOVE_SEMANTIC(Job);
    NO_COPY_SEMANTIC(Job);

    bool IsFieldPresentForOffset(uint32_t offset) const
    {
        return fields_.count(offset) != 0;
    }

    bool IsMethodPresentForOffset(uint32_t offset) const
    {
        return methods_.count(offset) != 0;
    }

    bool IsTypePresentForOffset(uint32_t offset) const
    {
        return types_.count(offset) != 0;
    }

    Field const *GetCachedField(uint32_t offset) const
    {
        return fields_.at(offset);
    }

    Method const *GetCachedMethod(uint32_t offset) const
    {
        return methods_.at(offset);
    }

    Type GetCachedType(uint32_t offset) const
    {
        return types_.at(offset);
    }

    Service *GetService() const
    {
        return service_;
    }

    Method const *JobMethod() const
    {
        return method_;
    }

    const CflowMethodInfo &JobMethodCflow() const
    {
        return *cflowInfo_;
    }

    const plugin::Plugin *JobPlugin() const
    {
        return plugin_;
    }

    template <typename Handler>
    void ForAllCachedTypes(Handler &&handler) const
    {
        for (const auto &item : types_) {
            handler(item.second);
        }
    }

    template <typename Handler>
    void ForAllCachedMethods(Handler &&handler) const
    {
        for (const auto &item : methods_) {
            handler(item.second);
        }
    }

    template <typename Handler>
    void ForAllCachedFields(Handler &&handler) const
    {
        for (const auto &item : fields_) {
            handler(item.second);
        }
    }

    const auto &Options() const
    {
        return options_;
    }

    bool DoChecks(TypeSystem *types);

    class ErrorHandler : public ClassLinkerErrorHandler {
        void OnError(ClassLinker::Error error, PandaString const &message) override;
    };

private:
    Service *service_;
    ClassLinker *classLinker_;
    Method const *method_;
    panda_file::SourceLang lang_;
    LanguageContext langContext_;
    ClassLinkerContext *classLinkerCtx_;
    const MethodOptions &options_;
    PandaUniquePtr<CflowMethodInfo> cflowInfo_;

    plugin::Plugin const *const plugin_;

    // NOTE(vdyadov): store file_id for double check during verification
    // offset -> cache item
    PandaUnorderedMap<uint32_t, Field const *> fields_;
    PandaUnorderedMap<uint32_t, Method const *> methods_;
    PandaUnorderedMap<uint32_t, Type> types_;

    bool ResolveIdentifiers();

    bool UpdateTypes(TypeSystem *types) const;

    bool Verify(TypeSystem *types) const;

    void AddField(uint32_t offset, Field const *field)
    {
        fields_.emplace(offset, field);
    }

    void AddMethod(uint32_t offset, Method const *method)
    {
        methods_.emplace(offset, method);
    }

    void AddType(uint32_t offset, Type const *type)
    {
        types_.emplace(offset, *type);
    }
};
}  // namespace ark::verifier

#endif  // PANDA_VERIFIER_JOB_QUEUE_JOB_H_
