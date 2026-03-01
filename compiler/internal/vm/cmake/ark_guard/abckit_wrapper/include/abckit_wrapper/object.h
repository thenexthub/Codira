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

#ifndef ABCKIT_WRAPPER_OBJECT_H
#define ABCKIT_WRAPPER_OBJECT_H

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <any>
#include "err.h"
#include "visitor/child_visitor.h"

namespace abckit_wrapper {

class Module;
class Namespace;
class Method;
class Field;
class Class;
class AnnotationInterface;

using Child = std::variant<Namespace *, Method *, Field *, Class *, AnnotationInterface *>;

/**
 * @brief Object
 *
 * abc file object
 */
class Object {
public:
    /**
     * Object Destructor
     */
    virtual ~Object() = default;

    /**
     * @brief init sub objects
     *
     * @return AbckitWrapperErrorCode
     */
    virtual AbckitWrapperErrorCode Init();

    /**
     * @brief Get object fully qualified name
     * @return object fullName
     */
    [[nodiscard]] std::string GetFullyQualifiedName() const;

    /**
     * @brief Get object raw name
     * @return object raw name
     */
    virtual std::string GetName() const = 0;

    /**
     * @brief Set object name
     * @param name object name
     * @return `true` for success
     */
    virtual bool SetName(const std::string &name) = 0;

    /**
     * @brief Get object package name
     * @return object package name
     */
    std::string GetPackageName() const;

    /**
     * @brief Object children accept visit
     * @param visitor ChildVisitor
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool ChildrenAccept(ChildVisitor &visitor);

    /**
     * object's owing module
     */
    std::optional<Module *> owningModule_ = std::nullopt;
    /**
     * object's owing namespace
     */
    std::optional<Namespace *> owningNamespace_ = std::nullopt;
    /**
     * object's parent object
     */
    std::optional<Object *> parent_ = std::nullopt;
    /**
     * object's directly child
     */
    std::unordered_map<std::string, Child> children_;
    /**
     * object's data for visit
     */
    std::any data_;
};

/**
 * @brief SingleImplObject
 * @tparam T abckit::core object type
 */
template <typename T>
class SingleImplObject {
public:
    /**
     * @brief SingleImplObject Constructor
     * @param impl abckit::core object
     */
    explicit SingleImplObject(T impl) : impl_(std::move(impl)) {}

    /**
     * @brief Get ArkTS impl
     * @tparam ArkTsType arkts impl type
     * @return arktsImpl
     */
    template <typename ArkTsType>
    ArkTsType GetArkTsImpl() const
    {
        return ArkTsType(this->impl_);
    }

    /**
     * @brief Get object raw name
     * @return object raw name
     */
    [[nodiscard]] std::string GetObjectName() const
    {
        return this->impl_.GetName();
    }

    /**
     * @brief Set object raw name
     * @tparam ArkTsType arkts impl type
     * @param name name to set
     * @return `true` for success
     */
    template <typename ArkTsType>
    [[nodiscard]] bool SetObjectName(const std::string &name) const
    {
        return GetArkTsImpl<ArkTsType>().SetName(name);
    }

    /**
     * abckit::core object impl
     */
    T impl_;
};

/**
 * @brief MultiImplObject
 * @tparam Args abckit::core object types
 */
template <typename... Args>
class MultiImplObject {
public:
    /**
     * @brief MultiImplObject Constructor
     * @param impl abckit::core object
     */
    explicit MultiImplObject(std::variant<Args...> impl) : impl_(std::move(impl)) {}

    /**
     * @brief Get core impl object
     * @tparam CoreType abckit::core object type
     * @return abckit::core object, if not exist return nullptr
     */
    template <typename CoreType>
    const CoreType *GetCoreImpl() const
    {
        if (auto coreImpl = std::get_if<CoreType>(&this->impl_)) {
            return coreImpl;
        }

        return nullptr;
    }

    /**
     * @brief Get arkts impl object
     * @tparam CoreType abckit::core object type
     * @tparam ArkTsType abckit::arkts object type
     * @return abckit::arkts object if not exists return std::nullopt
     */
    template <typename CoreType, typename ArkTsType>
    std::optional<ArkTsType> GetArkTsImpl() const
    {
        auto coreImpl = this->GetCoreImpl<CoreType>();
        if (coreImpl == nullptr) {
            return std::nullopt;
        }

        return ArkTsType(*coreImpl);
    }

    /**
     * @brief Get object raw name
     * @return object raw name
     */
    [[nodiscard]] std::string GetObjectName() const
    {
        return std::visit([](const auto &object) { return object.GetName(); }, this->impl_);
    }

    /**
     * abckit::core object impl
     */
    std::variant<Args...> impl_;
};

/**
 * @brief ObjectPool
 *
 * store all the objects of the abc file
 */
class ObjectPool {
public:
    /**
     * @brief Add object to objectPool, object's full name must be unique
     * @param object object unique ptr
     */
    void Add(std::unique_ptr<Object> &&object);

    /**
     * @brief Get object by object full name
     * @param objectName object full name
     * @return object ptr
     */
    std::optional<Object *> Get(const std::string &objectName) const;

    /**
     * @brief Clear object's visit data
     */
    void ClearObjectsData();

private:
    std::unordered_map<std::string, std::unique_ptr<Object>> objects_;
};

}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_OBJECT_H
