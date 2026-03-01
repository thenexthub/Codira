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
 * This file declares the TypeManager related classes, which manages all types.
 */

#ifndef CODIRA_SEMA_TEST_MANAGER_H
#define CODIRA_SEMA_TEST_MANAGER_H

#include "Codira/AST/Walker.h"
#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/Modules/ImportManager.h"
#include "Codira/Option/Option.h"
#include "Codira/Sema/GenericInstantiationManager.h"
#include "Codira/Sema/TypeManager.h"

namespace Codira {

class MockManager;
class MockSupportManager;
class MockUtils;

enum class MockKind : uint8_t {
    PLAIN_MOCK,
    SPY,
    NOT_MOCK
};

class TestManager {
public:
    explicit TestManager(
        ImportManager& im, TypeManager& tm, DiagnosticEngine& diag, const GlobalOptions& compilationOptions
    );
    void PreparePackageForTestIfNeeded(AST::Package& pkg);
    void MarkDeclsForTestIfNeeded(std::vector<Ptr<AST::Package>> pkgs) const;
    static bool IsDeclOpenToMock(const AST::Decl& decl);
    static bool IsDeclGeneratedForTest(const AST::Decl& decl);
    static bool IsMockAccessor(const AST::Decl& decl);
    void Init(GenericInstantiationManager* instantiationManager);
    ~TestManager();

private:
    ImportManager& importManager;
    TypeManager& typeManager;
    DiagnosticEngine& diag;
    const bool testEnabled;
    MockMode mockMode;
    const bool mockCompatibleIfNeeded;
    const bool mockCompatible;
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    const bool exportForTest;
#endif
    OwnedPtr<MockManager> mockManager {nullptr};
    OwnedPtr<MockSupportManager> mockSupportManager {nullptr};
    Ptr<MockUtils> mockUtils {nullptr};

    Ptr<AST::ClassDecl> GenerateMockClassIfNeededAndGet(const AST::CallExpr& callExpr, AST::Package& pkg);
    void GenerateAccessors(AST::Package& pkg);
    void ReplaceCallsWithAccessors(AST::Package& pkg);
    void ReplaceCallsToForeignFunctions(AST::Package& pkg);
    void HandleMockCalls(AST::Package& pkg);
    Ptr<AST::ClassLikeDecl> GetInstantiatedDeclInCurrentPackage(const Ptr<const AST::ClassLikeTy> classLikeToMockTy);
    void CheckIfNoMockSupportDependencies(const AST::Package& curPkg);
    bool IsThereMockUsage(AST::Package& pkg) const;
    static bool ArePackagesMockSupportConsistent(
        const AST::Package& currentPackage, const AST::Package& importedPackage);
    AST::VisitAction HandleCreateMockCall(AST::CallExpr& callExpr, AST::Package& pkg);
    void WrapWithRequireMockObjectIfNeeded(Ptr<AST::Expr> expr, Ptr<AST::Decl> target);
    AST::VisitAction HandleMockAnnotatedLambda(const AST::LambdaExpr& lambda);
    void ReportDoesntSupportMocking(const AST::Expr& reportOn, const std::string& name, const std::string& package);
    void ReportDoesntSupportFrozen(const AST::Expr& reportOn);
    void ReportUnsupportedType(const AST::Expr& reportOn);
    void ReportNotInTestMode(const AST::Expr& reportOn);
    void ReportMockDisabled(const AST::Expr& reportOn);
    void ReportWrongStaticDecl(const AST::Expr& reportOn);
    void PrepareDecls(AST::Package& pkg);
    void PrepareToSpy(AST::Package& pkg);
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    void ReportFrozenRequired(const AST::FuncDecl& reportOn);
    void MarkMockCreationContainingGenericFuncs(AST::Package& pkg) const;
    bool ShouldBeMarkedAsContainingMockCreationCall(
        const AST::CallExpr& callExpr, const Ptr<AST::FuncDecl> enclosingFunc) const;
    void HandleDeclsToExportForTest(std::vector<Ptr<AST::Package>> pkgs) const;
    void CollectInternalDeclUsages(AST::Package& pkg);
#endif
};

} // namespace Codira

#endif // CODIRA_SEMA_TYPE_MANAGER_H
