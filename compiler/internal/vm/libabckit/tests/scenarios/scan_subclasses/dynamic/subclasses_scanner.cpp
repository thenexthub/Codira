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

#include "subclasses_scanner.h"
#include <string>

void SubclassesScanner::Run()
{
    visitor_.EnumerateModules([&](AbckitCoreModule *mod) {
        std::vector<std::pair<AbckitCoreImportDescriptor *, size_t>> impDescriptors;
        GetImportDescriptors(mod, impDescriptors);
        if (impDescriptors.empty()) {
            return;
        }
        visitor_.EnumerateModuleFunctions(
            mod, [&](AbckitCoreFunction *method) { CollectSubClasses(method, impDescriptors); });
    });
}

void SubclassesScanner::CollectSubClasses(AbckitCoreFunction *method,
                                          const std::vector<std::pair<AbckitCoreImportDescriptor *, size_t>> &impDescrs)
{
    visitor_.EnumerateFunctionInsts(method, [&](AbckitInst *inst) {
        for (const auto &[impDescr, idx] : impDescrs) {
            ClassInfo classInfo;
            if (IsLoadApi(impDescr, inst, classInfo)) {
                subClasses_.emplace_back(classInfo);
            }
        }
    });
}

bool SubclassesScanner::IsLoadApi(AbckitCoreImportDescriptor *id, AbckitInst *inst, ClassInfo &subclassInfo)
{
    if (dynG_->iGetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR) {
        return false;
    }

    if (dynG_->iGetImportDescriptor(inst) != id) {
        return false;
    }

    bool found = false;
    visitor_.EnumerateInstUsers(inst, [&](AbckitInst *user) {
        if (dynG_->iGetOpcode(user) == ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER) {
            auto method = implG_->iGetFunction(user);
            auto klass = implI_->functionGetParentClass(method);
            auto module = implI_->classGetModule(klass);
            subclassInfo.className = visitor_.GetString(implI_->classGetName(klass));
            subclassInfo.path = visitor_.GetString(implI_->moduleGetName(module));
            found = true;
        }
    });

    return found;
}

// CC-OFFNXT(G.INC.12-CPP) false positive. this function can not be made static.
void SubclassesScanner::GetImportDescriptors(
    AbckitCoreModule *mod, std::vector<std::pair<AbckitCoreImportDescriptor *, size_t>> &importDescriptors)
{
    visitor_.EnumerateModuleImports(mod, [&](AbckitCoreImportDescriptor *id) {
        auto importName = visitor_.GetString(implI_->importDescriptorGetName(id));
        auto *importedModule = implI_->importDescriptorGetImportedModule(id);
        auto source = visitor_.GetString(implI_->moduleGetName(importedModule));
        for (size_t i = 0; i < baseClasses_.size(); ++i) {
            const auto baseClass = baseClasses_[i];
            if (source != baseClass.path) {
                continue;
            }
            if (importName == baseClass.className) {
                importDescriptors.emplace_back(id, i);
            }
        }
    });
}

bool SubclassesScanner::IsEqualsSubClasses(const std::vector<ClassInfo> &otherSubClasses)
{
    for (auto &otherSubClass : otherSubClasses) {
        auto iter = std::find_if(subClasses_.begin(), subClasses_.end(), [&otherSubClass](const ClassInfo &classInfo) {
            return (otherSubClass.className == classInfo.className) && (otherSubClass.path == classInfo.path);
        });
        if (iter == subClasses_.end()) {
            return false;
        }
    }
    return true;
}
