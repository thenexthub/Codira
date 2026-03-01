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
 * This file declares auxiliary methods for Codira Native FFI implementation with different targets
 */
#ifndef CODIRA_SEMA_NATIVE_FFI_UTILS
#define CODIRA_SEMA_NATIVE_FFI_UTILS

#include "Codira/Mangle/BaseMangler.h"
#include "Codira/Modules/ImportManager.h"
#include "Codira/Sema/TypeManager.h"
#include "Codira/AST/Create.h"
#include "Codira/AST/Match.h"
#include "Codira/AST/Utils.h"

namespace Codira::Native::FFI {
using namespace AST;

enum class ArrayOperationKind: uint8_t {
    CREATE,
    GET,
    SET,
    GET_LENGTH
};

OwnedPtr<RefExpr> CreateThisRef(Ptr<Decl> target, Ptr<Ty> ty, Ptr<File> curFile);

OwnedPtr<CallExpr> CreateThisCall(Decl& target, FuncDecl& baseTarget, Ptr<Ty> funcTy, Ptr<File> curFile);

OwnedPtr<PrimitiveType> CreateUnitType(Ptr<File> curFile);

std::vector<Ptr<Ty>> GetParamTys(FuncParamList& params);

OwnedPtr<RefExpr> CreateSuperRef(Ptr<Decl> target, Ptr<Ty> ty);

OwnedPtr<CallExpr> CreateSuperCall(Decl& target, FuncDecl& baseTarget, Ptr<Ty> funcTy);

ArrayOperationKind GetArrayOperationKind(Decl& decl);

template <typename Ret = Node, typename... Args>
std::vector<OwnedPtr<Ret>> Nodes(OwnedPtr<Args>&&... args)
{
    std::vector<OwnedPtr<Ret>> nodes;
    (nodes.push_back(std::forward<OwnedPtr<Args>>(args)), ...);
    return nodes;
}

namespace details {

template <typename T>
void WrapArg(std::vector<OwnedPtr<FuncArg>>* funcArgs, OwnedPtr<T>&& e)
{
    CODEC_ASSERT(e);
    if (auto ptr = As<ASTKind::FUNC_ARG>(e.get())) {
        funcArgs->emplace_back(ptr);
    } else {
        funcArgs->push_back(CreateFuncArg(std::forward<OwnedPtr<T>>(e)));
    }
}

} // namespace details

template <typename T>
OwnedPtr<T> WithinFile(OwnedPtr<T> node, Ptr<File> curFile)
{
    CODEC_NULLPTR_CHECK(curFile);
    node->curFile = curFile;
    return node;
}

template <typename... Args>
OwnedPtr<CallExpr> CreateCall(Ptr<FuncDecl> fd, Ptr<File> curFile, OwnedPtr<Args>&&... args)
{
    if (!fd) {
        return nullptr;
    }

    std::vector<OwnedPtr<FuncArg>> funcArgs;

    (details::WrapArg(&funcArgs, std::forward<OwnedPtr<Args>>(args)), ...);

    auto funcTy = StaticCast<FuncTy*>(fd->ty);

    return CreateCallExpr(WithinFile(CreateRefExpr(*fd), curFile), std::move(funcArgs), fd, funcTy->retTy,
                          CallKind::CALL_DECLARED_FUNCTION);
}

OwnedPtr<Type> CreateType(Ptr<Ty> ty);
OwnedPtr<Type> CreateFuncType(Ptr<FuncTy> ty);

OwnedPtr<Expr> CreateBoolMatch(OwnedPtr<Expr> selector, OwnedPtr<Expr> trueBranch, OwnedPtr<Expr> falseBranch,
    Ptr<Ty> ty);

StructDecl& GetStringDecl(const ImportManager& importManager);

/**
 * Returns synthetic lambda call that includes nodes. The result of the call expr is the last node:
 *
 * {
 *     node1;
 *     node2;
 *     ...
 *     return noden;
 * }()
 */

OwnedPtr<CallExpr> WrapReturningLambdaCall(TypeManager& typeManager, std::vector<OwnedPtr<Node>> nodes);
OwnedPtr<LambdaExpr> WrapReturningLambdaExpr(TypeManager& typeManager, std::vector<OwnedPtr<Node>> nodes);

/**
 * Returns trimmed codira library name.
 * For a filename in [outputLibPath] matched to "lib{libname}.{ext}" it returns {libname} if [trimmed] = `true`
 * and "lib{libname}.{ext}" if [trimmed] = `false`.
 * For other cases, it returns [fullPackageName]
 */
std::string GetCodiraLibName(const std::string& outputLibPath, const std::string& fullPackageName,
    bool trimmed = true);

std::string GetMangledMethodName(const BaseMangler& mangler, const std::vector<OwnedPtr<FuncParam>>& params,
    const std::string& methodName);

} // namespace Codira::Interop::Java

#endif // CODIRA_SEMA_NATIVE_FFI_UTILS
