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

#ifndef LSPSERVER_SIGNATUREHELP_H
#define LSPSERVER_SIGNATUREHELP_H
#include "../../common/Callbacks.h"
#include "../../logger/Logger.h"
#include "Codira/Lex/Token.h"
#include "Codira/Parse/Parser.h"
#include "Codira/AST/ASTContext.h"
#include "Codira/AST/Symbol.h"
#include "Codira/Basic/Match.h"
#include "Codira/Utils/FileUtil.h"
#include "Codira/Modules/ImportManager.h"
#include "../../ArkAST.h"

namespace ark {
class SignatureHelpImpl {
public:
    SignatureHelpImpl(const ArkAST &ast,
                      SignatureHelp &result,
                      Codira::ImportManager &importManager,
                      const SignatureHelpParams &params,
                      const Codira::Position &pos);

    ~SignatureHelpImpl() = default;

    void FindSignatureHelp();

private:
    void SetRealTokensAndIndex();

    void NormalFuncSignatureHelp();

    bool MemberFuncSignatureHelp();

    void FindFunDeclByType(Codira::AST::Ty &nodeTy, const std::string funcName);

    void FindFunDeclByNode(Codira::AST::Node &node);

    void FindFuncDeclByDeclType(Ptr<Codira::AST::Ty> declTy, const std::string& funcName);

    void FillingDeclsInPackage(std::string &packageName, const std::string &funcName,
                               const Codira::AST::Node &curNode);

    void ResolveFuncDecl(Codira::AST::Decl &decl);

    void ResolveClassDecl(Codira::AST::Node &node);

    int CalActiveParaAndParamPos();

    void CalBackParamPos(const int &index);

    void DealRetrigger();

    void FindRealActiveParamPos();

    bool checkModifier(const std::string curPkg, Ptr<const Codira::AST::Decl> decl) const;

    std::string ResolveFuncName();

    void ResolveParameter(std::string &detail, bool &firstParams, const OwnedPtr<FuncParam> &paramPtr,
                          Signatures &signatures);

    bool IsFuncDeclValid(Ptr<Codira::AST::FuncDecl> funcDecl);

    void FindSuperClassInit(const std::vector<Symbol*>& query);

    bool checkAccess(const std::string curPkg, const Codira::AST::Decl &decl) const;

    int GetDotIndex() const;

    int GetFuncNameIndex() const;

    const ArkAST *ast = nullptr;
    std::pair<std::vector<Token>, int> realTokensAndIndex = {{}, -1};
    SignatureHelp *result = {};
    Codira::ImportManager *importManager = nullptr;
    SignatureHelpParams params = {};
    Codira::Position pos = {};
    std::set<std::string> signatureLabel = {};
    std::string packageNameForPath;
    bool hasNameParam = false;
    int nameParamPos = -1;
    int leftQuoteIndex = -1;
    int funcNameIndex = -1;
    unsigned int offset = 0;
    std::set<Position> visitedFunc = {};
    const int meanNoMatchParameter = 500;
    std::string fatherCLassName;
    bool isThis = false;
};
} // namespace ark

#endif // LSPSERVER_SIGNATUREHELP_H
