/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "runtime/include/language_config.h"
#include "runtime/mem/gc/g1/ref_updater.h"
#include "runtime/mem/object_helpers-inl.h"
#include "runtime/mem/rem_set-inl.h"

namespace ark::mem {

template <class LanguageConfig>
ObjectHeader *BaseRefUpdater<LanguageConfig>::UpdateRefToMovedObject(ObjectHeader *object, ObjectHeader *ref,
                                                                     uint32_t offset) const
{
    return ObjectHelpers<LanguageConfig::LANG_TYPE>::UpdateRefToMovedObject(object, ref, offset);
}

template <class LanguageConfig, bool NEED_LOCK>
void UpdateRemsetRefUpdater<LanguageConfig, NEED_LOCK>::Process(ObjectHeader *object, size_t offset,
                                                                ObjectHeader *ref) const
{
    if (!this->IsSameRegion(object, ref)) {
        RemSet<>::AddRefWithAddr<NEED_LOCK>(object, offset, ref);
    }
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(BaseRefUpdater);
TEMPLATE_CLASS_LANGUAGE_CONFIG_AND_ARGS(UpdateRemsetRefUpdater, true);
TEMPLATE_CLASS_LANGUAGE_CONFIG_AND_ARGS(UpdateRemsetRefUpdater, false);

}  // namespace ark::mem
