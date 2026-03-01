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

#include "JavaDesugarManager.h"
#include "JavaInteropManager.h"
#include "Codira/AST/Match.h"
#include "Codira/AST/Walker.h"
#include "Codira/Utils/ConstantsUtils.h"

namespace Codira::Interop::Java {

void JavaDesugarManager::ProcessJavaMirrorImplStage(DesugarJavaMirrorImplStage stage, File& file)
{
    switch (stage) {
        case DesugarJavaMirrorImplStage::MIRROR_GENERATE_STUB:
            GenerateInMirrors(file, true);
            break;
        case DesugarJavaMirrorImplStage::MIRROR_GENERATE:
            GenerateInMirrors(file, false);
            break;
        case DesugarJavaMirrorImplStage::IMPL_GENERATE:
            GenerateInJavaImpls(file);
            break;
        case DesugarJavaMirrorImplStage::MIRROR_DESUGAR:
            DesugarMirrors(file);
            break;
        case DesugarJavaMirrorImplStage::IMPL_DESUGAR:
            DesugarInJavaImpls(file);
            break;
        case DesugarJavaMirrorImplStage::TYPECHECKS:
            DesugarTypechecks(file);
            break;
        default:
            CODEC_ABORT(); // unreachable state
    }

    std::move(generatedDecls.begin(), generatedDecls.end(), std::back_inserter(file.decls));
    generatedDecls.clear();
}

void JavaDesugarManager::ProcessCODEImplStage(DesugarCODEImplStage stage, File& file)
{
    switch (stage) {
        case DesugarCODEImplStage::IMPL_GENERATE:
            GenerateInCODEMapping(file);
            break;
        case DesugarCODEImplStage::IMPL_DESUGAR:
            DesugarInCODEMapping(file);
            break;
        case DesugarCODEImplStage::TYPECHECKS:
            DesugarTypechecks(file);
            break;
        default:
            CODEC_ABORT(); // unreachable state
    }

    std::move(generatedDecls.begin(), generatedDecls.end(), std::back_inserter(file.decls));
    generatedDecls.clear();
}

void JavaInteropManager::DesugarPackage(Package& pkg)
{
    if (!(hasMirrorOrImpl || enableInteropCODEMapping)) {
        return;
    }
    JavaDesugarManager desugarer{importManager, typeManager, diag, mangler, javagenOutputPath, outputPath};

    if (hasMirrorOrImpl) {
        auto nbegin = static_cast<uint8_t>(DesugarJavaMirrorImplStage::BEGIN);
        auto nend = static_cast<uint8_t>(DesugarJavaMirrorImplStage::END);
        for (uint8_t nstage = nbegin; nstage != nend; nstage++) {
            auto stage = static_cast<DesugarJavaMirrorImplStage>(nstage);
            if (stage == DesugarJavaMirrorImplStage::BEGIN) {
                continue;
            }
            for (auto& file : pkg.files) {
                desugarer.ProcessJavaMirrorImplStage(stage, *file);
            }
        }
    }

    // Currently CODEMapping is enable by compile config --enable-interop-codemapping
    if (enableInteropCODEMapping) {
        auto nbegin = static_cast<uint8_t>(DesugarCODEImplStage::BEGIN);
        auto nend = static_cast<uint8_t>(DesugarCODEImplStage::END);
        for (uint8_t nstage = nbegin; nstage != nend; nstage++) {
            auto stage = static_cast<DesugarCODEImplStage>(nstage);
            if (stage == DesugarCODEImplStage::BEGIN) {
                continue;
            }
            for (auto& file : pkg.files) {
                desugarer.ProcessCODEImplStage(stage, *file);
            }
        }
    }
}

} // namespace Codira::Interop::Java
