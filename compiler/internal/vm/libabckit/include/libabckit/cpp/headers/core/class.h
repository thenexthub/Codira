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

#ifndef CPP_ABCKIT_CORE_CLASS_H
#define CPP_ABCKIT_CORE_CLASS_H

#include "../base_classes.h"
#include "function.h"

#include <functional>
#include <vector>

namespace abckit::core {

/**
 * @brief Class
 */
class Class : public ViewInResource<AbckitCoreClass *, const File *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class core::Module;
    /// @brief to access private constructor
    friend class core::Namespace;
    /// @brief to access private constructor
    friend class core::Interface;
    /// @brief to access private constructor
    friend class core::Function;
    /// @brief to access private constructor
    friend class core::ClassField;
    /// @brief to access private constructor
    friend class abckit::Type;
    /// @brief to access private constructor
    friend class arkts::Class;
    /// @brief to access private constructor
    friend class arkts::Module;
    /// @brief abckit::DefaultHash<Class>
    friend class abckit::DefaultHash<Class>;
    /// @brief to access private constructor
    friend class abckit::File;

protected:
    /// @brief Core API View type
    using CoreViewT = Class;

public:
    /**
     * @brief Construct a new Class object
     * @param other
     */
    Class(const Class &other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

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
    Class(Class &&other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

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
     * @brief Returns binary file that the Class is a part of.
     * @return Pointer to the `File`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    const File *GetFile() const;

    /**
     * @brief Get Class name
     * @return `std::string`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::string GetName() const;

    /**
     * @brief Tells if Class is defined in the same binary or externally in another binary.
     * @return Returns `true` if Class is defined in another binary and `false` if defined locally.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsExternal() const;

    /**
     * @brief Tell if Class is final
     * @return Returns `true` if Class is final class and `false` otherwise
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsFinal() const;

    /**
     * @brief Tell if Class is abstract
     * @return Returns `true` if Class is abstract and `false`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsAbstract() const;

    /**
     * @brief Returns module for this `Class`.
     * @return Owning `core::Module`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `bool(*this)` results in `false`.
     */
    core::Module GetModule() const;

    /**
     * @brief Get vector with all Methods
     * @return std::vector<core::Function>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Function> GetAllMethods() const;

    /**
     * @brief Return vector with class's fields.
     * @return std::vector<core::ClassField>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::ClassField> GetFields() const;

    /**
     * @brief Get vector with all Annotations
     * @return std::vector<core::Annotation>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Annotation> GetAnnotations() const;

    /**
     * @brief Enumerates methods of Class, invoking callback `cb` for each method.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool EnumerateMethods(const std::function<bool(core::Function)> &cb) const;

    /**
     * @brief Enumerates annotations of Class, invoking callback `cb` for each annotation.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @return false` if was early exited. Otherwise - `true`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is false.
     */
    bool EnumerateAnnotations(const std::function<bool(core::Annotation)> &cb) const;

    /**
     * @brief Returns parent function for class.
     * @return `core::Function`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Function GetParentFunction() const;

    /**
     * @brief Returns parent namespace for class.
     * @return `core::Namespace`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Namespace GetParentNamespace() const;

    /**
     * @brief Returns super class for class.
     * @return `core::Class`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Class GetSuperClass() const;

    /**
     * @brief Returns subclasses for class.
     * @return `std::vector<core::Core>`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<Class> GetSubClasses() const;

    /**
     * @brief Returns implemented interfaces for class.
     * @return `std::vector<core::Core>`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<Interface> GetInterfaces() const;

private:
    Class(AbckitCoreClass *klass, const ApiConfig *conf, const File *file) : ViewInResource(klass), conf_(conf)
    {
        SetResource(file);
    };

    const ApiConfig *conf_;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_CLASS_H
