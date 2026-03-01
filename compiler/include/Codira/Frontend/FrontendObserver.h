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

#ifndef CODIRA_FRONTEND_FRONTENDOBSERVER_H
#define CODIRA_FRONTEND_FRONTENDOBSERVER_H

#include <algorithm>
#include <vector>

#include "Codira/Frontend/CompilerInstance.h"

namespace Codira {
/// A simple observer of frontend actions.
class FrontendObserver {
public:
    FrontendObserver() = default;
    virtual ~FrontendObserver() = default;

    /// An event, triggerd when the frontend has parsed AST.
    virtual void ParsedAST(CompilerInstance& instance) = 0;
};

/// List of FrontendObserver.
class MultiFrontendObserver : public FrontendObserver {
public:
    /// Make a clone of \ref observer and register it.
    void Add(FrontendObserver* observer) { observers.push_back(std::unique_ptr<FrontendObserver>(observer)); }

    /// An event, triggerd when the frontend has parsed AST.
    void ParsedAST(CompilerInstance& instance) override
    {
        std::for_each(observers.begin(), observers.end(),
            [&instance](const auto& observer) { observer->ParsedAST(instance); });
    };

private:
    std::vector<std::unique_ptr<FrontendObserver>> observers;
};
} // namespace Codira

#endif // CODIRA_FRONTEND_FRONTENDOBSERVER_H
