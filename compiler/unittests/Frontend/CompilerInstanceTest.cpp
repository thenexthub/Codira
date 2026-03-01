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

#include "gtest/gtest.h"
#include "Codira/AST/Match.h"
#include "Codira/AST/Node.h"
#include "Codira/Basic/SourceManager.h"
#include "Codira/FrontendTool/DefaultCompilerInstance.h"
#include "Codira/Utils/FileUtil.h"

#include <memory>
#include <string>

using namespace Codira;
using namespace AST;

class CompilerInstanceTest : public ::testing::Test {
public:
    CompilerInvocation invocation;
    DiagnosticEngine diag;
    std::string projectPath;
    std::string codiraHome;

protected:
    void SetUp() override
    {
#ifdef PROJECT_SOURCE_DIR
        // Gets the absolute path of the project from the compile parameter.
        projectPath = PROJECT_SOURCE_DIR;
        codiraHome = FileUtil::JoinPath(FileUtil::JoinPath(PROJECT_SOURCE_DIR, "build"), "build");
#else
        // Just in case, give it a default value. Assume the initial is in the build directory.
        projectPath = "..";
        codiraHome = FileUtil::JoinPath(FileUtil::JoinPath(".", "build"), "build");
#endif
#ifdef __x86_64__
        invocation.globalOptions.target.arch = Codira::Triple::ArchType::X86_64;
#else
        invocation.globalOptions.target.arch = Codira::Triple::ArchType::AARCH64;
#endif
#ifdef _WIN32
        invocation.globalOptions.target.os = Codira::Triple::OSType::WINDOWS;
#elif __unix__
        invocation.globalOptions.target.os = Codira::Triple::OSType::LINUX;
#endif
        invocation.globalOptions.compilePackage = true;
        invocation.globalOptions.compilationCachedPath = ".";
    };
    std::string code = R"(
        package pkg1
        class C{}
    )";
};

TEST_F(CompilerInstanceTest, DISABLED_FullCompile)
{
    std::unique_ptr<DefaultCompilerInstance> instance = std::make_unique<DefaultCompilerInstance>(invocation, diag);
    instance->srcDirs.emplace(FileUtil::JoinPath(projectPath, "unittests/Frontend/FullCompile/src"));
    instance->compileOnePackageFromSrcFiles = false;
    instance->codiraHome = codiraHome;
    instance->Compile();
    auto pkgs = instance->GetSourcePackages();
    ASSERT_EQ(pkgs.size(), 1);
    auto pkg = pkgs[0];
    ASTContext* ctx = instance->GetASTContextByPackage(pkg);
    ASSERT_TRUE(ctx != nullptr);
    EXPECT_EQ(ctx->curPackage, pkg);
}

TEST_F(CompilerInstanceTest, DISABLED_GetAllVisibleExtendMembers01)
{
    std::unique_ptr<DefaultCompilerInstance> instance = std::make_unique<DefaultCompilerInstance>(invocation, diag);
    instance->srcDirs.emplace(FileUtil::JoinPath(projectPath, "unittests/Frontend/FullCompile/src"));
    instance->compileOnePackageFromSrcFiles = false;
    instance->codiraHome = codiraHome;
    instance->Compile();
    auto pkgs = instance->GetSourcePackages();
    ASSERT_EQ(pkgs.size(), 1);
    ASTContext* ctx = instance->GetASTContextByPackage(pkgs[0]);
    ASSERT_NE(ctx, nullptr);
    Searcher searcher;
    auto extendSyms = searcher.Search(*ctx, "ast_kind:extend_decl", Sort::posAsc);
    // Int64 -> #{Eqq}
    auto memberSet1 = instance->GetAllVisibleExtendMembers(
        StaticAs<ASTKind::EXTEND_DECL>(extendSyms[0]->node)->extendedType->ty, *extendSyms[0]->node->curFile);
    bool containExtendMember = false;
    for (auto member : memberSet1) {
        EXPECT_TRUE(member->astKind == ASTKind::FUNC_DECL || member->astKind == ASTKind::PROP_DECL);
        if (member->identifier.Val() == "g") {
            containExtendMember = true;
            break;
        }
    }
    EXPECT_TRUE(containExtendMember);
    auto classSyms = searcher.Search(*ctx, "(ast_kind:class_decl && name:A)");
    containExtendMember = false;
    // class A -> #{Eqq}
    auto memberSet2 = instance->GetAllVisibleExtendMembers(
        RawStaticCast<InheritableDecl*>(classSyms[0]->node), *extendSyms[0]->node->curFile);
    for (auto member : memberSet2) {
        EXPECT_TRUE(member->astKind == ASTKind::FUNC_DECL || member->astKind == ASTKind::PROP_DECL);
        if (member->identifier.Val() == "g") {
            containExtendMember = true;
            break;
        }
    }
    EXPECT_TRUE(containExtendMember);
}

TEST_F(CompilerInstanceTest, Comments)
{
    std::unique_ptr<DefaultCompilerInstance> instance = std::make_unique<DefaultCompilerInstance>(invocation, diag);
    instance->srcDirs.emplace(FileUtil::JoinPath(projectPath, "unittests/Frontend/FullCompile/src"));
    instance->compileOnePackageFromSrcFiles = false;
    instance->codiraHome = codiraHome;
    instance->Compile();
    auto pkgs = instance->GetSourcePackages();
    ASSERT_EQ(pkgs.size(), 1);
    bool oneComments{false};
    for (auto& file : pkgs[0]->files) {
        auto comments = instance->GetSourceManager().GetSource(file->begin.fileID).offsetCommentsMap;
        if (comments.size() == 5) {
            oneComments = true;
            break;
        }
    }
    EXPECT_TRUE(oneComments);
}

TEST_F(CompilerInstanceTest, DISABLED_TrailingClosure)
{
    std::unique_ptr<DefaultCompilerInstance> instance = std::make_unique<DefaultCompilerInstance>(invocation, diag);
    instance->srcDirs.emplace(FileUtil::JoinPath(projectPath, "unittests/Frontend/TrailingClosure/src"));
    instance->compileOnePackageFromSrcFiles = false;
    instance->codiraHome = codiraHome;
    instance->Compile();
    auto pkgs = instance->GetSourcePackages();
    ASSERT_EQ(pkgs.size(), 1);
    ASTContext* ctx = instance->GetASTContextByPackage(pkgs[0]);
    ASSERT_NE(ctx, nullptr);
    Searcher searcher;
    auto syms = searcher.Search(*ctx, "name:i && ast_kind: ref_expr");
    // 3 * 2 of i.
    size_t n = 6;
    size_t isClonedNode = 0;
    for (size_t i = 0; i < n; i++) {
        ASSERT_TRUE(i < syms.size() && syms[i] && syms[i]->node);
        if (syms[i]->node->TestAttr(Attribute::IS_CLONED_SOURCE_CODE)) {
            ++isClonedNode;
        }
    }
    // Trailing Closure will be desugared without any cloned node.
    EXPECT_TRUE(isClonedNode == 0);
    syms = searcher.Search(*ctx, "name:f2 && ast_kind: ref_expr");
    EXPECT_TRUE(syms.size() == 1);
}
