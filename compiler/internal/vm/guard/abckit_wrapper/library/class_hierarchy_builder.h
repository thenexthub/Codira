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

#ifndef ABCKIT_WRAPPER_CLASS_HIERARCHY_BUILDER_H
#define ABCKIT_WRAPPER_CLASS_HIERARCHY_BUILDER_H

#include "abckit_wrapper/file_view.h"

namespace abckit_wrapper {
/**
 * @brief ClassHierarchyBuilder
 * build local classes cross ref
 */
class ClassHierarchyBuilder final : public ClassVisitor {
public:
    /**
     * @brief ClassHierarchyBuilder
     * @param fileView fileView reference
     */
    explicit ClassHierarchyBuilder(FileView &fileView) : fileView_(fileView) {}

    /**
     * @brief Build class hierarchy
     * 1.superClass
     * 2.interfaces
     * 3.subClasses
     */
    AbckitWrapperErrorCode Build();

private:
    bool Visit(Class *clazz) override;

    bool InitSuperClass(Class *clazz);

    bool InitInterfaces(Class *clazz);

    FileView &fileView_;
    AbckitWrapperErrorCode result_ = ERR_SUCCESS;
};
}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_CLASS_HIERARCHY_BUILDER_H
