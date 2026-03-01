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
#ifndef ABCKIT_SUBCLASSES_SCANNER_H
#define ABCKIT_SUBCLASSES_SCANNER_H

#include "libabckit/c/isa/isa_dynamic.h"
#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "helpers/visit_helper/visit_helper.h"
#include "helpers/visit_helper/visit_helper-inl.h"
#include "libabckit/c/metadata_core.h"

struct ClassInfo {
    std::string path;
    std::string className;
};

class SubclassesScanner {
public:
    SubclassesScanner(enum AbckitApiVersion version, const AbckitApi *impl, AbckitFile *file,
                      std::vector<ClassInfo> baseClasses)
        : file_(file), baseClasses_ {std::move(baseClasses)}
    {
        implI_ = AbckitGetInspectApiImpl(version);
        implG_ = AbckitGetGraphApiImpl(version);
        dynG_ = AbckitGetIsaApiDynamicImpl(version);
        visitor_ = VisitHelper(file_, impl, implI_, implG_, dynG_);
    }

    const std::vector<ClassInfo> &GetSubclasses()
    {
        return subClasses_;
    }

    void Run();

    bool IsEqualsSubClasses(const std::vector<ClassInfo> &otherSubClasses);

private:
    void CollectSubClasses(AbckitCoreFunction *method,
                           const std::vector<std::pair<AbckitCoreImportDescriptor *, size_t>> &impDescrs);

    bool IsLoadApi(AbckitCoreImportDescriptor *id, AbckitInst *inst, ClassInfo &subclassInfo);

    void GetImportDescriptors(AbckitCoreModule *mod,
                              std::vector<std::pair<AbckitCoreImportDescriptor *, size_t>> &importDescriptors);

private:
    const AbckitInspectApi *implI_ = nullptr;
    const AbckitGraphApi *implG_ = nullptr;
    const AbckitIsaApiDynamic *dynG_ = nullptr;
    AbckitFile *file_ = nullptr;
    VisitHelper visitor_;
    std::vector<ClassInfo> subClasses_;
    std::vector<ClassInfo> baseClasses_;
};

#endif  // ABCKIT_SUBCLASSES_SCANNER_H
