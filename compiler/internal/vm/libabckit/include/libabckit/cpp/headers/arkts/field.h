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

#ifndef CPP_ABCKIT_ARKTS_FIELD_H
#define CPP_ABCKIT_ARKTS_FIELD_H

#include "../core/field.h"
#include "../base_concepts.h"

namespace abckit::arkts {

/**
 * @brief ModuleField
 */
class ModuleField final : public core::ModuleField {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief abckit::DefaultHash<ModuleField>
    friend class abckit::DefaultHash<ModuleField>;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<ModuleField>;

public:
    /**
     * @brief Constructor Arkts API ModuleField from the Core API with compatibility check
     * @param coreOther - Core API ModuleField
     */
    explicit ModuleField(const core::ModuleField &coreOther);

    /**
     * @brief Construct a new ModuleField object
     * @param other
     */
    ModuleField(const ModuleField &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Class&
     */
    ModuleField &operator=(const ModuleField &other) = default;

    /**
     * @brief Construct a new ModuleField object
     * @param other
     */
    ModuleField(ModuleField &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return ModuleField&
     */
    ModuleField &operator=(ModuleField &&other) = default;

    /**
     * @brief Destroy the ModuleField object
     */
    ~ModuleField() override = default;

    /**
     * @brief Sets name for module field
     * @return `true` on success.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool SetName(const std::string &name) const;

    /**
     * @brief Set type for module field
     * @return `true` on success.
     * @param [ in ] type - Type to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool SetType(const Type &type) const;

    /**
     * @brief Set value for module field
     * @return `true` on success.
     * @param [ in ] value - Value to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool SetValue(const Value &value) const;

    /**
     * @brief Add annotation to the moduleField declaration.
     * @return `true` on success.
     * @param [ in ] ai - Annotation Interface that is used to create the annotation.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ai` is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if `ModuleField` doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    bool AddAnnotation(const AnnotationInterface &ai) const;

    /**
     * @brief Remove annotation from the moduleField declaration.
     * @return `true` on success.
     * @param [ in ] anno - Annotation to remove.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if `ModuleField` doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    bool RemoveAnnotation(const arkts::Annotation &anno) const;

private:
    /**
     * @brief Converts ModuleField from Core to Arkts target
     * @return AbckitArktsModuleField* - converted ModuleField
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsModuleField *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<ModuleField> targetChecker_;
};

/**
 * @brief NamespaceField
 */
class NamespaceField final : public core::NamespaceField {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief abckit::DefaultHash<NamespaceField>
    friend class abckit::DefaultHash<NamespaceField>;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<NamespaceField>;

public:
    /**
     * @brief Constructor Arkts API NamespaceField from the Core API with compatibility check
     * @param coreOther - Core API NamespaceField
     */
    explicit NamespaceField(const core::NamespaceField &coreOther);

    /**
     * @brief Construct a new NamespaceField object
     * @param other
     */
    NamespaceField(const NamespaceField &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Class&
     */
    NamespaceField &operator=(const NamespaceField &other) = default;

    /**
     * @brief Construct a new NamespaceField object
     * @param other
     */
    NamespaceField(NamespaceField &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return NamespaceField&
     */
    NamespaceField &operator=(NamespaceField &&other) = default;

    /**
     * @brief Destroy the NamespaceField object
     */
    ~NamespaceField() override = default;

    /**
     * @brief Sets name for namespace field
     * @return `true` on success.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool SetName(const std::string &name) const;

private:
    /**
     * @brief Converts NamespaceField from Core to Arkts target
     * @return AbckitArktsNamespaceField* - converted NamespaceField
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsNamespaceField *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<NamespaceField> targetChecker_;
};

/**
 * @brief ClassField
 */
class ClassField final : public core::ClassField {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief abckit::DefaultHash<ClassField>
    friend class abckit::DefaultHash<ClassField>;
    /// @brief to access private constructor
    friend class Class;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<ClassField>;

public:
    /**
     * @brief Constructor Arkts API ClassField from the Core API with compatibility check
     * @param coreOther - Core API ClassField
     */
    explicit ClassField(const core::ClassField &coreOther);

    /**
     * @brief Construct a new ClassField object
     * @param other
     */
    ClassField(const ClassField &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Class&
     */
    ClassField &operator=(const ClassField &other) = default;

    /**
     * @brief Construct a new ClassField object
     * @param other
     */
    ClassField(ClassField &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return ClassField&
     */
    ClassField &operator=(ClassField &&other) = default;

    /**
     * @brief Destroy the ClassField object
     */
    ~ClassField() override = default;

    /**
     * @brief Sets name for class field
     * @return `true` on success.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current `ClassField` is false.
     */
    bool SetName(const std::string &name) const;

    /**
     * @brief Set type for class field
     * @return `true` on success.
     * @param [ in ] type - Type to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool SetType(const Type &type) const;

    /**
     * @brief Set value for class field
     * @return `true` on success.
     * @param [ in ] value - Value to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool SetValue(const Value &value) const;

    /**
     * @brief Add annotation to the classField declaration.
     * @return `true` on success.
     * @param [ in ] ai - Annotation Interface that is used to create the annotation.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ai` is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if `ClassField` doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    bool AddAnnotation(const AnnotationInterface &ai) const;

    /**
     * @brief Remove annotation from the classField declaration.
     * @return `true` on success.
     * @param [ in ] anno - Annotation to remove.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if `ClassField` doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    bool RemoveAnnotation(const arkts::Annotation &anno) const;

private:
    /**
     * @brief Converts classField from Core to Arkts target
     * @return AbckitArktsClassField* - converted classField
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsClassField *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<ClassField> targetChecker_;
};

/**
 * @brief InterfaceField
 */
class InterfaceField final : public core::InterfaceField {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief abckit::DefaultHash<InterfaceField>
    friend class abckit::DefaultHash<InterfaceField>;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<InterfaceField>;
    /// @brief to access private constructor
    friend class abckit::arkts::Interface;

public:
    /**
     * @brief Constructor Arkts API InterfaceField from the Core API with compatibility check
     * @param coreOther - Core API InterfaceField
     */
    explicit InterfaceField(const core::InterfaceField &coreOther);

    /**
     * @brief Construct a new InterfaceField object
     * @param other
     */
    InterfaceField(const InterfaceField &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Class&
     */
    InterfaceField &operator=(const InterfaceField &other) = default;

    /**
     * @brief Construct a new InterfaceField object
     * @param other
     */
    InterfaceField(InterfaceField &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return InterfaceField&
     */
    InterfaceField &operator=(InterfaceField &&other) = default;

    /**
     * @brief Destroy the InterfaceField object
     */
    ~InterfaceField() override = default;

    /**
     * @brief Sets name for interface field
     * @return `true` on success.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current `ClassField` is false.
     */
    bool SetName(const std::string &name) const;

    /**
     * @brief Set type for interface field
     * @return `true` on success.
     * @param [ in ] type - Type to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool SetType(const Type &type) const;

    /**
     * @brief Add annotation to the interface field.
     * @return `true` on success.
     * @param [ in ] ai - Annotation Interface that is used to create the annotation.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ai` is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if `InterfaceField` doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    bool AddAnnotation(const AnnotationInterface &ai) const;

    /**
     * @brief Remove annotation from the interface field.
     * @return `true` on success.
     * @param [ in ] anno - Annotation to remove.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if `InterfaceField` doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    bool RemoveAnnotation(const arkts::Annotation &anno) const;

private:
    /**
     * @brief Converts InterfaceField from Core to Arkts target
     * @return AbckitArktsInterfaceField* - converted InterfaceField
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsInterfaceField *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<InterfaceField> targetChecker_;
};

/**
 * @brief EnumField
 */
class EnumField final : public core::EnumField {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief abckit::DefaultHash<EnumField>
    friend class abckit::DefaultHash<EnumField>;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<EnumField>;

public:
    /**
     * @brief Constructor Arkts API EnumField from the Core API with compatibility check
     * @param coreOther - Core API EnumField
     */
    explicit EnumField(const core::EnumField &coreOther);

    /**
     * @brief Construct a new EnumField object
     * @param other
     */
    EnumField(const EnumField &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Class&
     */
    EnumField &operator=(const EnumField &other) = default;

    /**
     * @brief Construct a new EnumField object
     * @param other
     */
    EnumField(EnumField &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return EnumField&
     */
    EnumField &operator=(EnumField &&other) = default;

    /**
     * @brief Destroy the EnumField object
     */
    ~EnumField() override = default;

    /**
     * @brief Sets name for enum field
     * @return `true` on success.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current `ClassField` is false.
     */
    bool SetName(const std::string &name) const;

private:
    /**
     * @brief Converts EnumField from Core to Arkts target
     * @return AbckitArktsEnumField* - converted EnumField
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsEnumField *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<EnumField> targetChecker_;
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_FIELD_H
