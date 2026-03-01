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

#ifndef LSPSERVER_ITEMRESOLVERUTIL_H
#define LSPSERVER_ITEMRESOLVERUTIL_H

#include "Codira/AST/ASTContext.h"
#include "Codira/AST/Types.h"
#include "../../json-rpc/CompletionType.h"
#include "../../json-rpc/Protocol.h"
#include "Codira/AST/Node.h"
#include "Codira/Basic/SourceManager.h"

namespace ark {
std::unordered_map<std::string, int> InitKeyMap();

class ItemResolverUtil {
public:
    static std::string ResolveNameByNode(Codira::AST::Node &node);

    static CompletionItemKind ResolveKindByNode(Codira::AST::Node &node);

    static CompletionItemKind ResolveKindByASTKind(Codira::AST::ASTKind &astKind);

    static std::string ResolveDetailByNode(Codira::AST::Node &node, Codira::SourceManager *sourceManager = nullptr);

    static std::string ResolveSourceByNode(Ptr<Codira::AST::Decl> decl, std::string path);

    static std::string ResolveSignatureByNode(const Codira::AST::Node &node,
                                              Codira::SourceManager *sourceManager = nullptr,
                                              bool isCompletionInsert = false,
                                              bool isAfterAT = false);

    static std::string ResolveInsertByNode(const Codira::AST::Node &primaryCtorDecl,
                                           Codira::SourceManager *sourceManager = nullptr, bool isAfterAT = false);

    static Ptr<Codira::AST::Decl> GetDeclByTy(Codira::AST::Ty *enumTy);

    static std::string GetGenericInsertByDecl(Ptr<Codira::AST::Generic> genericDecl);

    static void ResolveFuncParams(std::string &detail,
                                  const std::vector<OwnedPtr<Codira::AST::FuncParamList>> &paramLists,
                                  bool isEnumConstruct = false, Codira::SourceManager *sourceManager = nullptr,
                                  const std::string &filePath = "",
                                  bool needLastParam = true);

    static void ResolveMacroParams(std::string &detail,
        const std::vector<OwnedPtr<Codira::AST::FuncParamList>> &paramLists);

    static void GetDetailByTy(const Codira::AST::Ty *ty, std::string &detail, bool isLambda = false);

    static std::string GetGenericParamByDecl(Ptr<Codira::AST::Generic> genericDecl);

    static bool IsCustomAnnotation(const Codira::AST::Decl &decl);

    static void AddTypeByNodeAndType(std::string &detail, const std::string filePath, Ptr<Codira::AST::Node> type,
        Codira::SourceManager *sourceManager);

    static std::string FetchTypeString(const Codira::AST::Type &type);

    static void DealTypeDetail(std::string &detail, Ptr<Codira::AST::Type> type, const std::string &filePath,
        Codira::SourceManager *sourceManager = nullptr);

    static void ResolveFuncTypeParamSignature(std::string &detail,
        const std::vector<OwnedPtr<Codira::AST::Type>> &paramTypes,
        Codira::SourceManager *sourceManager, const std::string &filePath, bool needLastParam = true);

    static void ResolveFuncTypeParamInsert(std::string &detail,
        const std::vector<OwnedPtr<Codira::AST::Type>> &paramTypes, Codira::SourceManager *sourceManager,
        const std::string &filePath, int &numParm, bool needLastParam = true, bool needDefaultParamName = false);

    static std::string ResolveFollowLambdaSignature(const Codira::AST::Node &node,
        Codira::SourceManager *sourceManager = nullptr, const std::string &initFuncReplace = "");

    static std::string ResolveFollowLambdaInsert(const Codira::AST::Node &node,
        Codira::SourceManager *sourceManager = nullptr, const std::string &initFuncReplace = "");

    static void ResolveParamListFuncTypeVarDecl(const Codira::AST::Node &node, std::string &label,
        std::string &insertText, Codira::SourceManager *sourceManager = nullptr);

    static int AddGenericInsertByDecl(std::string &detail, Ptr<Codira::AST::Generic> genericDecl);

    static int ResolveFuncParamInsert(std::string &detail, const std::string myFilePath,
        Ptr<Codira::AST::FuncParam> param, int numParm, Codira::SourceManager *sourceManager, bool isEnumConstruct);

private:

    template <typename T>
    static std::string GetGenericString(const T &t);

    static void GetInitializerInfo(std::string &detail, const Codira::AST::VarDecl &decl,
                                   Codira::SourceManager *sourceManager, bool hasType);

    static void ResolveVarDeclDetail(std::string &detail, const Codira::AST::VarDecl &decl,
                                     Codira::SourceManager *sourceManager = nullptr);
    
