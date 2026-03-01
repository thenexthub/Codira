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

#ifndef GUARD_OBFUSCATOR_NAME_MARKER_H
#define GUARD_OBFUSCATOR_NAME_MARKER_H

#include <utility>

#include "abckit_wrapper/file_view.h"

#include "configuration/configuration.h"

namespace ark::guard {
/**
 * @brief NameMarker
 */
class NameMarker final : public abckit_wrapper::ModuleVisitor, public abckit_wrapper::ChildVisitor {
public:
    explicit NameMarker(Configuration configuration) : configuration_(std::move(configuration)) {}

    /**
     * @brief mark objects are kept
     * @param fileView abc fileView
     * @return true: success, false: failed
     */
    bool Execute(abckit_wrapper::FileView &fileView);

private:
    bool Visit(abckit_wrapper::Module *module) override;

    bool VisitNamespace(abckit_wrapper::Namespace *ns) override;

    bool VisitMethod(abckit_wrapper::Method *method) override;

    bool VisitField(abckit_wrapper::Field *field) override;

    bool VisitClass(abckit_wrapper::Class *clazz) override;

    bool VisitAnnotationInterface(abckit_wrapper::AnnotationInterface *ai) override;

    std::vector<ClassSpecification> CollectMatchedClassSpecification(const abckit_wrapper::Class *clazz);

    bool MatchClassSpecification(const abckit_wrapper::Class *clazz, const ClassSpecification &spec);

    bool MatchExtends(const abckit_wrapper::Class *clazz, const ClassSpecification &spec);

    bool MatchImplements(const abckit_wrapper::Class *clazz, const ClassSpecification &spec);

    bool MatchMethodSpecification(abckit_wrapper::Method *method, const MemberSpecification &spec);

    bool MatchFieldSpecification(abckit_wrapper::Field *field, const MemberSpecification &spec);

    bool CollectMatchedMethods(const std::unordered_map<std::string, abckit_wrapper::Method *> &methodTable,
                               const std::vector<MemberSpecification> &specifications,
                               std::unordered_map<std::string, abckit_wrapper::Member *> &matchedMembers);

    bool CollectMatchedFields(const std::unordered_map<std::string, abckit_wrapper::Field *> &fieldTable,
                              const std::vector<MemberSpecification> &specifications,
                              std::unordered_map<std::string, abckit_wrapper::Member *> &matchedMembers);

    bool MarkClassKept(abckit_wrapper::Class *clazz) const;

    [[nodiscard]] std::vector<ClassSpecification> CollectNameMatchedSpecification(const std::string &name,
                                                                                  uint32_t typeDeclarations) const;

    void MarkModuleKept(abckit_wrapper::Module *module);

    void MarkNamespaceKept(abckit_wrapper::Namespace *ns);

    Configuration configuration_ {};

    bool keepModuleAllChild_ = false;

    bool keepNamespaceAllChild_ = false;
};
}  // namespace ark::guard

#endif  // GUARD_OBFUSCATOR_NAME_MARKER_H