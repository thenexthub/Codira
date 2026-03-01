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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_VM_OPTIONS_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_VM_OPTIONS_H_

#include "libarkbase/macros.h"
#include "include/runtime_options.h"

namespace ark::ets {

class EtsVmOptions final {
public:
    explicit EtsVmOptions(bool isVerifyANI) : isVerifyANI_(isVerifyANI) {}
    ~EtsVmOptions() = default;

    bool IsVerifyANI() const
    {
        return isVerifyANI_;
    }

private:
    bool isVerifyANI_;

    NO_COPY_SEMANTIC(EtsVmOptions);
    NO_MOVE_SEMANTIC(EtsVmOptions);
};

inline void SetEtsVmOptions(RuntimeOptions &runtimeOptions, std::unique_ptr<EtsVmOptions> etsVmOptions)
{
    auto etsVmOptionsDeleter = etsVmOptions.get_deleter();
    void *optsData = etsVmOptions.release();
    auto deleter = [etsVmOptionsDeleter](void *data) { etsVmOptionsDeleter(reinterpret_cast<EtsVmOptions *>(data)); };
    runtimeOptions.SetLangExtensionOptions(panda_file::SourceLang::ETS, std::shared_ptr<void>(optsData, deleter));
}

inline const EtsVmOptions *GetEtsVmOptions(const RuntimeOptions &runtimeOptions)
{
    return reinterpret_cast<const EtsVmOptions *>(runtimeOptions.GetLangExtensionOptions(panda_file::SourceLang::ETS));
}

}  // namespace ark::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_ETS_VM_OPTIONS_H_