    static void ResolveFuncDeclQuickLook(std::string &detail, const Codira::AST::FuncDecl &decl,
                                         Codira::SourceManager *sourceManager = nullptr);

    static void ResolveFuncDeclDetail(std::string &detail, const Codira::AST::FuncDecl &decl,
                                      Codira::SourceManager *sourceManager = nullptr);

    static void ResolvePrimaryCtorDeclDetail(std::string &detail, const Codira::AST::PrimaryCtorDecl &decl,
                                             Codira::SourceManager *sourceManager = nullptr);

    static void ResolvePrimaryCtorDeclSignature(std::string &detail, const Codira::AST::PrimaryCtorDecl &decl,
                                                Codira::SourceManager *sourceManager = nullptr,
                                                bool isAfterAT = false);

    static void ResolvePatternSignature(std::string &signature, Ptr<Codira::AST::Pattern> pattern,
                                        Codira::SourceManager *sourceManager = nullptr);

    static void ResolvePatternDetail(std::string &detail, Ptr<Codira::AST::Pattern> pattern,
                                     Codira::SourceManager *sourceManager = nullptr);

    static void ResolveMacroDeclDetail(std::string &detail, const Codira::AST::MacroDecl &decl,
                                       Codira::SourceManager *sourceManager = nullptr);

    static void ResolveMacroDeclSignature(std::string &detail, const Codira::AST::MacroDecl &decl,
                                          Codira::SourceManager *sourceManager = nullptr);

    static void ResolveClassDeclDetail(std::string &detail, Codira::AST::ClassDecl &decl,
                                       Codira::SourceManager *sourceManager = nullptr);

    static void ResolveInterfaceDeclDetail(std::string &detail, Codira::AST::InterfaceDecl &decl,
                                           Codira::SourceManager *sourceManager = nullptr);

    static void ResolveEnumDeclDetail(std::string &detail, const Codira::AST::EnumDecl &decl,
                                      Codira::SourceManager *sourceManager = nullptr);

    static void ResolveFuncDeclSignature(std::string &detail, const Codira::AST::FuncDecl &decl,
                                         Codira::SourceManager *sourceManager = nullptr,
                                         bool isCompletionInsert = false,
                                         bool isAfterAT = false);

    template<typename T>
    static void ResolveFuncLikeDeclInsert(std::string &detail,
                                          const T &decl,
                                          Codira::SourceManager *sourceManager = nullptr,
                                          bool isAfterAT = false);

    static void ResolveStructDeclDetail(std::string &detail, const Codira::AST::StructDecl &decl);

    static void ResolveGenericParamDeclDetail(std::string &detail, const Codira::AST::GenericParamDecl &decl);

    static void ResolveTypeAliasDetail(std::string &detail, const Codira::AST::TypeAliasDecl &decl,
                                       Codira::SourceManager *sourceManager = nullptr);

    static void ResolveBuiltInDeclDetail(std::string &detail, const Codira::AST::BuiltInDecl &decl);
 
    static void ResolveMacroParamsInsert(std::string &detail,
                                         const std::vector<OwnedPtr<Codira::AST::FuncParamList>> &paramLists);

    template<typename T> static void ResolveFollowLambdaFuncSignature(std::string &detail, const T &decl,
        Codira::SourceManager *sourceManager = nullptr, const std::string &initFuncReplace = "");

    static void ResolveFollowLambdaVarSignature(std::string &detail, const Codira::AST::VarDecl &decl,
        Codira::SourceManager *sourceManager = nullptr, const std::string &initFuncReplace = "");

    template<typename T> static void ResolveFollowLambdaFuncInsert(std::string &detail, const T &decl,
        Codira::SourceManager *sourceManager = nullptr, const std::string &initFuncReplace = "");

    static void ResolveFollowLambdaVarInsert(std::string &detail, const Codira::AST::VarDecl &decl,
        Codira::SourceManager *sourceManager = nullptr, const std::string &initFuncReplace = "");

    static const int detailMaxLen = 256;

    template<typename T> static int BuildLambdaFuncPreParamInsert(const T &decl, Codira::SourceManager *sourceManager,
        OwnedPtr<Codira::AST::FuncParamList> &paramList, const std::string &myFilePath, std::string &insertText);

    static void GetFuncNamedParam(std::string &detail, Codira::SourceManager *sourceManager,
        const std::string &filePath, const OwnedPtr<Codira::AST::FuncParam> &param);

    template<typename T> static void DealEmptyParamFollowLambda(const T &decl,
        Codira::SourceManager *sourceManager, OwnedPtr<Codira::AST::FuncParamList> &paramList, std::string &signature,
        const std::string &myFilePath);
};
} // namespace ark

#endif // LSPSERVER_ITEMRESOLVERUTIL_H
