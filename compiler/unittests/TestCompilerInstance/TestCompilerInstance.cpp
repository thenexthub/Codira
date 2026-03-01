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
 * This file implements the TestCompilerInstance.
 */

#include "TestCompilerInstance.h"

#include "Codira/IncrementalCompilation/ASTCacheCalculator.h"
#include "Codira/Parse/Parser.h"

using namespace Codira;
using namespace AST;

namespace {
RawMangled2DeclMap RunASTCacheCalculation(const Package& sourcePackage, const GlobalOptions op)
{
    Codira::IncrementalCompilation::ASTCacheCalculator pc{
        sourcePackage, std::make_pair(op.enableCompileDebug, op.displayLineInfo)};
    pc.Walk();
    return pc.mangled2Decl;
}
} // namespace

bool TestCompilerInstance::Compile(CompileStage stage)
{
    if (!PerformParse()) {
        return false;
    }
    if (stage == CompileStage::PARSE) {
        return true;
    }
    // Preprocess for incremental mangle.
    if (!srcPkgs.empty() && stage == CompileStage::DESUGAR_AFTER_SEMA) {
        rawMangleName2DeclMap = RunASTCacheCalculation(*srcPkgs[0], invocation.globalOptions);
    }
    auto modular = ModularizeCompilation();
    auto importRes = PerformImportPackage();
    if (stage == CompileStage::IMPORT_PACKAGE) {
        return modular && importRes;
    }
    auto macroRes = PerformMacroExpand();
    auto semaRes = PerformSema();
    if (stage == CompileStage::SEMA || stage == CompileStage::DESUGAR_AFTER_SEMA) {
        return Utils::AllOf(importRes, macroRes, semaRes, modular);
    }
    auto giRes = PerformGenericInstantiation();
    return Utils::AllOf(importRes, macroRes, semaRes, giRes, modular);
}

bool TestCompilerInstance::ParseCode()
{
    auto package = MakeOwned<Package>();
    auto fileID = GetSourceManager().AddSource("", code);
    Parser parser(fileID, code, diag, GetSourceManager(), invocation.globalOptions.enableAddCommentToAst,
        invocation.globalOptions.compileCoded);
    parser.SetCompileOptions(invocation.globalOptions);
    auto file = parser.ParseTopLevel();
    GetSourceManager().AddComments(parser.GetCommentsMap());
    if (!file->package) {
        package->fullPackageName = DEFAULT_PACKAGE_NAME;
    } else {
        package->fullPackageName = file->package->GetPackageName();
    }
    file->curPackage = package.get();
    package->files.push_back(std::move(file));
    srcPkgs.push_back(std::move(package));
    return true;
}

bool TestCompilerInstance::PerformParse()
{
    if (!code.empty()) {
        return ParseCode();
    }
    return compileStrategy->Parse();
}
