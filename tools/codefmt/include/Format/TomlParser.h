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

#ifndef CODEFMT_TOMLPARSER_H
#define CODEFMT_TOMLPARSER_H

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <optional>

namespace Codira::Format {
class TomlParser {
public:
    using ValueType = std::variant<int, bool, std::string>;

    bool ReadFile(const std::string& filename);
    std::optional<ValueType> GetValue(const std::string& key) const;

private:
    std::map<std::string, std::optional<ValueType>> data;
    std::string Trim(const std::string& str);
    std::optional<ValueType> ParseValue(const std::string& value);
};
} // namespace Codira::Format
#endif // CODEFMT_TOMLPARSER_H
