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

/**
 * @file
 *
 * This file declares diagnostic related functions for Modules.
 */

#ifndef CODIRA_MODULES_DIAGS_H
#define CODIRA_MODULES_DIAGS_H

#include <string>

#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/Modules/ImportManager.h"

namespace Codira::Modules {
using namespace AST;
void WarnUselessImport(DiagnosticEngine& diag, const Range& importRange, const Decl& decl);
void WarnConflictImport(DiagnosticEngine& diag, const std::string& name, const Range& current, const Range& previous);
void WarnRepeatedFeatureName(DiagnosticEngine& diag, std::string& name, const Range& current, const Range& previous);
void DiagForNullPackageFeature(DiagnosticEngine& diag, const Range& current, Ptr<FeaturesDirective> refFeature);
void DiagForDifferentPackageFeatureConsistency(DiagnosticEngine& diag,
    Ptr<FeaturesDirective> feature, Ptr<FeaturesDirective> refFeature, bool hasAnno);
void DiagForDifferentPackageNames(DiagnosticEngine& diag,
    const std::map<std::pair<std::string, std::string>, std::pair<Position, bool>>& packageNamePosMap);
void DiagRootPackageModifier(DiagnosticEngine& diag, const PackageSpec& packageSpec);
} // namespace Codira::Modules

#endif
