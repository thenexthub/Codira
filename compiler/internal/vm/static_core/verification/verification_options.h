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

#ifndef PANDA_VERIFICATION_OPTIONS_H_
#define PANDA_VERIFICATION_OPTIONS_H_

#include "verification/public.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "verification/config/options/method_options_config.h"

#include <string>
#include <unordered_map>

namespace ark::verifier {

struct VerificationOptions {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    std::string configFile = "default";
    VerificationMode mode = VerificationMode::DISABLED;
    struct {
        bool status = false;
    } show;
    bool verifyRuntimeLibraries = false;
    bool syncOnClassInitialization = false;
    size_t verificationThreads = 1;
    struct {
        std::string file;
        bool updateOnExit = false;
    } cache;
    struct {
        struct {
            bool regChanges = false;
            bool context = false;
            bool typeSystem = false;
        } show;
        struct {
            bool undefinedClass = false;
            bool undefinedMethod = false;
            bool undefinedField = false;
            bool undefinedType = false;
            bool undefinedString = false;
            bool methodAccessViolation = false;
            bool errorInExceptionHandler = false;
            bool permanentRuntimeException = false;
            bool fieldAccessViolation = false;
            bool wrongSubclassingInMethodArgs = false;
        } allow;
        MethodOptionsConfig *methodOptions = nullptr;
        MethodOptionsConfig &GetMethodOptions()
        {
            return *methodOptions;
        }
        const MethodOptionsConfig &GetMethodOptions() const
        {
            return *methodOptions;
        }
    } debug;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    void Initialize();
    void Destroy();

    bool IsEnabled() const
    {
        return mode != VerificationMode::DISABLED;
    }

    bool IsOnlyVerify() const
    {
        return mode == VerificationMode::AHEAD_OF_TIME || mode == VerificationMode::DEBUG;
    }
};

}  // namespace ark::verifier

#endif  // !PANDA_VERIFICATION_OPTIONS_H_
