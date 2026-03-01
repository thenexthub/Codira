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

#ifndef CPP_ABCKIT_TYPE_H
#define CPP_ABCKIT_TYPE_H

#include "base_classes.h"
#include "core/class.h"
#include "libabckit/c/abckit.h"

#include <string>
#include <vector>

namespace abckit {

/**
 * @brief Type
 */
class Type : public ViewInResource<AbckitType *, const File *> {
    /// @brief abckit::File
    friend class abckit::File;
    /// @brief abckit::File
    friend class abckit::core::AnnotationInterfaceField;
    /// @brief abckit::Value
    friend class abckit::Value;
    /// @brief arkts::AnnotationInterface
    friend class arkts::AnnotationInterface;
    /// @brief abckit::Instruction
    friend class abckit::Instruction;
    /// @brief core::ModuleField
    friend class abckit::core::ModuleField;
    /// @brief core::NamespaceField
    friend class abckit::core::NamespaceField;
    /// @brief core::ClassField
    friend class abckit::core::ClassField;
    /// @brief core::InterfaceField
    friend class abckit::core::InterfaceField;
    /// @brief core::EnumField
    friend class abckit::core::EnumField;
    /// @brief core::FunctionParam
    friend class abckit::core::FunctionParam;
    /// @brief core::Function
    friend class abckit::core::Function;
    /// @brief arkts::ModuleField
    friend class arkts::ModuleField;
    /// @brief arkts::ClassField
    friend class arkts::ClassField;
    /// @brief arkts::Class
    friend class arkts::Class;
    /// @brief arkts::Module
    friend class arkts::Module;
    /// @brief arkts::InterfaceField
    friend class arkts::InterfaceField;
    /// @brief arkts::Interface
    friend class arkts::Interface;
    /// @brief arkts::Enum
    friend class arkts::Enum;

public:
    /**
     * @brief Construct a new Type object
     * @param other
     */
    Type(const Type &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Type&
     */
    Type &operator=(const Type &other) = default;

    /**
     * @brief Construct a new Type object
     * @param other
     */
    Type(Type &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Type&
     */
    Type &operator=(Type &&other) = default;

    /**
     * @brief Destroy the Type object
     */
    ~Type() override = default;

    /**
     * @brief Returns the Type Id of type
     * @return Returns the AbckitTypeId
     */
    inline enum AbckitTypeId GetTypeId() const
    {
        auto ret = GetApiConfig()->cIapi_->typeGetTypeId(GetView());
        CheckError(GetApiConfig());
        return ret;
    }

    /**
     * @brief Returns instance of a `core::Class` that the type is reference to.
     * @return Instance of a `core::Class` that the type references.
     */
    inline core::Class GetReferenceClass() const
    {
        auto *ret = GetApiConfig()->cIapi_->typeGetReferenceClass(GetView());
        CheckError(GetApiConfig());
        return core::Class(ret, GetApiConfig(), GetResource());
    }

    /**
     * @brief Returns the Type Name of type
     * @return std::String
     */
    inline std::string GetName() const
    {
        const ApiConfig *conf = GetApiConfig();
        AbckitString *cString = conf->cIapi_->typeGetName(GetView());
        CheckError(conf);
        std::string str = conf->cIapi_->abckitStringToString(cString);
        CheckError(conf);
        return str;
    }

    /**
     * @brief Returns the Rank of type
     * @return Returns the Rank of type
     */
    inline size_t GetRank() const
    {
        auto ret = GetApiConfig()->cIapi_->typeGetRank(GetView());
        CheckError(GetApiConfig());
        return ret;
    }

    /**
     * @brief Sets the Name of type
     * @param name The Name of type
     */
    inline void SetName(const std::string &name)
    {
        GetApiConfig()->cMapi_->typeSetName(GetView(), name.c_str(), name.size());
        CheckError(GetApiConfig());
    }

    /**
     * @brief Sets the Rank of type
     * @param rank The Rank of type
     */
    inline void SetRank(size_t rank)
    {
        GetApiConfig()->cMapi_->typeSetRank(GetView(), rank);
        CheckError(GetApiConfig());
    }

    /**
     * @brief Returns true if the type is a union
     * @return true if the type is a union
     */
    inline bool IsUnion() const
    {
        auto ret = GetApiConfig()->cIapi_->typeIsUnion(GetView());
        CheckError(GetApiConfig());
        return ret;
    }

    /**
     * @brief Enumerates the union types
     * @param types The union types
     * @return true if the enumeration is successful
     */
    inline bool EnumerateUnionTypes(std::vector<Type> &types) const
    {
        Payload<std::vector<Type> *> payload {&types, GetApiConfig(), GetResource()};
        auto ret =
            GetApiConfig()->cIapi_->typeEnumerateUnionTypes(GetView(), &payload, [](AbckitType *type, void *data) {
                const auto &payload = *static_cast<Payload<std::vector<Type> *> *>(data);
                payload.data->push_back(Type(type, payload.config, payload.resource));
                return true;
            });
        CheckError(GetApiConfig());
        return ret;
    }

protected:
    /**
     * @brief Get the Api Config object
     * @return const ApiConfig*
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }

private:
    explicit Type(AbckitType *type, const ApiConfig *conf, const File *file) : ViewInResource(type), conf_(conf)
    {
        SetResource(file);
    };
    const ApiConfig *conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_TYPE_H
