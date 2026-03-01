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

#ifndef API_SCANNER_H
#define API_SCANNER_H

#include <string>
#include <unordered_map>
#include <vector>

#include "helpers/visit_helper/visit_helper-inl.h"
#include "libabckit/c/abckit.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/isa/isa_dynamic.h"

struct ApiInfo {
    std::string source;
    std::string objName;
    std::string propName;
};

struct UsageInfo {
    std::string source;
    std::string funcName;
    size_t idx;
};

using ApiUsageMap = std::unordered_map<size_t, std::vector<UsageInfo>>;

using SuspectsMap = std::unordered_map<AbckitCoreImportDescriptor *, size_t>;

class ApiScanner {
public:
    ApiScanner(enum AbckitApiVersion version, AbckitFile *file, std::vector<ApiInfo> apiList);

    const ApiUsageMap &GetApiUsages()
    {
        CollectApiUsages();
        return apiUsages_;
    }

    std::string ApiUsageMapGetString() const;

    bool IsUsagesEqual(const std::vector<UsageInfo> &otherUsages) const;

private:
    void CollectApiUsages();

    void CollectUsageInMethod(AbckitCoreFunction *method, SuspectsMap &suspects);

    std::string GetString(AbckitString *str) const;

    void AddApiUsage(size_t apiIndex, const UsageInfo &usage)
    {
        auto iter = apiUsages_.find(apiIndex);
        if (iter == apiUsages_.end()) {
            std::vector<UsageInfo> usages;
            usages.emplace_back(usage);
            apiUsages_.emplace(apiIndex, usages);
        } else {
            iter->second.emplace_back(usage);
        }
    }
    std::string GetMethodName(AbckitCoreFunction *method);
    bool GetSuspects(AbckitCoreModule *mod, SuspectsMap &suspects);
    bool IsLoadApi(AbckitCoreImportDescriptor *id, size_t apiIndex, AbckitInst *inst);

private:
    const AbckitApi *impl_ = nullptr;
    const AbckitInspectApi *implI_ = nullptr;
    const AbckitGraphApi *implG_ = nullptr;
    const AbckitIsaApiDynamic *dynG_ = nullptr;
    AbckitFile *file_ = nullptr;
    VisitHelper vh_;
    std::vector<ApiInfo> apiList_;
    ApiUsageMap apiUsages_;
};

#endif /* API_SCANNER_H */
