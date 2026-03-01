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

#ifndef CPP_ABCKIT_LITERAL_H
#define CPP_ABCKIT_LITERAL_H

#include <string>
#include "base_classes.h"
#include "core/function.h"
#include "literal_array.h"

namespace abckit {

/**
 * @brief Literal
 */
class Literal : public ViewInResource<AbckitLiteral *, const File *> {
    /// @brief abckit::File
    friend class abckit::File;
    /// @brief abckit::LiteralArray
    friend class abckit::LiteralArray;
    /// @brief abckit::DefaultHash<Literal>
    friend class abckit::DefaultHash<Literal>;

public:
    /**
     * @brief Construct a new Literal object
     * @param other
     */
    Literal(const Literal &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Literal&
     */
    Literal &operator=(const Literal &other) = default;

    /**
     * @brief Construct a new Literal object
     * @param other
     */
    Literal(Literal &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Literal&
     */
    Literal &operator=(Literal &&other) = default;

    /**
     * @brief Destroy the Literal object
     */
    ~Literal() override = default;

    /**
     * @brief Get the Literal Array object
     * @return abckit::LiteralArray
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    abckit::LiteralArray GetLiteralArray() const;

    /**
     * @brief Returns binary file that the LiteralArray is a part of.
     * @return pointer to File.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    const File *GetFile() const
    {
        return GetResource();
    };

    /**
     * @brief Returns boolean value for Literal.
     * @return Boolean value of the Literal.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool GetBool() const;

    /**
     * @brief Returns byte value for Literal.
     * @return Byte value of the Literal.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    uint8_t GetU8() const;

    /**
     * @brief Returns short value for Literal.
     * @return Short value of the Literal.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    uint16_t GetU16() const;

    /**
     * @brief Returns integer value for Literal.
     * @return Short value of the Literal.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    uint32_t GetU32() const;

    /**
     * @brief Returns long value for Literal.
     * @return Short value of the Literal.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    uint64_t GetU64() const;

    /**
     * @brief Returns method affiliate value for the Literal.
     * @return uint16_t containing method affiliate оffset that is stored in the Literal.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    uint16_t GetMethodAffiliate() const;

    /**
     * @brief Returns float value that is stored in Literal.
     * @return float value for Literal.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    float GetFloat() const;

    /**
     * @brief Returns double value that is stored in Literal.
     * @return float value for Literal.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    double GetDouble() const;

    /**
     * @brief Returns string value for Literal.
     * @return std::string that holds the string value of the Literal.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::string GetString() const;

    /**
     * @brief Returns method descriptor that is stored in Literal.
     * @return std::string that holds the string value of the Literal.
     * @note Allocates
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::string GetMethod() const;

    /**
     * @brief Returns type tag that the Literal has.
     * @return Tag of the Literal.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    enum AbckitLiteralTag GetTag() const;

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
    Literal(AbckitLiteral *lit, const ApiConfig *conf, const File *file) : ViewInResource(lit), conf_(conf)
    {
        SetResource(file);
    };
    const ApiConfig *conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_LITERAL_H
