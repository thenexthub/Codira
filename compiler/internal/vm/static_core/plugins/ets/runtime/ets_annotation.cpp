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

#include "libarkfile/annotation_data_accessor.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "runtime/include/method.h"

namespace ark::ets {

/*static*/
panda_file::File::EntityId EtsAnnotation::FindAsyncAnnotation(const Method *method)
{
    return FindAnnotation(method, panda_file_items::class_descriptors::ASYNC);
}

/*static*/
panda_file::File::EntityId EtsAnnotation::OptionalParameters(const Method *method)
{
    return FindAnnotation(method, panda_file_items::class_descriptors::OPTIONAL_PARAMETERS_ANNOTATION);
}

/*static*/
panda_file::File::EntityId EtsAnnotation::FindAnnotation(const Method *method, const std::string_view &sign)
{
    panda_file::File::EntityId foundId;
    const panda_file::File &pf = *method->GetPandaFile();
    panda_file::MethodDataAccessor mda(pf, method->GetFileId());
    mda.EnumerateAnnotations([&pf, &foundId, sign](panda_file::File::EntityId annId) {
        panda_file::AnnotationDataAccessor ada(pf, annId);
        const char *className = utf::Mutf8AsCString(pf.GetStringData(ada.GetClassId()).data);
        if (className == sign) {
            foundId = annId;
        }
    });
    return foundId;
}

}  // namespace ark::ets
