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

#ifndef LSPSERVER_URI_H
#define LSPSERVER_URI_H

#include <string>
#include <tuple>

/**
 * According to the language service protocol to create structure
 * see https://microsoft.github.io/language-server-protocol/specifications/specification-3-16/#baseProtocol
 */
namespace ark {
constexpr long URI_SECOND_POS = 2;
constexpr unsigned int HEXADECIMAL = 16;
class URI {
public:
    URI(const std::string &scheme, const std::string &authority, const std::string &body);
    ~URI() {}

    std::string ToString() const;

    static URI Parse(const std::string& origUri);

    static std::string Resolve(const URI &u);

    static std::string Resolve(const std::string &fileURI);

    friend bool operator==(const URI &lhs, const URI &rhs)
    {
        return std::tie(lhs.scheme, lhs.authority, lhs.body) ==
               std::tie(rhs.scheme, rhs.authority, rhs.body);
    }

    friend bool operator<(const URI &lhs, const URI &rhs)
    {
        return std::tie(lhs.scheme, lhs.authority, lhs.body) <
               std::tie(rhs.scheme, rhs.authority, rhs.body);
    }

    static std::string GetAbsolutePath(std::string bodyPath);

    static URI URIFromAbsolutePath(const std::string absolutePath);

private:
    URI() = default;
    std::string scheme;
    std::string authority;
    std::string body;
};
} // namespace ark

#endif // LSPSERVER_URI_H
