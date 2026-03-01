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

#ifndef CPP_ABCKIT_CORE_FUNCTION_PARAM_H
#define CPP_ABCKIT_CORE_FUNCTION_PARAM_H

#include "../base_classes.h"

namespace abckit::core {

/**
 * @brief FunctionParam
 */
class FunctionParam : public ViewInResource<AbckitCoreFunctionParam *, const File *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class core::Function;
    /// @brief to access private constructor
    friend class abckit::File;
    /// @brief abckit::DefaultHash<FunctionParam>
    friend abckit::DefaultHash<FunctionParam>;

protected:
    /// @brief Core API View type
    using CoreViewT = FunctionParam;

public:
    /**
     * @brief Construct a new FunctionParam object
     * @param other
     */
    FunctionParam(const FunctionParam &other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

    /**
     * @brief Constructor
     * @param other
     * @return FunctionParam&
     */
    FunctionParam &operator=(const FunctionParam &other) = default;

    /**
     * @brief Construct a new FunctionParam object
     * @param other
     */
    FunctionParam(FunctionParam &&other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

    /**
     * @brief Constructor
     * @param other
     * @return FunctionParam&
     */
    FunctionParam &operator=(FunctionParam &&other) = default;

    /**
     * @brief Destroy the FunctionParam object
     */
    ~FunctionParam() override = default;

    /**
     * @brief Returns type for FunctionParam.
     * @return abckit::Type
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Type GetType() const;

private:
    FunctionParam(AbckitCoreFunctionParam *param, const ApiConfig *conf, const File *file)
        : ViewInResource(param), conf_(conf)
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

#endif  // CPP_ABCKIT_CORE_FUNCTION_PARAM_H
