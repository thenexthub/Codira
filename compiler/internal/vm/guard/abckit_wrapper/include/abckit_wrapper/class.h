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

#ifndef ABCKIT_WRAPPER_CLASS_H
#define ABCKIT_WRAPPER_CLASS_H

#include "method.h"
#include "field.h"

namespace abckit_wrapper {
/**
 * @brief Class
 */
class Class final : public Object,
                    public AnnotationTarget,
                    public AccessFlagsTarget,
                    public MultiImplObject<abckit::core::Class, abckit::core::Interface, abckit::core::Enum> {
public:
    /**
     * @brief Class Constructor
     * @param clazz abckit::core::Class, abckit::core::Interface or abckit::core::Enum
     */
    explicit Class(std::variant<abckit::core::Class, abckit::core::Interface, abckit::core::Enum> clazz)
        : MultiImplObject(std::move(clazz))
    {
    }

    AbckitWrapperErrorCode Init() override;

    std::string GetName() const override;

    bool SetName(const std::string &name) override;

    /**
     * @brief Tells if Class is defined in the same binary or externally in another binary.
     * @return Returns `true` if Class is defined in another binary and `false` if defined locally.
     */
    bool IsExternal() const;

    /**
     * @brief Check if it is an interface
     * @return Returns `true` Class actual is interface, `false` not interface
     */
    bool IsInterface() const;

    /**
     * @brief Check if it is a class
     * @return Returns `true` Class actual is class, `false` not class
     */
    bool IsClass() const;

    /**
     * @brief Check if it is an enum
     * @return Returns `true` Class actual is enum, `false` not enum
     */
    bool IsEnum() const;

    /**
     * @brief Accept visit
     * @param visitor ClassVisitor
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool Accept(ClassVisitor &visitor);

    /**
     * @brief Accept ChildVisitor visit
     * @param visitor ChildVisitor
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool Accept(ChildVisitor &visitor);

    /**
     * @brief Class members accept visit
     * @param visitor MemberVisitor
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool MembersAccept(MemberVisitor &visitor);

    /**
     * @brief Class hierarchy accept
     * @param visitor ClassVisitor
     * @param visitThis whether visit current class
     * @param visitSuper whether visit super class
     * @param visitInterfaces whether visit interfaces
     * @param visitSubclasses whether visit subclassed
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool HierarchyAccept(ClassVisitor &visitor, bool visitThis, bool visitSuper, bool visitInterfaces,
                         bool visitSubclasses);

private:
    AbckitWrapperErrorCode InitMethods();

    AbckitWrapperErrorCode InitFields();

    void InitAnnotation(Annotation *annotation) override;

    std::vector<abckit::core::Annotation> GetAnnotations() const override;

    void InitAccessFlags() override;

public:
    std::optional<Class *> superClass_;
    std::unordered_map<std::string, Class *> interfaces_;
    std::unordered_map<std::string, Class *> subClasses_;
    std::unordered_map<std::string, Method *> methodTable_;
    std::unordered_map<std::string, Field *> fieldTable_;
};
}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_CLASS_H
