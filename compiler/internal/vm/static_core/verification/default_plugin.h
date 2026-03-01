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

#ifndef PANDA_VERIFICATION_DEFAULT_PLUGIN_H__
#define PANDA_VERIFICATION_DEFAULT_PLUGIN_H__

#include "abs_int_inl_compat_checks.h"
#include "verification/plugins.h"

namespace ark::verifier::plugin {

class DefaultPlugin final : public Plugin {
public:
    ManagedThread *CreateManagedThread() const override;
    void DestroyManagedThread(ManagedThread *thr) const override;
    void TypeSystemSetup(TypeSystem *types) const override;

    CheckResult const &CheckFieldAccessViolation([[maybe_unused]] Field const *field,
                                                 [[maybe_unused]] Method const *from,
                                                 [[maybe_unused]] TypeSystem *types) const override
    {
        return CheckResult::ok;
    }

    CheckResult const &CheckMethodAccessViolation([[maybe_unused]] Method const *method,
                                                  [[maybe_unused]] Method const *from,
                                                  [[maybe_unused]] TypeSystem *types) const override
    {
        return CheckResult::ok;
    }

    CheckResult const &CheckClassAccessViolation([[maybe_unused]] Class const *method,
                                                 [[maybe_unused]] Method const *from,
                                                 [[maybe_unused]] TypeSystem *types) const override
    {
        return CheckResult::ok;
    }

    CheckResult const &CheckClass([[maybe_unused]] Class const *klass) const override
    {
        return CheckResult::ok;
    }

    Type NormalizeType(Type type, TypeSystem *types) const override;
};

}  // namespace ark::verifier::plugin

#endif
