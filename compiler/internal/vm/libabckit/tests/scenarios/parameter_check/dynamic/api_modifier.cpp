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

#include "api_modifier.h"
#include "helpers/visit_helper/visit_helper-inl.h"
#include "libabckit/c/metadata_core.h"

#include <iostream>
#include <cstring>

void ApiModifier::Run()
{
    visitor_.EnumerateModules([&](AbckitCoreModule *mod) {
        AbckitCoreImportDescriptor *impDescriptor = GetImportDescriptor(mod);
        if (impDescriptor == nullptr) {
            return;
        }
        visitor_.EnumerateModuleFunctions(mod, [&](AbckitCoreFunction *method) {
            ModifyFunction(method, impDescriptor);
            return true;
        });
    });
}

AbckitCoreImportDescriptor *ApiModifier::GetImportDescriptor(AbckitCoreModule *module)
{
    AbckitCoreImportDescriptor *impDescriptor = nullptr;
    visitor_.EnumerateModuleImports(module, [&](AbckitCoreImportDescriptor *id) {
        auto importName = visitor_.GetString(implI_->importDescriptorGetName(id));
        auto *importedModule = implI_->importDescriptorGetImportedModule(id);
        auto source = visitor_.GetString(implI_->moduleGetName(importedModule));
        if (source != methodInfo_.path) {
            return false;
        }
        if (importName == methodInfo_.className) {
            impDescriptor = id;
            return true;
        }
        return false;
    });
    return impDescriptor;
}

void ApiModifier::ModifyFunction(AbckitCoreFunction *method, AbckitCoreImportDescriptor *id)
{
    visitor_.EnumerateFunctionInsts(method, [&](AbckitInst *inst) {
        auto subclassMethod = GetSubclassMethod(id, inst);
        if (subclassMethod != nullptr) {
            AddParamChecker(subclassMethod);
        }
    });
}

AbckitCoreFunction *ApiModifier::GetSubclassMethod(AbckitCoreImportDescriptor *id, AbckitInst *inst)
{
    if (dynG_->iGetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR) {
        return nullptr;
    }

    if (dynG_->iGetImportDescriptor(inst) != id) {
        return nullptr;
    }

    AbckitCoreFunction *foundMethod = nullptr;
    visitor_.EnumerateInstUsers(inst, [&](AbckitInst *user) {
        if (dynG_->iGetOpcode(user) == ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER) {
            auto method = implG_->iGetFunction(user);
            auto klass = implI_->functionGetParentClass(method);
            foundMethod = GetMethodToModify(klass);
        }
    });

    return foundMethod;
}

AbckitCoreFunction *ApiModifier::GetMethodToModify(AbckitCoreClass *klass)
{
    AbckitCoreFunction *foundMethod = nullptr;
    visitor_.EnumerateClassMethods(klass, [&](AbckitCoreFunction *method) {
        auto name = visitor_.GetString(implI_->functionGetName(method));
        if (name == methodInfo_.methodName) {
            foundMethod = method;
            return true;
        }
        return false;
    });
    return foundMethod;
}

void ApiModifier::AddParamChecker(AbckitCoreFunction *method)
{
    AbckitGraph *graph = implI_->createGraphFromFunction(method);

    visitor_.TransformMethod(method, [&](AbckitFile *file, AbckitCoreFunction *method) {
        AbckitBasicBlock *startBB = implG_->gGetStartBasicBlock(graph);
        AbckitInst *idx = implG_->bbGetLastInst(startBB);
        AbckitInst *arr = implG_->iGetPrev(idx);

        std::vector<AbckitBasicBlock *> succBBs;
        implG_->bbVisitSuccBlocks(startBB, &succBBs, [](AbckitBasicBlock *succBasicBlock, void *d) {
            auto *succs = reinterpret_cast<std::vector<AbckitBasicBlock *> *>(d);
            succs->emplace_back(succBasicBlock);
            return true;
        });

        AbckitString *str = implM_->createString(file, "length", strlen("length"));

        AbckitInst *constant = implG_->gFindOrCreateConstantI32(graph, -1);
        AbckitInst *arrLength = dynG_->iCreateLdobjbyname(graph, arr, str);

        AbckitBasicBlock *trueBB = succBBs[0];
        implG_->bbDisconnectSuccBlock(startBB, ABCKIT_TRUE_SUCC_IDX);
        AbckitBasicBlock *falseBB = implG_->bbCreateEmpty(graph);
        implG_->bbAppendSuccBlock(falseBB, implG_->gGetEndBasicBlock(graph));
        implG_->bbAddInstBack(falseBB, dynG_->iCreateReturn(graph, constant));
        AbckitBasicBlock *ifBB = implG_->bbCreateEmpty(graph);
        AbckitInst *intrinsicGreatereq = dynG_->iCreateGreatereq(graph, arrLength, idx);
        AbckitInst *ifInst = dynG_->iCreateIf(graph, intrinsicGreatereq, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ);
        implG_->bbAddInstBack(ifBB, arrLength);
        implG_->bbAddInstBack(ifBB, intrinsicGreatereq);
        implG_->bbAddInstBack(ifBB, ifInst);
        implG_->bbAppendSuccBlock(startBB, ifBB);
        implG_->bbAppendSuccBlock(ifBB, trueBB);
        implG_->bbAppendSuccBlock(ifBB, falseBB);

        implM_->functionSetGraph(method, graph);
        impl_->destroyGraph(graph);
    });
}
