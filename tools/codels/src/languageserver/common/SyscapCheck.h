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

#ifndef LSPSERVER_SYSCAPCHECK_H
#define LSPSERVER_SYSCAPCHECK_H

#include <unordered_map>
#include <unordered_set>
#include "Codira/AST/Node.h"

namespace ark {
using SysCapSet = std::unordered_set<std::string>;
class SyscapCheck {
public:
    SyscapCheck() = default;
    explicit SyscapCheck(const std::string& moduleName);
    void SetIntersectionSet(const std::string& moduleName);
    static void ParseCondition(const std::unordered_map<std::string, std::string>& passedWhenKeyValue);
    static void ParseJsonFile(const std::vector<uint8_t>& in);
    // if check node has syscap and not match, return false
    bool CheckSysCap(Ptr<Codira::AST::Node> node);
    bool CheckSysCap(Ptr<Codira::AST::Node> node, bool& matchSyscap) const;
    bool CheckSysCap(Ptr<Codira::AST::Decl> decl) const;
    bool CheckSysCap(const Codira::AST::Decl& decl) const;
    bool CheckSysCap(const std::string& syscapName);
    SysCapSet intersectionSet;
    static std::unordered_map<std::string, SysCapSet> module2SyscapsMap;
};

} // namespace ark

#endif // LSPSERVER_SYSCAPCHECK_H
