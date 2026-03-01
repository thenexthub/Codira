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

#ifndef CPP_ABCKIT_CORE_NAMESPACE_H
#define CPP_ABCKIT_CORE_NAMESPACE_H

#include "../base_classes.h"

#include <functional>

namespace abckit::core {

/**
 * @brief Namespace
 */
class Namespace : public ViewInResource<AbckitCoreNamespace *, const File *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class abckit::File;
    /// @brief to access private constructor
    friend class core::Module;
    /// @brief to access private constructor
    friend class core::Class;
    /// @brief to access private constructor
    friend class core::Interface;
    /// @brief to access private constructor
    friend class core::Enum;
    /// @brief to access private constructor
    friend class core::NamespaceField;
    /// @brief to access private constructor
    friend class core::Function;
    /// @brief to access private constructor
    friend class core::AnnotationInterface;
    /// @brief abckit::DefaultHash<Namespace>
    friend class abckit::DefaultHash<Namespace>;

protected:
    /// @brief Core API View type
    using CoreViewT = Namespace;

public:
    /**
     * @brief Construct a new Namespace object
     * @param other
     */
    Namespace(const Namespace &other) = default;

    /**
     * @brief
     * @param other
     * @return Namespace&
     */
    Namespace &operator=(const Namespace &other) = default;

    /**
     * @brief Construct a new Namespace object
     * @param other
     */
    Namespace(Namespace &&other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

    /**
     * @brief
     * @param other
     * @return Namespace&
     */
    Namespace &operator=(Namespace &&other) = default;

    /**
     * @brief Destroy the Namespace object
     */
    ~Namespace() override = default;

    /**
     * @brief Returns module for this `Namespace`.
     * @return Owning `core::Module`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `bool(*this)` results in `false`.
     */
    core::Module GetModule() const;

    /**
     * @brief Return namespace's name.
     * @return `std::string`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::string GetName() const;

    /**
     * @brief Tells if Namespace is defined in the same binary or externally in another binary.
     * @return Returns `true` if Namespace is defined in another binary and `false` if defined locally.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsExternal() const;

    /**
     * @brief Returns parent namespace.
     * @return `core::Namespace` or NULL if namespace has no parent namespace.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Namespace GetParentNamespace() const;

    /**
     * @brief Return vector with namespace's classes.
     * @return std::vector<core::Class>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Class> GetClasses() const;

    /**
     * @brief Return vector with namespace's interfaces.
     * @return std::vector<core::Interface>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Interface> GetInterfaces() const;

    /**
     * @brief Return vector with namespace's enums.
     * @return std::vector<core::Enum>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Enum> GetEnums() const;

    /**
     * @brief Return vector with namespace's annotation interfaces.
     * @return std::vector<core::AnnotationInterface>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::AnnotationInterface> GetAnnotationInterfaces() const;

    /**
     * @brief Return vector with namespace's functions.
     * @return std::vector<core::Function>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Function> GetTopLevelFunctions() const;

    /**
     * @brief Return vector with namespace's fields.
     * @return std::vector<core::NamespaceField>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::NamespaceField> GetFields() const;

    /**
     * @brief Return vector with namespace's namespaces.
     * @return std::vector<core::Namespace>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Namespace> GetNamespaces() const;

    /**
     * @brief Enumerates namespaces defined inside of the Namespace, invoking callback `cb` for each inner
     * namespace.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if 'cb' is false.
     */
    bool EnumerateNamespaces(const std::function<bool(core::Namespace)> &cb) const;

    /**
     * @brief Enumerates classes of the Namespace, invoking callback `cb` for each class.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if 'cb' is false.
     */
    bool EnumerateClasses(const std::function<bool(core::Class)> &cb) const;

    /**
     * @brief Enumerates top level functions of the Namespace, invoking callback `cb` for each top level function.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if 'cb' is false.
     */
    bool EnumerateTopLevelFunctions(const std::function<bool(core::Function)> &cb) const;

private:
    Namespace(AbckitCoreNamespace *ns, const ApiConfig *conf, const File *file) : ViewInResource(ns), conf_(conf)
    {
        SetResource(file);
    };
    const ApiConfig *conf_;

protected:
    /**
     * @brief Returns Api Config.
     * @return const ApiConfig*
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_NAMESPACE_H
