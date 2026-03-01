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

#ifndef GUARD_NAME_MAPPING_NAME_MAPPING_MANAGER_H
#define GUARD_NAME_MAPPING_NAME_MAPPING_MANAGER_H

#include "order_name_generator.h"

#include <memory>
#include <unordered_map>

#include "error.h"

namespace ark::guard {

enum class NameGeneratorType { ORDER };

/**
 * @brief NameMapping
 */
class NameMapping {
public:
    /**
     * @brief Create name generator
     * @param type name generator type
     */
    ErrorCode Create(NameGeneratorType type);

    /**
     * @brief Add name to used name list
     * @param usedName used name
     */
    void AddUsedNameList(const std::string &usedName) const;

    /**
     * @brief Add nameList to used name list
     * @param usedNameList used name list
     */
    void AddUsedNameList(const std::unordered_set<std::string> &usedNameList) const;

    /**
     * @brief Get obf name with origin name
     * @param originName: origin name
     * @return std::string obf name
     */
    std::string GetName(const std::string &originName);

private:
    std::unique_ptr<NameGenerator> nameGenerator_;

    std::unordered_map<std::string, std::string> nameMapping_;
};

/**
 * @brief NameMappingManager
 */
class NameMappingManager {
public:
    /**
     * @brief Get the name mapping from the descriptor NameMapping using the description, and create it if not available
     * @param descriptor descriptor
     * @param type name generator type
     * @return std::shared_ptr<NameMapping>
     */
    std::shared_ptr<NameMapping> GetNameMapping(const std::string &descriptor,
                                                NameGeneratorType type = NameGeneratorType::ORDER);

private:
    /**
     * store descriptor name mapping
     */
    std::unordered_map<std::string, std::shared_ptr<NameMapping>> descriptorNameMapping_;
};
}  // namespace ark::guard

#endif  // GUARD_NAME_MAPPING_NAME_MAPPING_MANAGER_H
