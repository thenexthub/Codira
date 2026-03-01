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

#ifndef CPP_ABCKIT_CORE_INTERFACE_H
#define CPP_ABCKIT_CORE_INTERFACE_H

#include "../base_classes.h"
#include <vector>

namespace abckit::core {

/**
 * @brief Interface
 */
class Interface : public ViewInResource<AbckitCoreInterface *, const File *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class core::Module;
    /// @brief to access private constructor
    friend class core::Namespace;
    /// @brief to access private constructor
    friend class core::Class;
    /// @brief to access private constructor
    friend class Function;
    /// @brief to access private constructor
    friend class InterfaceField;
    /// @brief abckit::DefaultHash<Interface>
    friend abckit::DefaultHash<Interface>;
    /// @brief to access private constructor
    friend class abckit::File;
    /// @brief to access private constructor
    friend class arkts::Interface;

protected:
    /// @brief Core API View type
    using CoreViewT = Interface;

public:
    /**
     * @brief Construct a new Interface object
     * @param other
     */
    Interface(const Interface &other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

    /**
     * @brief Constructor
     * @param other
     * @return Interface&
     */
    Interface &operator=(const Interface &other) = default;

    /**
     * @brief Construct a new Interface object
     * @param other
     */
    Interface(Interface &&other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

    /**
     * @brief Constructor
     * @param other
     * @return Interface&
     */
    Interface &operator=(Interface &&other) = default;

    /**
     * @brief Destroy the Interface object
     */
    ~Interface() override = default;

    /**
     * @brief Returns binary file that the Interface is a part of.
     * @return Pointer to the `File`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    const File *GetFile() const;

    /**
     * @brief Get Interface name
     * @return `std::string`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::string GetName() const;

    /**
     * @brief Tells if Interface is defined in the same binary or externally in another binary.
     * @return Returns `true` if Interface is defined in another binary and `false` if defined locally.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsExternal() const;

    /**
     * @brief Returns module for this `Interface`.
     * @return Owning `core::Module`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `bool(*this)` results in `false`.
     */
    core::Module GetModule() const;

    /**
     * @brief Returns parent namespace for interface.
     * @return `core::Namespace`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Namespace GetParentNamespace() const;

    /**
     * @brief Get vector with all Methods
     * @return std::vector<core::Function>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Function> GetAllMethods() const;

    /**
     * @brief Return vector with interface's fields.
     * @return std::vector<core::InterfaceField>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::InterfaceField> GetFields() const;

    /**
     * @brief Return vector with interface's super interfaces.
     * @return std::vector<core::Interface>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Interface> GetSuperInterfaces() const;

    /**
     * @brief Return vector with interface's sub interfaces.
     * @return std::vector<core::Interface>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Interface> GetSubInterfaces() const;

    /**
     * @brief Return vector with classes that implement this interface.
     * @return std::vector<core::Class>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Class> GetClasses() const;

    /**
     * @brief Get vector with all Annotations
     * @return std::vector<core::Annotation>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Annotation> GetAnnotations() const;

private:
    Interface(AbckitCoreInterface *iface, const ApiConfig *conf, const File *file) : ViewInResource(iface), conf_(conf)
    {
        SetResource(file);
    }

    const ApiConfig *conf_;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_INTERFACE_H
