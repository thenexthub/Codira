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
 * This file implements diagnostics for modules.
 */

#include "ModulesDiag.h"

#include "Codira/AST/Match.h"
#include "Codira/Modules/ModulesUtils.h"

namespace Codira::Modules {
namespace {
bool IsSamePosition(const Position& pos1, const Position& pos2)
{
    return pos1 == pos2 && pos1.fileID == pos2.fileID;
}
} // namespace

void WarnUselessImport(DiagnosticEngine& diag, const Range& importRange, const Decl& decl)
{
    auto& name = decl.identifier;
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::package_shadowed_import, importRange, name);
    builder.AddNote(MakeRange(decl.identifier.Begin(), name), "'" + name + "' is declared here");
}

void WarnConflictImport(DiagnosticEngine& diag, const std::string& name, const Range& current, const Range& previous)
{
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::package_conflict_import, current, name);
    builder.AddNote(previous, "The previous was imported here");
}

void WarnRepeatedFeatureName(DiagnosticEngine& diag, std::string& name, const Range& current, const Range& previous)
{
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::feature_already_seen_name, current);
    builder.AddNote(previous, "feature '" + name + "' previously used here");
}

void DiagForNullPackageFeature(DiagnosticEngine& diag, const Range& current, const Ptr<FeaturesDirective> refFeature)
{
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::feature_null_declaration, current);
    builder.AddNote(
        MakeRange(refFeature->featuresSet->begin, refFeature->featuresSet->end),
        "perhaps you meant these features");
}

void DiagForDifferentPackageFeatureConsistency(DiagnosticEngine& diag, const Ptr<FeaturesDirective> feature,
    const Ptr<FeaturesDirective> refFeature, bool hasAnno)
{
    if (feature->annotations.empty() && hasAnno) {
        auto builder = diag.DiagnoseRefactor(DiagKindRefactor::parse_fail_expected_annotation,
            MakeRange(feature->featuresPos, feature->featuresPos + std::string("features").size()), "@NonProduct");
    } else {
        auto builder = diag.DiagnoseRefactor(DiagKindRefactor::feature_different_consistency,
            MakeRange(feature->featuresSet->begin, feature->featuresSet->end));
        builder.AddNote(
            MakeRange(refFeature->featuresSet->begin, refFeature->featuresSet->end),
            "perhaps you meant these features");
    }
}

void DiagForDifferentPackageNames(DiagnosticEngine& diag,
    const std::map<std::pair<std::string, std::string>, std::pair<Position, bool>>& packageNamePosMap)
{
    Position diagPosition;
    std::pair<std::string, std::string> diagPackageDecl;
    for (auto [pkgPair, pair] : packageNamePosMap) {
        if (pair.second) {
            diagPosition = pair.first;
            diagPackageDecl = pkgPair;
            break;
        }
    }
    auto builder = diag.DiagnoseRefactor(
        DiagKindRefactor::package_multiple_package_declarations, MakeRange(diagPosition, diagPackageDecl.first));
    uint8_t counter = 0;
    for (auto [pkgPair, pair] : packageNamePosMap) {
        // 2 is maximum number of diagnostic prints.
        if (counter >= 2) {
            break;
        }
        if (IsSamePosition(diagPosition, pair.first)) {
            continue;
        }
        if (pair.second) {
            builder.AddNote(MakeRange(pair.first, pkgPair.first),
                "another different package declaration '" + pkgPair.second + " package " + pkgPair.first + "'");
        } else {
            builder.AddNote(MakeRange(pair.first, ""),
                "another different package declaration 'public package " + DEFAULT_PACKAGE_NAME + "'");
        }
        counter++;
    }
}

void DiagRootPackageModifier(DiagnosticEngine& diag, const PackageSpec& packageSpec)
{
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::package_root_package_should_be_public, packageSpec,
        MakeRange(packageSpec.packageName));
    auto packageMsg = "package " + packageSpec.packageName;
    builder.AddNote("default modifier of 'package' is 'public', you can use '" + packageMsg + "' instead");
}
} // namespace Codira::Modules
