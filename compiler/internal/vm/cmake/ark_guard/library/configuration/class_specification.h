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

#ifndef GUARD_CONFIGURATION_CLASS_SPECIFICATION_H
#define GUARD_CONFIGURATION_CLASS_SPECIFICATION_H

#include <cstdint>
#include <string>
#include <vector>

namespace ark::guard {

/**
 * @brief MemberSpecification
 */
class MemberSpecification {
public:
    /**
     * @brief default constructor, enabling object creation without parameters
     */
    MemberSpecification() = default;

    /**
     * @brief constructor, initializes the annotationName, setAccessFlags and unSetAccessFlags
     * @param annotationName annotation name
     * @param setAccessFlags set access modifier
     * @param unSetAccessFlags unspecified access modifier
     */
    MemberSpecification(std::string annotationName, uint32_t setAccessFlags, uint32_t unSetAccessFlags)
        : annotationName_(std::move(annotationName)),
          setAccessFlags_(setAccessFlags),
          unSetAccessFlags_(unSetAccessFlags)
    {
    }

    /**
     * @brief Set member info
     * @param memberName member name
     * @param memberType member type
     * @param memberValue member value
     */
    void SetMemberInfo(const std::string &memberName, const std::string &memberType, const std::string &memberValue)
    {
        memberName_ = memberName;
        memberType_ = memberType;
        memberValue_ = memberValue;
    }

    /**
     * @brief Set keep member not be obfuscated
     * @param keep whether to keep member to be obfuscated. false: obfuscated, true: not obfuscated
     */
    void SetKeepMember(bool keep)
    {
        keepMember_ = keep;
    }

    /**
     * @brief keep member not be obfuscated
     * The Default value is false, when the value is true, all members will not be obfuscated.
     */
    bool keepMember_ = false;

    /**
     * @brief annotation name
     */
    std::string annotationName_;

    /**
     * @brief Set access modifier
     */
    uint32_t setAccessFlags_ = 0;

    /**
     * @brief Unspecified access modifier
     */
    uint32_t unSetAccessFlags_ = 0;

    /**
     * @brief member name
     * when the member type is different, the meaning of this field value is also different
     * field: field name
     * method: method name
     */
    std::string memberName_;

    /**
     * @brief member type
     * when the member type is different, the meaning of this field value is also different
     * field: field type
     * method: argument description
     */
    std::string memberType_;

    /**
     * @brief member value
     * when the member type is different, the meaning of this field value is also different
     * field: field value
     * method: return type
     */
    std::string memberValue_;
};

/**
 * @brief ClassSpecification
 * Format: class_specification
 */
class ClassSpecification {
public:
    /**
     * @brief default constructor, enabling object creation without parameters
     */
    ClassSpecification() = default;

    /**
     * @brief constructor, initializes the annotationName, setAccessFlags and unSetAccessFlags
     * @param annotationName annotation name
     * @param setAccessFlags set access modifier
     * @param unSetAccessFlags unspecified access modifier
     */
    ClassSpecification(std::string annotationName, uint32_t setAccessFlags, uint32_t unSetAccessFlags)
        : annotationName_(std::move(annotationName)),
          setAccessFlags_(setAccessFlags),
          unSetAccessFlags_(unSetAccessFlags)
    {
    }

    /**
     * @brief Set type declarations
     * @param setTypeDeclarations set type declarations
     * @param unSetTypeDeclarations unspecified type declarations
     * @param className class name
     */
    void SetClassInfo(uint32_t setTypeDeclarations, uint32_t unSetTypeDeclarations, const std::string &className)
    {
        setTypeDeclarations_ = setTypeDeclarations;
        unSetTypeDeclarations_ = unSetTypeDeclarations;
        className_ = className;
    }

    /**
     * @brief Set keep className not be obfuscated
     * @param keep whether to keep className to be obfuscated. false: obfuscated, true: not obfuscated
     */
    void SetKeepClassName(bool keep)
    {
        keepClassName_ = keep;
    }

    /**
     * @brief Set extension info
     * @param extensionType extension type
     * @param extensionAnnotationName extension annotation name
     * @param extensionClassName extension class name
     */
    void SetExtensionInfo(uint32_t extensionType, const std::string &extensionAnnotationName,
                          const std::string &extensionClassName)
    {
        extensionType_ = extensionType;
        extensionAnnotationName_ = extensionAnnotationName;
        extensionClassName_ = extensionClassName;
    }

    /**
     * @brief add field specification
     * @param specification MemberSpecification
     */
    void AddField(MemberSpecification &&specification)
    {
        this->fieldSpecifications_.emplace_back(std::move(specification));
    }

    /**
     * @brief add method specification
     * @param specification MemberSpecification
     */
    void AddMethod(MemberSpecification &&specification)
    {
        this->methodSpecifications_.emplace_back(std::move(specification));
    }

    /**
     * @brief annotation name
     */
    std::string annotationName_;

    /**
     * @brief Set access modifier
     */
    uint32_t setAccessFlags_ = 0;

    /**
     * @brief Unspecified access modifier
     */
    uint32_t unSetAccessFlags_ = 0;

    /**
     * @brief Set type declarations
     */
    uint32_t setTypeDeclarations_ = 0;

    /**
     * @brief Unspecified type declarations
     */
    uint32_t unSetTypeDeclarations_ = 0;

    /**
     * @brief className, This value is the name of the object being kept
     */
    std::string className_;

    /**
     * @brief keep className not be obfuscated
     * The Default value is true, when the keep option is "-keepclassmembers", the value is false.
     */
    bool keepClassName_ = true;

    /**
     * @brief extension type
     */
    uint32_t extensionType_ = 0;

    /**
     * @brief extension annotation name
     */
    std::string extensionAnnotationName_;

    /**
     * @brief extension class name
     */
    std::string extensionClassName_;

    /**
     * @brief field specifications
     */
    std::vector<MemberSpecification> fieldSpecifications_;

    /**
     * @brief method specifications
     */
    std::vector<MemberSpecification> methodSpecifications_;
};
}  // namespace ark::guard

#endif  // GUARD_CONFIGURATION_CLASS_SPECIFICATION_H
