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

#ifndef CPP_ABCKIT_VALUE_H
#define CPP_ABCKIT_VALUE_H

#include "base_classes.h"

namespace abckit {

/**
 * @brief Value
 */
class Value : public ViewInResource<AbckitValue *, const File *> {
    /// @brief abckit::File
    friend class abckit::File;
    /// @brief abckit::core::Annotation
    friend class core::Annotation;
    /// @brief abckit::core::Annotation
    friend class core::AnnotationElement;
    /// @brief abckit::core::AnnotationInterfaceField
    friend class core::AnnotationInterfaceField;
    /// @brief abckit::arkts::Annotation
    friend class arkts::Annotation;
    /// @brief abckit::DefaultHash<Value>
    friend class abckit::DefaultHash<Value>;
    /// @brief arkts::AnnotationInterface
    friend class arkts::AnnotationInterface;
    /// @brief abckit::core::ClassField
    friend class core::ClassField;
    /// @brief abckit::core::ModuleField
    friend class core::ModuleField;
    /// @brief arkts::ModuleField
    friend class arkts::ModuleField;
    /// @brief arkts::ClassField
    friend class arkts::ClassField;
    /// @brief arkts::Class
    friend class arkts::Class;
    /// @brief arkts::Module
    friend class arkts::Module;
    /// @brief arkts::Interface
    friend class arkts::Interface;
    /// @brief arkts::Enum
    friend class arkts::Enum;
    /// @brief abckit::core::EnumField
    friend class core::EnumField;

public:
    /**
     * @brief Construct a new Value object
     * @param other
     */
    Value(const Value &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Value&
     */
    Value &operator=(const Value &other) = default;

    /**
     * @brief Construct a new Value object
     * @param other
     */
    Value(Value &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Value&
     */
    Value &operator=(Value &&other) = default;

    /**
     * @brief Destroy the Value object
     */
    ~Value() override = default;

    /**
     * @brief Returns boolean value that value holds.
     * @return Boolean value that is stored in the value.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if value is NULL.
     * @note Set `ABCKIT_STATUS_TODO` error if value type id differs from `ABCKIT_TYPE_ID_U1`.
     */
    bool GetU1() const;

    /**
     * @brief Returns int value that value holds.
     * @return Int value that is stored in the value.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if value is NULL.
     * @note Set `ABCKIT_STATUS_TODO` error if value type id differs from `ABCKIT_TYPE_ID_INT`.
     */
    int GetInt() const;

    /**
     * @brief Returns double value that value holds.
     * @return Double value that is stored in the value.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if value is NULL.
     * @note Set `ABCKIT_STATUS_TODO` error if value type id differs from `ABCKIT_TYPE_ID_F64`.
     */
    double GetDouble() const;

    /**
     * @brief Returns binary file that the Value is a part of.
     * @return Pointer to the file that contains value.
     */
    const File *GetFile() const;

    /**
     * @brief Returns type that value has.
     * @return Pointer to the `AbckitType` that corresponds to provided value.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if value is NULL.
     */
    Type GetType() const;

    /**
     * @brief Returns literal array that value holds.
     * @return Pointer to the `AbckitLiteralArray`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if value is NULL.
     * @note Set `ABCKIT_STATUS_TODO` error if value type id differs from `ABCKIT_TYPE_ID_LITERALARRAY`.
     */
    abckit::LiteralArray GetLiteralArray() const;

    /**
     * @brief Returns string that value holds.
     * @return `std::string`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if value is NULL.
     * @note Set `ABCKIT_STATUS_TODO` error if value type id differs from `ABCKIT_TYPE_ID_LITERALARRAY`.
     */
    std::string GetString() const;

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
    explicit Value(AbckitValue *val, const ApiConfig *conf, const File *file) : ViewInResource(val), conf_(conf)
    {
        SetResource(file);
    };
    const ApiConfig *conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_VALUE_H
