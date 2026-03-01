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

#ifndef CPP_ABCKIT_LITERAL_ARRAY_H
#define CPP_ABCKIT_LITERAL_ARRAY_H

#include "base_classes.h"
#include "config.h"
#include "value.h"
#include "literal.h"
#include <functional>

namespace abckit {

/**
 * @brief LiteralArray
 */
class LiteralArray : public ViewInResource<AbckitLiteralArray *, const File *> {
    /// @brief abckit::File
    friend class abckit::File;
    /// @brief abckit::Literal
    friend class abckit::Literal;
    /// @brief abckit::Instruction
    friend class abckit::Instruction;
    /// @brief abckit::DefaultHash<LiteralArray>
    friend class abckit::DefaultHash<LiteralArray>;
    /// @brief abckit::DynamicIsa
    friend class abckit::DynamicIsa;
    /// @brief abckit::Value
    friend class abckit::Value;

public:
    /**
     * @brief Construct a new Literal Array object
     * @param other
     */
    LiteralArray(const LiteralArray &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return LiteralArray&
     */
    LiteralArray &operator=(const LiteralArray &other) = default;

    /**
     * @brief Construct a new Literal Array object
     * @param other
     */
    LiteralArray(LiteralArray &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return LiteralArray&
     */
    LiteralArray &operator=(LiteralArray &&other) = default;

    /**
     * @brief Destroy the Literal Array object
     *
     */
    ~LiteralArray() override = default;

    /**
     * @brief Enumerates elements of the literal array, invoking callback `cb` for each of it's
     * elements.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if 'cb' is false.
     */
    bool EnumerateElements(const std::function<bool(Literal)> &cb) const;

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
    LiteralArray(AbckitLiteralArray *lita, const ApiConfig *conf, const File *file) : ViewInResource(lita), conf_(conf)
    {
        SetResource(file);
    };
    const ApiConfig *conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_LITERAL_ARRAY_H
