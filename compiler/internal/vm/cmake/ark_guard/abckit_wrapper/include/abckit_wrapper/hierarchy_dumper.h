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

#ifndef ABCKIT_WRAPPER_HIERARCHY_DUMPER_H
#define ABCKIT_WRAPPER_HIERARCHY_DUMPER_H

#include <fstream>
#include "file_view.h"

namespace abckit_wrapper {

/**
 * @brief HierarchyDumper
 * dump fileView hierarchy
 */
class HierarchyDumper final : public ModuleVisitor, public ChildVisitor, public AnnotationVisitor {
public:
    /**
     * @brief HierarchyDumper Constructor
     * @param fileView abc fileView
     * @param path dump file path
     */
    explicit HierarchyDumper(FileView &fileView, const std::string_view &path)
        : fileView_(fileView), ofstream_(path.data())
    {
    }

    /**
     * Dump fileView hierarchy to file
     * @return AbckitWrapperErrorCode
     */
    AbckitWrapperErrorCode Dump();

private:
    static std::string GetIndent(Object *object);

    bool Visit(Module *module) override;

    bool VisitNamespace(Namespace *ns) override;

    bool VisitMethod(Method *method) override;

    bool VisitField(Field *field) override;

    bool VisitClass(Class *clazz) override;

    bool Visit(Annotation *annotation) override;

    FileView &fileView_;
    std::ofstream ofstream_;
};
}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_HIERARCHY_DUMPER_H
