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

#ifndef CODIRA_CHIR_CHECKER_UNREACHABLE_BRANCH_CHECK_H
#define CODIRA_CHIR_CHECKER_UNREACHABLE_BRANCH_CHECK_H

#include "Codira/CHIR/Analysis/AnalysisWrapper.h"
#include "Codira/CHIR/Analysis/ConstAnalysis.h"
#include "Codira/CHIR/Analysis/Utils.h"
#include "Codira/CHIR/DiagAdapter.h"
#include "Codira/CHIR/Package.h"
#include "Codira/Utils/TaskQueue.h"

namespace Codira::CHIR {

class UnreachableBranchCheck {
public:
    using ConstAnalysisWrapper = AnalysisWrapper<ConstAnalysis, ConstDomain>;
    explicit UnreachableBranchCheck(
        ConstAnalysisWrapper* constAnalysisWrapper, DiagAdapter& diag, const std::string& packageName);

    void RunOnPackage(const Package& package, size_t threadNum);

    void RunOnFunc(const Ptr<Func> func);

private:
    void PrintWarning(const Terminator& node, Block& block, std::set<Block*>& hasProcessed, bool isRecursive = false);

    DiagAdapter& diag;
    ConstAnalysisWrapper* analysisWrapper;

    const std::string& currentPackageName;
};

} // namespace Codira::CHIR

#endif
