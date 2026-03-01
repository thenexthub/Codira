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
 * This file declares a thin bridge to interoplib.objc library
 */

#ifndef CODIRA_SEMA_OBJ_C_UTILS_INTEROPLIB_BRIDGE_H
#define CODIRA_SEMA_OBJ_C_UTILS_INTEROPLIB_BRIDGE_H

#include "Codira/AST/Match.h"
#include "Codira/AST/Node.h"
#include "Codira/Modules/ImportManager.h"
#include "Codira/Utils/SafePointer.h"

namespace Codira::Interop::ObjC {
class InteropLibBridge {
public:
    explicit InteropLibBridge(ImportManager& importManager, DiagnosticEngine& diag)
        : importManager(importManager), diag(diag)
    {
    }

    /**
     * Gets NativeObjCId declaration.
     * id or Instance Method Pointer type.
     */
    Ptr<AST::TypeAliasDecl> GetNativeObjCIdDecl();

    /**
     * Gets NativeObjCId semantic type.
     */
    Ptr<AST::Ty> GetNativeObjCIdTy();

    /**
     * Gets NativeObjCSel declaration.
     * SEL.
     */
    Ptr<AST::TypeAliasDecl> GetNativeObjCSelDecl();

    /**
     * Gets NativeObjCSuperPtr (CPointer<NativeObjCSuper>) declaration.
     * objc_super*.
     */
    Ptr<AST::TypeAliasDecl> GetNativeObjCSuperPtrDecl();

    /**
     * Gets RegistryId declaration.
     * An opaque identifier for Codira mirror objects.
     */
    Ptr<AST::TypeAliasDecl> GetRegistryIdDecl();

    /**
     * Gets RegistryId semantic type.
     */
    Ptr<AST::Ty> GetRegistryIdTy();

    /**
     * Gets ObjCUnreachableCodeException declaration.
     * An exception that has to be used to mark an unreachable code (e.g instantiation @ObjCImpl objects from Codira
     * side).
     */
    Ptr<AST::ClassDecl> GetObjCUnreachableCodeExceptionDecl();

    Ptr<AST::FuncDecl> GetGetFromRegistryByNativeHandleDecl();

    Ptr<AST::FuncDecl> GetGetFromRegistryByIdDecl();

    Ptr<AST::FuncDecl> GetPutToRegistryDecl();

    Ptr<AST::FuncDecl> GetRemoveFromRegistryDecl();

    Ptr<AST::FuncDecl> GetAllocDecl();

    Ptr<AST::FuncDecl> GetWithAutoreleasePoolDecl();

    Ptr<AST::FuncDecl> GetWithAutoreleasePoolObjDecl();

    Ptr<AST::FuncDecl> GetRegisterNameDecl();

    Ptr<AST::FuncDecl> GetGetInstanceVariableObjDecl();

    Ptr<AST::FuncDecl> GetSetInstanceVariableObjDecl();

    Ptr<AST::FuncDecl> GetGetInstanceVariableDecl();

    Ptr<AST::FuncDecl> GetSetInstanceVariableDecl();

    Ptr<AST::FuncDecl> GetGetClassDecl();

    /**
     * Gets ObjCRuntime declaration.
     * This struct exports interface of an Objective-C runtime.
     */
    Ptr<AST::StructDecl> GetObjCRuntimeDecl();

    OwnedPtr<AST::MemberAccess> CreateObjCRuntimeMsgSendExpr();

    OwnedPtr<AST::MemberAccess> CreateObjCRuntimeReleaseExpr();

    /**
     * Get objc.lang.ObjCPointer declaration
    */
    Ptr<AST::StructDecl> GetObjCPointerDecl();

private:
    OwnedPtr<AST::RefExpr> CreateObjCRuntimeRefExpr();

    template <AST::ASTKind K = AST::ASTKind::DECL> auto GetInteropLibDecl(const std::string& ident)
    {
        const auto interoplibObjCPackageName = "interoplib.objc";
        auto decl = importManager.GetImportedDecl(interoplibObjCPackageName, ident);
        if (!decl) {
            diag.DiagnoseRefactor(DiagKindRefactor::sema_member_not_imported, DEFAULT_POSITION,
                interoplibObjCPackageName + std::string(".") + ident);
            return Ptr(AST::As<K>(nullptr));
        }

        CODEC_ASSERT(decl && decl->astKind == K);
        return Ptr(AST::StaticAs<K>(decl));
    }

    template <AST::ASTKind K = AST::ASTKind::DECL> auto GetObjCLangDecl(const std::string& ident)
    {
        const auto objcLangPackageName = "objc.lang";
        auto decl = importManager.GetImportedDecl(objcLangPackageName, ident);
        if (!decl) {
            diag.DiagnoseRefactor(DiagKindRefactor::sema_member_not_imported, DEFAULT_POSITION, ident);
            return Ptr(AST::As<K>(nullptr));
        }

        CODEC_ASSERT(decl && decl->astKind == K);
        return Ptr(AST::StaticAs<K>(decl));
    }

    ImportManager& importManager;
    DiagnosticEngine& diag;
};

} // namespace Codira::Interop::ObjC

#endif // CODIRA_SEMA_OBJ_C_UTILS_INTEROPLIB_BRIDGE_H
