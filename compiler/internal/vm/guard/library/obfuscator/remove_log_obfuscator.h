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

#ifndef GUARD_REMOVE_LOG_OBFUSCATOR_H
#define GUARD_REMOVE_LOG_OBFUSCATOR_H

#include "abckit_wrapper/file_view.h"

namespace ark::guard {
/**
 * @brief RemoveLogObfuscator
 */
class RemoveLogObfuscator : public abckit_wrapper::ModuleVisitor, public abckit_wrapper::ChildVisitor {
public:
    /**
     * @brief constructor
     */
    RemoveLogObfuscator() = default;

    /**
     * @brief Execute obfuscate
     * @param fileView abc fileView
     * @return true: success, false: failed
     */
    bool Execute(abckit_wrapper::FileView &fileView);

private:
    bool Visit(abckit_wrapper::Module *module) override;

    bool VisitNamespace(abckit_wrapper::Namespace *ns) override;

    bool VisitClass(abckit_wrapper::Class *clazz) override;

    bool VisitField(abckit_wrapper::Field *field) override;

    bool VisitAnnotationInterface(abckit_wrapper::AnnotationInterface *ai) override;

    bool VisitMethod(abckit_wrapper::Method *method) override;
};
}  // namespace ark::guard

#endif  // GUARD_REMOVE_LOG_OBFUSCATOR_H
