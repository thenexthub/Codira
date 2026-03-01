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

#include "plugins/ets/runtime/ets_annotation.h"
#include "plugins/ets/runtime/ets_class_linker.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "types/ets_field.h"

namespace ark::ets {

EtsClassLinker::EtsClassLinker(ClassLinker *classLinker) : classLinker_(classLinker) {}

/*static*/
Expected<PandaUniquePtr<EtsClassLinker>, PandaString> EtsClassLinker::Create(ClassLinker *classLinker)
{
    PandaUniquePtr<EtsClassLinker> etsClassLinker = MakePandaUnique<EtsClassLinker>(classLinker);
    return Expected<PandaUniquePtr<EtsClassLinker>, PandaString>(std::move(etsClassLinker));
}

bool EtsClassLinker::Initialize()
{
    ClassLinkerExtension *ext = classLinker_->GetExtension(panda_file::SourceLang::ETS);
    ext_ = EtsClassLinkerExtension::FromCoreType(ext);
    return true;
}

bool EtsClassLinker::InitializeClass(EtsCoroutine *coroutine, EtsClass *klass)
{
    ASSERT(klass != nullptr);
    return classLinker_->InitializeClass(coroutine, klass->GetRuntimeClass());
}

EtsClass *EtsClassLinker::GetClassRoot(EtsClassRoot root) const
{
    return EtsClass::FromRuntimeClass(ext_->GetClassRoot(static_cast<ark::ClassRoot>(root)));
}

EtsClass *EtsClassLinker::GetClass(const char *name, bool needCopyDescriptor, ClassLinkerContext *classLinkerContext,
                                   ClassLinkerErrorHandler *errorHandler)
{
    const uint8_t *classDescriptor = utf::CStringAsMutf8(name);
    Class *cls = ext_->GetClass(classDescriptor, needCopyDescriptor, classLinkerContext, errorHandler);
    return LIKELY(cls != nullptr) ? EtsClass::FromRuntimeClass(cls) : nullptr;
}

EtsClass *EtsClassLinker::GetClass(const panda_file::File &pf, panda_file::File::EntityId id,
                                   ClassLinkerContext *classLinkerContext, ClassLinkerErrorHandler *errorHandler)
{
    Class *cls = ext_->GetClass(pf, id, classLinkerContext, errorHandler);
    return LIKELY(cls != nullptr) ? EtsClass::FromRuntimeClass(cls) : nullptr;
}

Method *EtsClassLinker::GetMethod(const panda_file::File &pf, panda_file::File::EntityId id,
                                  ClassLinkerContext *classLinkerContext, ClassLinkerErrorHandler *errorHandler)
{
    return classLinker_->GetMethod(pf, id, classLinkerContext, errorHandler);
}

Method *EtsClassLinker::GetAsyncImplMethod(Method *method, EtsCoroutine *coroutine)
{
    ASSERT(method != nullptr);
    panda_file::File::EntityId asyncAnnId = EtsAnnotation::FindAsyncAnnotation(method);
    ASSERT(asyncAnnId.IsValid());
    const panda_file::File &pf = *method->GetPandaFile();
    panda_file::AnnotationDataAccessor ada(pf, asyncAnnId);
    auto implMethodId = ada.GetElement(0).GetScalarValue().Get<panda_file::File::EntityId>();
    auto *ctx = method->GetClass()->GetLoadContext();
    Method *result = GetMethod(pf, implMethodId, ctx);
    if (result == nullptr) {
        panda_file::MethodDataAccessor mda(pf, implMethodId);
        PandaStringStream out;
        out << "Cannot resolve async method " << mda.GetFullName();
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::LINKER_UNRESOLVED_METHOD_ERROR, out.str());
    }
    return result;
}

}  // namespace ark::ets
