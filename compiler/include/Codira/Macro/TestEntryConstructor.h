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
 * This file declares the Test Entry Constructor related classes, which provide to construct test entry ast.
 */

#ifndef CODIRA_FRONTEND_TESTCONSTRUCTION_H
#define CODIRA_FRONTEND_TESTCONSTRUCTION_H

#include "Codira/AST/ASTContext.h"
#include "Codira/AST/Node.h"

namespace Codira {
template <typename T> inline std::unique_ptr<T> MakeUniquePtr()
{
    auto ptr = std::make_unique<T>();
    ptr->EnableAttr(AST::Attribute::COMPILER_ADD);
    return ptr;
}
template <typename T> inline OwnedPtr<T> MakeOwnedNode()
{
    auto ptr = MakeOwned<T>();
    ptr->EnableAttr(AST::Attribute::COMPILER_ADD);
    return ptr;
}
class TestPackage {
public:
    explicit TestPackage(const std::string& packageName) : packageName(packageName) {};
    const std::string& packageName;
    std::vector<Ptr<AST::FuncDecl>> testRegisterFunctions;
};

class TestModule {
public:
    explicit TestModule(const std::string& moduleName) : moduleName(moduleName) {};
    const std::string& moduleName;
    std::vector<OwnedPtr<TestPackage>> testPackages;
};

class TestEntryConstructor {
public:
    explicit TestEntryConstructor(DiagnosticEngine& diag) : diag(diag) {};
    void CheckTestSuite(const std::vector<OwnedPtr<AST::Package>>& packages);
    static void ConstructTestSuite(const std::string& moduleName,
        std::vector<OwnedPtr<AST::Package>>& srcPkgs,
        const std::vector<Ptr<AST::PackageDecl>> importedPkgs, bool compileTestsOnly);

    static bool IsTestRegistrationFunction(const Ptr<AST::Decl> funcDecl);
    DiagnosticEngine& diag;

private:
    void CheckTestSuiteConstraints(AST::Node& root, const std::vector<Ptr<AST::FuncDecl>>& funcs);
    void CheckClassWithMacro(AST::MacroExpandDecl& med);
    void CheckFunctionWithAtTest(
        AST::MacroExpandDecl& med, const std::vector<Ptr<AST::FuncDecl>>& funcs, const std::string& macroName);
    static void ConstructTestImports(AST::Package& pkg, TestModule& module);
    static void ConstructTestEntry(AST::Package& pkg, TestModule& module);
    static Ptr<AST::Package> FindMainPartPkgForTestPkg(
        const Ptr<AST::Package> testPackage, std::vector<Ptr<AST::PackageDecl>> importedPkgs);
};
} // namespace Codira
#endif // CODIRA_FRONTEND_TESTCONSTRUCTION_H
