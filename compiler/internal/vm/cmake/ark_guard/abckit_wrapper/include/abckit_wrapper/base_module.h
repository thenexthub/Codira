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

#ifndef ABCKIT_WRAPPER_BASE_MODULE_H
#define ABCKIT_WRAPPER_BASE_MODULE_H

#include <variant>
#include "libabckit/cpp/abckit_cpp.h"
#include "object.h"

namespace abckit_wrapper {
/**
 * @brief BaseModule
 */
class BaseModule : public Object, public MultiImplObject<abckit::core::Module, abckit::core::Namespace> {
public:
    /**
     * @brief BaseModule Constructor
     * @param module abckit::core::Module or abckit::core::Namespace
     */
    explicit BaseModule(std::variant<abckit::core::Module, abckit::core::Namespace> module)
        : MultiImplObject(std::move(module))
    {
    }

    AbckitWrapperErrorCode Init() override;

    /**
     * @brief Tells if Module/Namespace is defined in the same binary or externally in another binary.
     * @return Returns `true` if Module/Namespace is defined in another binary and `false` if defined locally.
     */
    bool IsExternal();

    std::string GetName() const override;

protected:
    /**
     * @brief Init for sub object
     * @param object sub object
     */
    virtual void InitForObject(Object *object) = 0;

private:
    AbckitWrapperErrorCode InitNamespaces();

    AbckitWrapperErrorCode InitFunctions();

    AbckitWrapperErrorCode InitModuleFields();

    AbckitWrapperErrorCode InitClasses();

    AbckitWrapperErrorCode InitInterfaces();

    AbckitWrapperErrorCode InitEnums();

    AbckitWrapperErrorCode InitAnnotationInterfaces();
};
}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_BASE_MODULE_H
