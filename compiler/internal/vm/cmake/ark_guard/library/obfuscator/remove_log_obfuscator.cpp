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

#include "remove_log_obfuscator.h"

#include <regex>

#include "abckit_wrapper/method.h"
#include "logger.h"

namespace {
const std::vector<std::regex> LOG_PATTERNS = {
    std::regex("^.*:std.core.Console;.*"),        std::regex("^debug:.*;std.core.Array;void;"),
    std::regex("^info:.*;std.core.Array;void;"),  std::regex("^warn:.*;std.core.Array;void;"),
    std::regex("^error:.*;std.core.Array;void;"), std::regex("^fatal:.*;std.core.Array;void;")};

bool IsCallLog(const abckit::Instruction &instruction)
{
    if (instruction.GetId() == 0 || !instruction.CheckIsCall()) {
        return false;
    }

    const auto &functionName = instruction.GetFunction().GetName();
    return std::any_of(LOG_PATTERNS.begin(), LOG_PATTERNS.end(),
                       [&](const std::regex &re) { return std::regex_match(functionName, re); });
}

bool RemoveConsoleLog(const abckit_wrapper::Method *method)
{
    const auto &function = method->GetArkTsImpl<abckit::core::Function>();
    const auto &arkTsFunction = method->GetArkTsImpl<abckit::arkts::Function>();
    if (function.IsExternal() || arkTsFunction.IsAbstract() || arkTsFunction.IsNative()) {
        return true;
    }
    auto graph = function.CreateGraph();
    bool isRemove = false;
    for (const auto &block : graph.GetBlocksRPO()) {
        for (auto &ins : block.GetInstructions()) {
            if (IsCallLog(ins)) {
                ins.Remove();
                isRemove = true;
            }
        }
    }
    if (isRemove) {
        LOG_I << "Remove console log for method: " << method->GetFullyQualifiedName();
        function.SetGraph(graph);
    }
    return true;
}
}  // namespace

bool ark::guard::RemoveLogObfuscator::Execute(abckit_wrapper::FileView &fileView)
{
    return fileView.ModulesAccept(this->Wrap<abckit_wrapper::LocalModuleFilter>());
}

bool ark::guard::RemoveLogObfuscator::VisitMethod(abckit_wrapper::Method *method)
{
    return RemoveConsoleLog(method);
}

bool ark::guard::RemoveLogObfuscator::Visit(abckit_wrapper::Module *module)
{
    return module->ChildrenAccept(*this);
}

bool ark::guard::RemoveLogObfuscator::VisitNamespace(abckit_wrapper::Namespace *ns)
{
    return ns->ChildrenAccept(*this);
}

bool ark::guard::RemoveLogObfuscator::VisitClass(abckit_wrapper::Class *clazz)
{
    return clazz->ChildrenAccept(*this);
}

bool ark::guard::RemoveLogObfuscator::VisitAnnotationInterface(abckit_wrapper::AnnotationInterface *ai)
{
    return true;
}

bool ark::guard::RemoveLogObfuscator::VisitField(abckit_wrapper::Field *field)
{
    return true;
}

