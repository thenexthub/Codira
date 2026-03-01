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

#ifndef ABCKIT_API_MODIFIER_H
#define ABCKIT_API_MODIFIER_H

#include <vector>
#include "libabckit/c/abckit.h"
#include "helpers/visit_helper/visit_helper.h"

struct MethodInfo {
    std::string path;
    std::string className;
    std::string methodName;
};

class ApiModifier {
public:
    ApiModifier(enum AbckitApiVersion version, AbckitFile *file, const AbckitApi *impl, MethodInfo methodInfo)
        : impl_(impl), file_(file), methodInfo_(std::move(methodInfo))
    {
        implM_ = AbckitGetModifyApiImpl(version);
        implI_ = AbckitGetInspectApiImpl(version);
        implG_ = AbckitGetGraphApiImpl(version);
        dynG_ = AbckitGetIsaApiDynamicImpl(version);
        visitor_ = VisitHelper(file_, impl_, implI_, implG_, dynG_);
    }

    void Run();

    AbckitFile *GetFile()
    {
        return file_;
    }

private:
    AbckitCoreImportDescriptor *GetImportDescriptor(AbckitCoreModule *module);
    void ModifyFunction(AbckitCoreFunction *method, AbckitCoreImportDescriptor *id);
    void AddParamChecker(AbckitCoreFunction *method);
    AbckitCoreFunction *GetSubclassMethod(AbckitCoreImportDescriptor *id, AbckitInst *inst);
    AbckitCoreFunction *GetMethodToModify(AbckitCoreClass *klass);

private:
    const AbckitApi *impl_ = nullptr;
    const AbckitModifyApi *implM_ = nullptr;
    const AbckitInspectApi *implI_ = nullptr;
    const AbckitGraphApi *implG_ = nullptr;
    const AbckitIsaApiDynamic *dynG_ = nullptr;
    AbckitFile *file_ = nullptr;
    VisitHelper visitor_;
    MethodInfo methodInfo_;
};

#endif  // ABCKIT_API_MODIFIER_H
