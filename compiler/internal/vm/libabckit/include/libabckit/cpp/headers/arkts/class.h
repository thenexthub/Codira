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

#ifndef CPP_ABCKIT_ARKTS_CLASS_H
#define CPP_ABCKIT_ARKTS_CLASS_H

#include "../core/class.h"
#include "../base_concepts.h"

namespace abckit::arkts {

/**
 * @brief Class
 */
class Class final : public core::Class {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class Module;
    /// @brief to access private constructor
    friend class Namespace;
    /// @brief abckit::DefaultHash<Class>
    friend class abckit::DefaultHash<Class>;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<Class>;

public:
    /**
     * @brief Constructor Arkts API Class from the Core API with compatibility check
     * @param coreOther - Core API Class
     */
    explicit Class(const core::Class &coreOther);

    /**
     * @brief Construct a new Class object
     * @param other
     */
    Class(const Class &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Class&
     */
    Class &operator=(const Class &other) = default;

    /**
     * @brief Construct a new Class object
     * @param other
     */
    Class(Class &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Class&
     */
    Class &operator=(Class &&other) = default;

    /**
     * @brief Destroy the Class object
     */
    ~Class() override = default;

    /**
     * @brief Sets name for class
     * @return `true` on success.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool SetName(const std::string &name) const;

    /**
     * @brief Add field to the class declaration.
     * @return Newly Add ClassField.
     * @param [ in ] name - Name to be set.
     * @param [ in ] type - Type to be set.
     * @param [ in ] value - Value to be set.
     * @param [ in ] isStatic - isStatic to be set.
     * @param [ in ] fieldVisibility - fieldVisibility to be set.
     * @note Allocates
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `type` is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if Class doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    arkts::ClassField AddField(const std::string_view name, const Type &type, const Value &value, bool isStatic = false,
                               AbckitArktsFieldVisibility fieldVisibility = AbckitArktsFieldVisibility::PUBLIC);

    /**
     * @brief Add field to the class declaration.
     * @return Newly Add ClassField.
     * @param [ in ] name - Name to be set.
     * @param [ in ] type - Type to be set.
     * @param [ in ] isStatic - isStatic to be set.
     * @param [ in ] fieldVisibility - fieldVisibility to be set.
     * @note Allocates
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `type` is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if Class doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    arkts::ClassField AddField(const std::string_view name, const Type &type, bool isStatic = false,
                               AbckitArktsFieldVisibility fieldVisibility = AbckitArktsFieldVisibility::PUBLIC);

    /**
     * @brief Sets owning module for class.
     * @return `true` on success.
     * @param [ in ] module - Module to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if module is false.
     */
    bool SetOwningModule(const Module &module) const;

    /**
     * @brief Remove annotation from the class declaration.
     * @param [ in ] anno - Annotation to remove.
     * @return New state of Class.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current `Class` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if current `Class` doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    Class RemoveAnnotation(arkts::Annotation anno) const;

    /**
     * @brief Add annotation to the class declaration.
     * @return Newly created annotation.
     * @param [ in ] ai - Annotation Interface that is used to create the annotation.
     * @note Allocates
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ai` is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if class Class doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    Annotation AddAnnotation(AnnotationInterface ai);

    /**
     * @brief Adds interface `iface` to the class declaration.
     * @return `true` on success.
     * @param [ in ] iface - Interface to be added.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     */
    bool AddInterface(arkts::Interface iface);

    /**
     * @brief Removes interface `iface` from the class declaration.
     * @return `true` on success.
     * @param [ in ] iface - Interface to be removed.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is not an interface of the class.
     */
    bool RemoveInterface(arkts::Interface iface);

    /**
     * @brief Set super class `superClass` for the class declaration.
     * @return `true` on success.
     * @param [ in ] superClass - Super class to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `superClass` is NULL.
     */
    bool SetSuperClass(arkts::Class superClass);

    /**
     * @brief Creates a new class with name `name`.
     * @return The newly created Class object.
     * @param [ in ] m - The module in which to create the class.
     * @param [ in ] name - The name of the class to create.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     */
    static Class CreateClass(Module m, const std::string &name);

    /**
     * @brief Remove class field from the class declaration.
     * @param [ in ] field - class field to remove.
     * @return `true` on success.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current `Class` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is false.
     */
    bool RemoveField(arkts::ClassField field) const;

    /**
     * @brief Add function to the class declaration.
     * @param [ in ] function - the function is added to the class.
     * @return `true` on success.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current `Class` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is false.
     */
    bool AddMethod(arkts::Function function);

    /**
     * @brief Removes method `method` from the list of methods for the class declaration.
     * @return `true` on success.
     * @param [ in ] method - Method to be removed.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `method` is NULL.
     */
    bool RemoveMethod(arkts::Function method);

private:
    /**
     * @brief Converts class from Core to Arkts target
     * @return AbckitArktsClass* - converted class
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsClass *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<Class> targetChecker_;
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_CLASS_H
