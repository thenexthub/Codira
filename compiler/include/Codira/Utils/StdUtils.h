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

#ifndef CODIRA_UTILS_STDUTILS_H
#define CODIRA_UTILS_STDUTILS_H

#include <optional>
#include <string>

namespace Codira {
constexpr int STOINT_BASE{10};
std::optional<int> Stoi(const std::string& s, int base = STOINT_BASE);
std::optional<long> Stol(const std::string& s, int base = STOINT_BASE);
std::optional<unsigned long> Stoul(const std::string& s, int base = STOINT_BASE);
std::optional<long long> Stoll(const std::string& s, int base = STOINT_BASE);
std::optional<unsigned long long> Stoull(const std::string& s, int base = STOINT_BASE);
std::optional<double> Stod(const std::string& s);
std::optional<long double> Stold(const std::string& s);
}
#endif
