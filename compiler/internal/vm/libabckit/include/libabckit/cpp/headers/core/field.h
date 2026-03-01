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

#ifndef CPP_ABCKIT_CORE_FIELD_H
#define CPP_ABCKIT_CORE_FIELD_H

#include <vector>
#include "../base_classes.h"

namespace abckit::core {

/**
 * @brief ModuleField
 */
class ModuleField : public ViewInResource<AbckitCoreModuleField *, const File *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class core::Module;
    /// @brief to access private constructor
    friend class arkts::Module;
    /// @brief to access private constructor
    friend class core::Namespace;
    /// @brief abckit::DefaultHash<ModuleField>
    friend class abckit::DefaultHash<ModuleField>;

protected:
    /// @brief Core API View type
    using CoreViewT = ModuleField;

public:
    /**
     * @brief Construct a new ModuleField object
     * @param other
     */
    ModuleField(const ModuleField &other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

    /**
     * @brief Constructor
     * @param other
     * @return ModuleField&
     */
    ModuleField &operator=(const ModuleField &other) = default;

    /**
     * @brief Construct a new ModuleField object
     * @param other
     */
    ModuleField(ModuleField &&other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

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
     * @brief Returns module for module field.
     * @return core::Module
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Module GetModule() const;

    /**
     * @brief Returns name.
     * @return std::string
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::string GetName() const;

    /**
     * @brief Returns type.
     * @return abckit::Type
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Type GetType() const;

    /**
     * @brief Returns value.
     * @return abckit::Value
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Value GetValue() const;

    /**
     * @brief Tell if field is Public.
     * @return Return `true` if field is Public otherwise false
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsPublic() const;

    /**
     * @brief Tell if field is Private.
     * @return Return `true` if field is Private otherwise false
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsPrivate() const;

private:
    ModuleField(AbckitCoreModuleField *field, const ApiConfig *conf, const File *file)
        : ViewInResource(field), conf_(conf)
    {
        SetResource(file);
    };
    const ApiConfig *conf_;

protected:
    /**
     * @brief Get the Api Config object
     * @return const ApiConfig*
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

/**
 * @brief NamespaceField
 */
class NamespaceField : public ViewInResource<AbckitCoreNamespaceField *, const File *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class core::Namespace;
    /// @brief abckit::DefaultHash<NamespaceField>
    friend class abckit::DefaultHash<NamespaceField>;

protected:
    /// @brief Core API View type
    using CoreViewT = NamespaceField;

public:
    /**
     * @brief Construct a new NamespaceField object
     * @param other
     */
    NamespaceField(const NamespaceField &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return NamespaceField&
     */
    NamespaceField &operator=(const NamespaceField &other) = default;

    /**
     * @brief Construct a new Field object
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
     * @brief Returns namespace for namespace field.
     * @return core::Class
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Namespace GetNamespace() const;

    /**
     * @brief Returns name.
     * @return std::string
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::string GetName() const;

    /**
     * @brief Returns type.
     * @return abckit::Type
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Type GetType() const;

private:
    NamespaceField(AbckitCoreNamespaceField *field, const ApiConfig *conf, const File *file)
        : ViewInResource(field), conf_(conf)
    {
        SetResource(file);
    };
    const ApiConfig *conf_;

protected:
    /**
     * @brief Get the Api Config object
     * @return const ApiConfig*
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

/**
 * @brief ClassField
 */
class ClassField : public ViewInResource<AbckitCoreClassField *, const File *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class core::Class;
    /// @brief to access private constructor
    friend class arkts::Class;
    /// @brief abckit::DefaultHash<ClassField>
    friend class abckit::DefaultHash<ClassField>;

protected:
    /// @brief Core API View type
    using CoreViewT = ClassField;

public:
    /**
     * @brief Construct a new ClassField object
     * @param other
     */
    ClassField(const ClassField &other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

    /**
     * @brief Constructor
     * @param other
     * @return ClassField&
     */
    ClassField &operator=(const ClassField &other) = default;

    /**
     * @brief Construct a new Field object
     * @param other
     */
    ClassField(ClassField &&other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

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
     * @brief Returns class for class field.
     * @return core::Class
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Class GetClass() const;

    /**
     * @brief Returns name.
     * @return std::string
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::string GetName() const;

    /**
     * @brief Returns type.
     * @return abckit::Type
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Type GetType() const;

    /**
     * @brief Returns value.
     * @return abckit::Value
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Value GetValue() const;

    /**
     * @brief Tell if field is Public.
     * @return Return `true` if field is Public otherwise false
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsPublic() const;

    /**
     * @brief Tell if field is Public.
     * @return Return `true` if field is Protected otherwise false
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsProtected() const;

    /**
     * @brief Tell if field is Private.
     * @return Return `true` if field is Private otherwise false
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsPrivate() const;

    /**
     * @brief Tell if field is Internal.
     * @return Return `true` if field is Internal otherwise false
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsInternal() const;

    /**
     * @brief Tell if field is Static.
     * @return Return `true` if field is Static otherwise false
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsStatic() const;

    /**
     * @brief Tell if field is Final.
     * @return Return `true` if field is Final otherwise false
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsFinal() const;

    /**
     * @brief Get vector with all Annotations
     * @return std::vector<core::Annotation>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Annotation> GetAnnotations() const;

private:
    ClassField(AbckitCoreClassField *field, const ApiConfig *conf, const File *file)
        : ViewInResource(field), conf_(conf)
    {
        SetResource(file);
    };
    const ApiConfig *conf_;

protected:
    /**
     * @brief Get the Api Config object
     * @return const ApiConfig*
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

/**
 * @brief InterfaceField
 */
class InterfaceField : public ViewInResource<AbckitCoreInterfaceField *, const File *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class core::Interface;
    /// @brief to access private constructor
    friend class arkts::Interface;
    /// @brief abckit::DefaultHash<InterfaceField>
    friend class abckit::DefaultHash<InterfaceField>;

protected:
    /// @brief Core API View type
    using CoreViewT = InterfaceField;

public:
    /**
     * @brief Construct a new InterfaceField object
     * @param other
     */
    InterfaceField(const InterfaceField &other) =
        default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

    /**
     * @brief Constructor
     * @param other
     * @return InterfaceField&
     */
    InterfaceField &operator=(const InterfaceField &other) = default;

    /**
     * @brief Construct a new InterfaceField object
     * @param other
     */
    InterfaceField(InterfaceField &&other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

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
     * @brief Returns Interface for interface field.
     * @return core::Interface
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Interface GetInterface() const;

    /**
     * @brief Returns name.
     * @return std::string
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::string GetName() const;

    /**
     * @brief Returns type.
     * @return abckit::Type
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Type GetType() const;

    /**
     * @brief Tell if field is Private.
     * @return Return `true` if field is Private otherwise false
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsPrivate() const;

    /**
     * @brief Tell if field is Readonly.
     * @return Return `true` if field is Readonly otherwise false
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsReadonly() const;

    /**
     * @brief Get vector with all Annotations
     * @return std::vector<core::Annotation>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Annotation> GetAnnotations() const;

private:
    InterfaceField(AbckitCoreInterfaceField *field, const ApiConfig *conf, const File *file)
        : ViewInResource(field), conf_(conf)
    {
        SetResource(file);
    };
    const ApiConfig *conf_;

protected:
    /**
     * @brief Get the Api Config object
     * @return const ApiConfig*
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

/**
 * @brief EnumField
 */
class EnumField : public ViewInResource<AbckitCoreEnumField *, const File *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class core::Enum;
    /// @brief to access private constructor
    friend class arkts::Enum;
    /// @brief abckit::DefaultHash<EnumField>
    friend class abckit::DefaultHash<EnumField>;

protected:
    /// @brief Core API View type
    using CoreViewT = EnumField;

public:
    /**
     * @brief Construct a new EnumField object
     * @param other
     */
    EnumField(const EnumField &other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

    /**
     * @brief Constructor
     * @param other
     * @return EnumField&
     */
    EnumField &operator=(const EnumField &other) = default;

    /**
     * @brief Construct a new EnumField object
     * @param other
     */
    EnumField(EnumField &&other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

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
     * @brief Returns Enum for enum field.
     * @return core::Enum
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Enum GetEnum() const;

    /**
     * @brief Returns name.
     * @return std::string
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::string GetName() const;

    /**
     * @brief Returns type.
     * @return abckit::Type
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Type GetType() const;

    /**
     * @brief Returns Value.
     * @return abckit::Value
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Value GetValue() const;

private:
    EnumField(AbckitCoreEnumField *field, const ApiConfig *conf, const File *file) : ViewInResource(field), conf_(conf)
    {
        SetResource(file);
    };
    const ApiConfig *conf_;

protected:
    /**
     * @brief Get the Api Config object
     * @return const ApiConfig*
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_FIELD_H
