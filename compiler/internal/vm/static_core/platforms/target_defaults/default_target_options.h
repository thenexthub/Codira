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

#ifndef PLATFORMS_TARGET_DEFAULTS_DEFAULT_TARGET_OPTIONS_H
#define PLATFORMS_TARGET_DEFAULTS_DEFAULT_TARGET_OPTIONS_H

#include <cstdint>
#include <map>
#include <string>
#include <functional>

#include "libarkbase/macros.h"

namespace ark::default_target_options {

using TargetGetterCallback = std::function<std::string()>;

std::string GetTargetString();

template <class ModelMap>
static typename ModelMap::mapped_type GetTargetSpecificOptionValue(const ModelMap &modelMap,
                                                                   const TargetGetterCallback &getTargetString)
{
    std::string model = getTargetString();
    if (modelMap.count(model) != 0) {
        return modelMap.at(model);
    }
    if (modelMap.count("default") != 0) {
        return modelMap.at("default");
    }
    UNREACHABLE();
}

bool GetCoverageEnable();

std::string GetCodeCoverageOutput();

}  // namespace ark::default_target_options

#endif  // PLATFORMS_TARGET_DEFAULTS_DEFAULT_TARGET_OPTIONS_H
