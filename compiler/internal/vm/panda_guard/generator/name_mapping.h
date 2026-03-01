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

#ifndef PANDA_GUARD_GENERATOR_NAME_MAPPING_H
#define PANDA_GUARD_GENERATOR_NAME_MAPPING_H

#include <memory>

#include "configs/guard_name_cache.h"
#include "configs/guard_options.h"
#include "name_generator.h"

namespace panda::guard {

class NameMapping {
public:
    explicit NameMapping(const std::shared_ptr<NameCache> &nameCache, std::shared_ptr<GuardOptions> options = nullptr);

    std::string GetName(const std::string &origin, bool generateNew = true);

    std::string GetFileName(const std::string &origin, bool generateNew = true);

    std::string GetFilePath(const std::string &origin, bool generateNew = true);

    void AddReservedNames(const std::set<std::string> &nameList) const;

    void AddNameMapping(const std::string &name);

    void AddNameMapping(const std::string &origin, const std::string &name);

    void AddNameMapping(const std::set<std::string> &nameList);

    void AddFileNameMapping(const std::string &name);

    void AddFileNameMapping(const std::set<std::string> &nameList);

private:
    std::unique_ptr<NameGenerator> fileNameGenerator_;

    std::unique_ptr<NameGenerator> nameGenerator_;

    std::map<std::string, std::string> fileNameMapping_ {};

    std::map<std::string, std::string> nameMapping_ {};

    std::shared_ptr<GuardOptions> options_;
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_GENERATOR_NAME_MAPPING_H
