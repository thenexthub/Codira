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

#ifndef ABCKIT_WRAPPER_FIELD_H
#define ABCKIT_WRAPPER_FIELD_H

#include <variant>
#include "libabckit/cpp/abckit_cpp.h"
#include "member.h"
#include "visitor.h"

namespace abckit_wrapper {
/**
 * @brief Field
 */
class Field final
    : public Member,
      public MultiImplObject<abckit::core::ModuleField, abckit::core::NamespaceField, abckit::core::ClassField,
                             abckit::core::InterfaceField, abckit::core::EnumField> {
public:
    /**
     * Field Constructor
     * @param field build from abckit::core field
     *  abckit::core::ModuleField
     *  abckit::core::NamespaceField
     *  abckit::core::ClassField
     *  abckit::core::InterfaceField
     *  abckit::core::EnumField
     */
    explicit Field(std::variant<abckit::core::ModuleField, abckit::core::NamespaceField, abckit::core::ClassField,
                                abckit::core::InterfaceField, abckit::core::EnumField>
                       field)
        : MultiImplObject(std::move(field))
    {
    }

    AbckitWrapperErrorCode Init() override;

    std::string GetName() const override;

    /**
     * @brief Get field type name
     * @return std::string field type name
     */
    std::string GetType() const;

    /**
     * @brief Check if field is interface field
     * @return true if field is interface field
     */
    bool IsInterfaceField() const;

    bool SetName(const std::string &name) override;

    /**
     * @brief Accept visit
     * @param visitor FieldVisitor
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool Accept(FieldVisitor &visitor);

    /**
     * @brief Accept ChildVisitor visit
     * @param visitor ChildVisitor
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool Accept(ChildVisitor &visitor);

private:
    AbckitWrapperErrorCode InitSignature() override;

    void InitAccessFlags() override;

    void InitAnnotation(Annotation *annotation) override;

    std::vector<abckit::core::Annotation> GetAnnotations() const override;

    bool SetFieldName(const std::string &name) const;
};
}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_FIELD_H
