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

#ifndef GUARD_NAME_MAPPING_NAME_GENERATOR_H
#define GUARD_NAME_MAPPING_NAME_GENERATOR_H

#include <string>
#include <unordered_set>

namespace ark::guard {

/**
 * @brief NameGenerator
 */
class NameGenerator {
public:
    /**
     * @brief default constructor, enabling object creation without parameters
     */
    NameGenerator() = default;

    /**
     * @brief constructor, initializes the usedNameList_
     * @param usedNameList used name list
     */
    explicit NameGenerator(std::unordered_set<std::string> &&usedNameList) : usedNameList_(std::move(usedNameList)) {}

    /**
     * @brief NameGenerator default Destructor
     */
    virtual ~NameGenerator() = default;

    /**
     * @brief Generate Name
     */
    virtual std::string GenerateName() = 0;

    /**
     * @brief Add name to used name list
     * @param name used name
     */
    void AddUsedNameList(const std::string &name)
    {
        if (name.empty()) {
            return;
        }
        usedNameList_.reserve(usedNameList_.size() + 1);
        usedNameList_.emplace(name);
    }

    /**
     * @brief Add name to used name list
     * @param usedNameList used name list
     */
    void AddUsedNameList(const std::unordered_set<std::string> &usedNameList)
    {
        if (usedNameList.empty()) {
            return;
        }
        usedNameList_.reserve(usedNameList_.size() + usedNameList.size());
        usedNameList_.insert(usedNameList.begin(), usedNameList.end());
    }

    /**
     * @brief Check the name has been used
     * @param name name
     * @return bool true: used, false: not used
     */
    bool IsNameUsed(const std::string &name) const
    {
        return usedNameList_.find(name) != usedNameList_.end();
    }

protected:
    /**
     * @brief Store used names
     */
    std::unordered_set<std::string> usedNameList_ {};
};
}  // namespace ark::guard

#endif  // GUARD_NAME_MAPPING_NAME_GENERATOR_H
