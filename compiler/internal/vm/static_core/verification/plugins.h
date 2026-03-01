/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_VERIFICATION_PLUGINS_H
#define PANDA_VERIFICATION_PLUGINS_H

#include "abs_int_inl_compat_checks.h"
#include <libarkfile/include/source_lang_enum.h>

namespace ark::verifier::plugin {

class Plugin {
public:
    virtual ManagedThread *CreateManagedThread() const = 0;
    virtual void DestroyManagedThread(ManagedThread *thr) const = 0;
    virtual void TypeSystemSetup(TypeSystem *types) const = 0;

    virtual CheckResult const &CheckFieldAccessViolation(Field const *field, Method const *from,
                                                         TypeSystem *types) const = 0;

    virtual CheckResult const &CheckMethodAccessViolation(Method const *method, Method const *from,
                                                          TypeSystem *types) const = 0;

    virtual CheckResult const &CheckClassAccessViolation(Class const *klass, Method const *from,
                                                         TypeSystem *types) const = 0;

    virtual Type NormalizeType(Type type, TypeSystem *types) const = 0;

    virtual CheckResult const &CheckClass(Class const *klass) const = 0;
};

PANDA_PUBLIC_API Plugin const *GetLanguagePlugin(panda_file::SourceLang lang);

}  // namespace ark::verifier::plugin

#endif  // PANDA_VERIFICATION_PLUGINS_H
