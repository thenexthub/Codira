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

#ifndef GUARD_OBFUSCATOR_ELEMENT_OBFUSCATOR_H
#define GUARD_OBFUSCATOR_ELEMENT_OBFUSCATOR_H

#include "abckit_wrapper/file_view.h"

#include "name_mapping/name_mapping_manager.h"

namespace ark::guard {

/**
 * @brief ElementObfuscator
 */
class ElementObfuscator final : public abckit_wrapper::ModuleVisitor, public abckit_wrapper::ChildVisitor {
public:
    /**
     * @brief constructor, initializes nameMappingManager
     * @param nameMappingManager nameMappingManager
     */
    explicit ElementObfuscator(NameMappingManager &nameMappingManager) : nameMappingManager_(nameMappingManager) {}

    /**
     * @brief Execute obfuscate
     * @param fileView abc fileView
     * @return true: success, false: failed
     */
    bool Execute(abckit_wrapper::FileView &fileView);

private:
    enum class ObjectType {
        OBJECT,
        MODULE,
        NAMESPACE,
        METHOD,
        FIELD,
        CLASS,
        ANNOTATION_INTERFACE,
    };

    bool Visit(abckit_wrapper::Module *module) override;

    bool VisitNamespace(abckit_wrapper::Namespace *ns) override;

    bool VisitMethod(abckit_wrapper::Method *method) override;

    bool VisitField(abckit_wrapper::Field *field) override;

    bool VisitClass(abckit_wrapper::Class *clazz) override;

    bool VisitAnnotationInterface(abckit_wrapper::AnnotationInterface *ai) override;

    bool VisitMember(abckit_wrapper::Member *member);

    bool VisitObject(abckit_wrapper::Object *object, bool visitChild = true, ObjectType type = ObjectType::OBJECT);

    bool ObfModuleWithKeptPath(abckit_wrapper::Object *module, const std::string &path);

    NameMappingManager &nameMappingManager_;
};
}  // namespace ark::guard

#endif  // GUARD_OBFUSCATOR_ELEMENT_OBFUSCATOR_H
