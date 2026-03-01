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

#include "Codira/CHIR/AST2CHIR/AST2CHIRChecker.h"

#include "Codira/Utils/ProfileRecorder.h"
#include "Codira/CHIR/CHIRCasting.h"
#include "Codira/CHIR/Utils.h"
#include "Codira/CHIR/Value.h"

using namespace Codira;
using namespace Codira::CHIR;

namespace {
void Errorln(const std::string& info)
{
    std::cerr << "ast2chir checker error: " << info << std::endl;
}

bool CheckPrimitiveType(const Codira::AST::Ty& astTy, const Type& chirTy)
{
    const std::map<Type::TypeKind, Codira::AST::TypeKind> chir2astTy = {
        {Type::TypeKind::TYPE_INT8, Codira::AST::TypeKind::TYPE_INT8},
        {Type::TypeKind::TYPE_INT16, Codira::AST::TypeKind::TYPE_INT16},
        {Type::TypeKind::TYPE_INT32, Codira::AST::TypeKind::TYPE_INT32},
        {Type::TypeKind::TYPE_INT64, Codira::AST::TypeKind::TYPE_INT64},
        {Type::TypeKind::TYPE_INT_NATIVE, Codira::AST::TypeKind::TYPE_INT_NATIVE},
        {Type::TypeKind::TYPE_UINT8, Codira::AST::TypeKind::TYPE_UINT8},
        {Type::TypeKind::TYPE_UINT16, Codira::AST::TypeKind::TYPE_UINT16},
        {Type::TypeKind::TYPE_UINT32, Codira::AST::TypeKind::TYPE_UINT32},
        {Type::TypeKind::TYPE_UINT64, Codira::AST::TypeKind::TYPE_UINT64},
        {Type::TypeKind::TYPE_UINT_NATIVE, Codira::AST::TypeKind::TYPE_UINT_NATIVE},
        {Type::TypeKind::TYPE_FLOAT16, Codira::AST::TypeKind::TYPE_FLOAT16},
        {Type::TypeKind::TYPE_FLOAT32, Codira::AST::TypeKind::TYPE_FLOAT32},
        {Type::TypeKind::TYPE_FLOAT64, Codira::AST::TypeKind::TYPE_FLOAT64},
        {Type::TypeKind::TYPE_RUNE, Codira::AST::TypeKind::TYPE_RUNE},
        {Type::TypeKind::TYPE_BOOLEAN, Codira::AST::TypeKind::TYPE_BOOLEAN},
        {Type::TypeKind::TYPE_UNIT, Codira::AST::TypeKind::TYPE_UNIT},
        {Type::TypeKind::TYPE_NOTHING, Codira::AST::TypeKind::TYPE_NOTHING}};
    if (chir2astTy.count(chirTy.GetTypeKind()) == 0) {
        Codira::InternalError("unsupported type kind");
    }
    return chir2astTy.at(chirTy.GetTypeKind()) == astTy.kind;
}

bool CheckType(const Codira::AST::Ty& astTy, const Type& chirTy);

bool CheckTypeArgs(const Codira::AST::Ty& astTy, const Type& chirTy)
{
    auto astTyArgs = astTy.typeArgs;
    auto chirTyArgs = chirTy.GetTypeArgs();
    if (astTyArgs.size() != chirTyArgs.size()) {
        return false;
    }
    for (size_t i = 0; i < astTyArgs.size(); ++i) {
        if (!CheckType(*astTyArgs[i], *chirTyArgs[i])) {
            return false;
        }
    }
    return true;
}

bool CheckTupleType(const Codira::AST::Ty& astTy, const Type& chirTy)
{
    auto astTyArgs = astTy.typeArgs;
    auto chirTyArgs = chirTy.GetTypeArgs();
    if (astTyArgs.size() != chirTyArgs.size()) {
        return false;
    }
    for (size_t loop = 0; loop < astTyArgs.size(); loop++) {
        if (!CheckType(*astTyArgs[loop], *chirTyArgs[loop])) {
            return false;
        }
    }
    return true;
}

bool CheckFuncType(const Codira::AST::Ty& astTy, const Type& chirTy)
{
    auto astTyArgs = astTy.typeArgs;
    auto chirTyArgs = chirTy.GetTypeArgs();
    if (astTyArgs.size() != chirTyArgs.size()) {
        return false;
    }
    for (size_t loop = 0; loop < astTyArgs.size(); loop++) {
        if (!CheckType(*astTyArgs[loop], *chirTyArgs[loop])) {
            return false;
        }
    }
    return true;
}

bool CheckMethodType(const Codira::AST::Ty& astTy, const Type& chirTy)
{
    auto astTyArgs = astTy.typeArgs;
    auto chirTyArgs = chirTy.GetTypeArgs();
    if (astTyArgs.size() + 1 != chirTyArgs.size()) {
        return false;
    }
    for (size_t loop = 0; loop < astTyArgs.size(); loop++) {
        if (!CheckType(*astTyArgs[loop], *chirTyArgs[loop + 1])) {
            return false;
        }
    }
    return true;
}

bool CheckStructType(const Codira::AST::Ty& astTy, const Type& chirTy)
{
    if (!astTy.IsStruct()) {
        return false;
    }
    auto astStruct = Codira::StaticCast<const Codira::AST::StructTy&>(astTy).declPtr;
    auto chirStruct = Codira::StaticCast<const StructType&>(chirTy).GetStructDef();
    if (astStruct->mangledName != chirStruct->GetIdentifierWithoutPrefix()) {
        return false;
    }
    return CheckTypeArgs(astTy, chirTy);
}

bool CheckClassType(const Codira::AST::Ty& astTy, const Type& chirTy)
{
    if (!astTy.IsClass()) {
        return false;
    }
    auto astClass = Codira::StaticCast<const Codira::AST::ClassTy&>(astTy).declPtr;
    auto chirClass = Codira::StaticCast<const ClassType&>(chirTy).GetClassDef();
    if (astClass->mangledName != chirClass->GetIdentifierWithoutPrefix()) {
        return false;
    }
    return CheckTypeArgs(astTy, chirTy);
}

bool CheckInterfaceType(const Codira::AST::Ty& astTy, const Type& chirTy)
{
    if (!astTy.IsInterface()) {
        return false;
    }
    auto astClass = Codira::StaticCast<const Codira::AST::InterfaceTy&>(astTy).declPtr;
    auto chirClass = Codira::StaticCast<const ClassType&>(chirTy).GetClassDef();
    if (astClass->mangledName != chirClass->GetIdentifierWithoutPrefix()) {
        return false;
    }
    return CheckTypeArgs(astTy, chirTy);
}

bool CheckEnumType(const Codira::AST::Ty& astTy, const Type& chirTy)
{
    if (!astTy.IsEnum()) {
        return false;
    }
    auto astEnum = Codira::StaticCast<const Codira::AST::EnumTy&>(astTy).declPtr;
    auto chirEnum = Codira::StaticCast<const EnumType&>(chirTy).GetEnumDef();
    if (astEnum->mangledName != chirEnum->GetIdentifierWithoutPrefix()) {
        return false;
    }
    return CheckTypeArgs(astTy, chirTy);
}

bool CheckRawArrayType(const Codira::AST::Ty& astTy, const Type& chirTy)
{
    if (!astTy.IsArray()) {
        return false;
    }
    auto astArray = Codira::StaticCast<const Codira::AST::ArrayTy&>(astTy);
    auto& chirArray = Codira::StaticCast<const RawArrayType&>(chirTy);
    return astArray.dims == chirArray.GetDims() && CheckType(*astTy.typeArgs[0], *chirTy.GetTypeArgs()[0]);
}

bool CheckVArrayType(const Codira::AST::Ty& astTy, const Type& chirTy)
{
    if (astTy.kind != Codira::AST::TypeKind::TYPE_VARRAY) {
        return false;
    }
    auto astArray = Codira::StaticCast<const Codira::AST::VArrayTy&>(astTy);
    auto& chirArray = Codira::StaticCast<const VArrayType&>(chirTy);
    return astArray.size == chirArray.GetSize() && CheckType(*astTy.typeArgs[0], *chirTy.GetTypeArgs()[0]);
}

bool CheckCPointerType(const Codira::AST::Ty& astTy, const Type& chirTy)
{
    if (!astTy.IsPointer()) {
        return false;
    }
    return CheckType(*astTy.typeArgs[0], *chirTy.GetTypeArgs()[0]);
}

bool CheckType(const Codira::AST::Ty& astTy, const Type& chirTy)
{
    if (chirTy.IsPrimitive()) {
        return CheckPrimitiveType(astTy, chirTy);
    } else if (chirTy.IsTuple()) {
        return CheckTupleType(astTy, chirTy);
    } else if (chirTy.IsFunc()) {
        return CheckFuncType(astTy, chirTy);
    } else if (chirTy.IsStruct()) {
        return CheckStructType(astTy, chirTy);
    } else if (chirTy.IsClass()) {
        if (StaticCast<const ClassType&>(chirTy).GetClassDef()->IsClass()) {
            return CheckClassType(astTy, chirTy);
        } else {
            return CheckInterfaceType(astTy, chirTy);
        }
    } else if (chirTy.IsEnum()) {
        return CheckEnumType(astTy, chirTy);
    } else if (chirTy.IsRawArray()) {
        return CheckRawArrayType(astTy, chirTy);
    } else if (chirTy.IsVArray()) {
        return CheckVArrayType(astTy, chirTy);
    } else if (chirTy.IsCPointer()) {
        return CheckCPointerType(astTy, chirTy);
    } else if (chirTy.IsCString()) {
        return astTy.IsCString();
    } else if (chirTy.IsRef()) {
        return CheckType(astTy, *chirTy.GetTypeArgs()[0]);
    }
    return true;
}

bool CheckClass(const Codira::AST::ClassDecl& decl, const ClassDef& classDef)
{
    if (!classDef.IsClass()) {
        Errorln(classDef.GetIdentifier() + " is expected to be a classDef.");
        return false;
    }
    AST::ClassTy* astSupClsTy = nullptr;
    for (auto& super : decl.inheritedTypes) {
        if (super->ty->kind == AST::TypeKind::TYPE_CLASS) {
            astSupClsTy = StaticCast<AST::ClassTy*>(super->ty);
        }
    }
    auto chirSupClsTy = classDef.GetSuperClassTy();
    // check super class type
    if (astSupClsTy != nullptr && chirSupClsTy != nullptr) {
        if (!CheckType(*astSupClsTy, *chirSupClsTy)) {
            Errorln(classDef.GetIdentifier() + " set wrong super class.");
            return false;
        }
    } else if (astSupClsTy != nullptr) {
        Errorln(classDef.GetIdentifier() + " not set super class.");
        return false;
    } else if (chirSupClsTy != nullptr) {
        Errorln(classDef.GetIdentifier() + " set redundant super class.");
        return false;
    }
    return true;
}

bool CheckInterface(const ClassDef& chirNode)
{
    if (!chirNode.IsInterface()) {
        Errorln(chirNode.GetIdentifier() + " is expected to be a interfaceDef.");
        return false;
    }
    return true;
}

const CustomTypeDef* GetParentCustomTypeDef(const Value& value)
{
    if (auto func = DynamicCast<FuncBase>(&value)) {
        return func->GetParentCustomTypeDef();
    } else if (auto var = DynamicCast<GlobalVarBase>(&value)) {
        return var->GetParentCustomTypeDef();
    }
    return nullptr;
}

bool CheckInheritDeclGlobalMember(
    const Codira::AST::Decl& decl, const CustomTypeDef& chirNode, const AST2CHIRNodeMap<Value>& globalCache)
{
    auto chirCache = globalCache.TryGet(decl);
    if (chirCache == nullptr && decl.TestAttr(Codira::AST::Attribute::COMMON)) {
        return true;
    }
    if (chirCache == nullptr) {
        Errorln("not find " + decl.mangledName + " translator in " + chirNode.GetIdentifier() + ".");
        return false;
    }
    // skip checking temporarily
    if (decl.IsConst() && decl.TestAttr(AST::Attribute::IMPORTED)) {
        return true;
    }
    // check chir node have a right declaredParent
    auto funcDecl = DynamicCast<const Codira::AST::FuncDecl*>(&decl);
    if (funcDecl == nullptr || !IsStaticInit(*funcDecl)) {
        if (auto def = GetParentCustomTypeDef(*chirCache); def != &chirNode) {
            Errorln("not find " + chirCache->GetIdentifier() + " in " + chirNode.GetIdentifier() + ".");
            return false;
        }
    }
    // member func
    if (decl.astKind == Codira::AST::ASTKind::FUNC_DECL) {
        if (decl.TestAttr(Codira::AST::Attribute::PLATFORM) && chirNode.TestAttr(Attribute::DESERIALIZED)) {
            // `platform` function type can be subtype of `common` function type.
            // We keep origin type in CHIR, however AST type is updated. Thus it's not an error.
            return true;
        }
        if (!decl.TestAttr(Codira::AST::Attribute::STATIC) && !CheckMethodType(*decl.ty, *chirCache->GetType())) {
            Errorln(chirCache->GetIdentifier() + " is expected to be promoted " + Codira::AST::Ty::ToString(decl.ty) +
                ".");
            return false;
        }
        if (decl.TestAttr(Codira::AST::Attribute::STATIC) && !CheckFuncType(*decl.ty, *chirCache->GetType())) {
            Errorln(chirCache->GetIdentifier() + " is expected to be promoted " + Codira::AST::Ty::ToString(decl.ty) +
                ".");
            return false;
        }
    }
    return true;
}

bool CheckAbstractMethod(const Codira::AST::Decl& decl, const CustomTypeDef& chirNode)
{
    if (chirNode.GetCustomKind() != CustomDefKind::TYPE_CLASS) {
        return true;
    }
    if (decl.astKind == Codira::AST::ASTKind::PROP_DECL) {
        auto ret = true;
        auto& propDecl = Codira::StaticCast<Codira::AST::PropDecl&>(decl);
        for (auto& itp : propDecl.getters) {
            ret = CheckAbstractMethod(*itp, chirNode) && ret;
        }
        for (auto& itp : propDecl.setters) {
            ret = CheckAbstractMethod(*itp, chirNode) && ret;
        }
        return ret;
    }
    auto& classDef = Codira::StaticCast<const ClassDef&>(chirNode);
    for (auto& it : classDef.GetAbstractMethods()) {
        if (it.GetMangledName() != decl.mangledName + ".0") {
            continue;
        }
        auto res = true;
        if (it.TestAttr(Attribute::STATIC)) {
            res = CheckType(*decl.ty, *it.methodTy);
        } else {
            auto astTyArgs = decl.ty->typeArgs;
            auto chirTyArgs = it.methodTy->GetTypeArgs();
            if (astTyArgs.size() + 1 != chirTyArgs.size()) {
                res = false;
            } else {
                for (size_t i = 0; i < astTyArgs.size(); ++i) {
                    res = CheckType(*astTyArgs[i], *chirTyArgs[i + 1]) && res;
                }
            }
        }
        if (!res) {
            Errorln(it.GetMangledName() + " is expected to be " + Codira::AST::Ty::ToString(decl.ty) + " in " +
                chirNode.GetIdentifier() + ".");
            return false;
        }
        return true;
    }
    if (decl.platformImplementation && decl.platformImplementation->TestAttr(AST::Attribute::OPEN)) {
        // ABSTRACT COMMON was replaced with OPEN PLATFORM
        return true;
    }
    Errorln("not find abstract method " + decl.mangledName + " in " + chirNode.GetIdentifier() + ".");
    return false;
}

bool CheckLocalVar(const Codira::AST::Decl& decl, const CustomTypeDef& chirNode)
{
    auto localVars = chirNode.GetAllInstanceVars();
    if (chirNode.GetCustomKind() == CustomDefKind::TYPE_CLASS) {
        auto& classDef = static_cast<const ClassDef&>(chirNode);
        localVars = classDef.GetDirectInstanceVars();
    }
    for (auto& it : localVars) {
        if (it.name != decl.identifier.Val()) {
            continue;
        }
        if (!DynamicCast<AST::RefEnumTy*>(decl.ty) && !CheckType(*decl.ty, *it.type)) {
            Errorln(it.name + " is expected to be " + Codira::AST::Ty::ToString(decl.ty) + " in " +
                chirNode.GetIdentifier() + ".");
            return false;
        }
        return true;
    }
    Errorln("not find local var " + decl.mangledName + " in " + chirNode.GetIdentifier() + ".");
    return false;
}

bool CheckInheritDeclMembers(
    const Codira::AST::InheritableDecl& decl, const CustomTypeDef& chirNode, const AST2CHIRNodeMap<Value>& globalCache)
{
    auto ret = true;
    for (auto& it : decl.GetMemberDecls()) {
        // All of call to JArray constructors will be desugared, so we can skip the useless constructor member directly.
        if (it->TestAttr(Codira::AST::Attribute::GENERIC)) {
            continue;
        }
        // decl in Interface is abstract method.
        if (decl.astKind == Codira::AST::ASTKind::INTERFACE_DECL) {
            if (it->astKind != AST::ASTKind::VAR_DECL) {
                ret = CheckAbstractMethod(*it, chirNode) && ret;
            }
            continue;
        }
        // abstract method not have a real node
        if (it->TestAttr(Codira::AST::Attribute::ABSTRACT)) {
            ret = CheckAbstractMethod(*it, chirNode) && ret;
            continue;
        }
        if (decl.astKind == Codira::AST::ASTKind::INTERFACE_DECL && !it->TestAttr(Codira::AST::Attribute::STATIC) &&
            (it->astKind == Codira::AST::ASTKind::FUNC_DECL || it->astKind == Codira::AST::ASTKind::PROP_DECL)) {
            ret = CheckAbstractMethod(*it, chirNode) && ret;
            continue;
        }
        // local member var not have a real node
        if (it->astKind == Codira::AST::ASTKind::VAR_DECL && !it->TestAttr(Codira::AST::Attribute::STATIC)) {
            ret = CheckLocalVar(*it, chirNode) && ret;
            continue;
        }
        // primary ctor not have a cache
        if (it->astKind == Codira::AST::ASTKind::PRIMARY_CTOR_DECL) {
            continue;
        }
        if (it->astKind == Codira::AST::ASTKind::PROP_DECL) {
            auto& propDecl = Codira::StaticCast<Codira::AST::PropDecl&>(*it);
            for (auto& itp : propDecl.getters) {
                ret = CheckInheritDeclGlobalMember(*itp, chirNode, globalCache) && ret;
            }
            for (auto& itp : propDecl.setters) {
                ret = CheckInheritDeclGlobalMember(*itp, chirNode, globalCache) && ret;
            }
            continue;
        }
        // other all need a real node, contain method and static var.
        ret = CheckInheritDeclGlobalMember(*it, chirNode, globalCache) && ret;
    }
    return ret;
}

bool CheckClassLike(
    const Codira::AST::ClassLikeDecl& decl, const CustomTypeDef& chirNode, const AST2CHIRNodeMap<Value>& globalCache)
{
    if (chirNode.GetCustomKind() != CustomDefKind::TYPE_CLASS) {
        Errorln(chirNode.GetIdentifier() + " is expected to be a classDef/interfaceDef.");
        return false;
    }
    auto ret = true;
    auto& classDef = static_cast<const ClassDef&>(chirNode);
    // check super interface same
    auto astSupInter = decl.GetSuperInterfaceTys();
    auto chirSupClsInter = classDef.GetImplementedInterfaceDefs();
    if (astSupInter.size() != chirSupClsInter.size()) {
        Errorln(chirNode.GetIdentifier() + " set wrong super interfaces.");
        ret = false;
    }

    if (decl.astKind == Codira::AST::ASTKind::CLASS_DECL) {
        ret = CheckClass(static_cast<const Codira::AST::ClassDecl&>(decl), classDef) && ret;
    } else {
        ret = CheckInterface(classDef) && ret;
    }
    return CheckInheritDeclMembers(decl, chirNode, globalCache) && ret;
}

bool CheckEnum(
    const Codira::AST::EnumDecl& decl, const CustomTypeDef& chirNode, const AST2CHIRNodeMap<Value>& globalCache)
{
    if (chirNode.GetCustomKind() != CustomDefKind::TYPE_ENUM) {
        Errorln(chirNode.GetIdentifier() + " is expected to be a enumDef.");
        return false;
    }
    return CheckInheritDeclMembers(decl, chirNode, globalCache);
}

bool CheckStruct(
    const Codira::AST::StructDecl& decl, const CustomTypeDef& chirNode, const AST2CHIRNodeMap<Value>& globalCache)
{
    if (chirNode.GetCustomKind() != CustomDefKind::TYPE_STRUCT) {
        Errorln(chirNode.GetIdentifier() + " is expected to be a structDef.");
        return false;
    }
    return CheckInheritDeclMembers(decl, chirNode, globalCache);
}

bool CheckFunc(const Codira::AST::FuncDecl& decl, const Value& chirNode)
{
    if (!Is<FuncBase>(chirNode)) {
        Errorln(chirNode.GetIdentifier() + " is expected to be a func.");
        return false;
    }
    // not check member method, it will check in customDef
    if (decl.outerDecl != nullptr) {
        return true;
    }
    auto astTy = decl.ty;
    auto chirTy = chirNode.GetType();
    if (!CheckType(*astTy, *chirTy)) {
        bool report = true;
        if (decl.TestAttr(AST::Attribute::PLATFORM) && chirNode.TestAttr(Attribute::DESERIALIZED)) {
            // `platform` function type can be subtype of `common` function type.
            // We keep origin type in CHIR, however AST type is updated. Thus it's not an error.
            report = false;
        }

        if (report) {
            Errorln(chirNode.GetIdentifier() + " is expected to be " + Codira::AST::Ty::ToString(astTy) + ".");
            return false;
        }
    }
    return true;
}

bool CheckVar(const Codira::AST::VarDecl& decl, const Value& chirNode)
{
    if (!Is<GlobalVarBase>(chirNode)) {
        Errorln(chirNode.GetIdentifier() + " is expected to be a globalVar.");
        return false;
    }
    auto astTy = decl.ty;
    auto chirTy = chirNode.GetType();
    if (!CheckType(*astTy, *chirTy)) {
        Errorln(chirNode.GetIdentifier() + " is expected to be " + Codira::AST::Ty::ToString(astTy) + ".");
        return false;
    }
    return true;
}

/// Search for vtable method duplication.
//  O(n^2) is okay because of average table size
inline FuncBase* CheckVtableCopy(const std::vector<VirtualFuncInfo>& tableMethods)
{
    auto size = tableMethods.size();

    for (size_t i = 0; i < size; i++) {
        auto instance1 = tableMethods[i].instance;

        for (size_t j = i + 1; j < size; j++) {
            auto instance2 = tableMethods[j].instance;

            if (instance1 == instance2 && tableMethods[i].srcCodeIdentifier == tableMethods[j].srcCodeIdentifier) {
                return instance1;
            }
        }
    }

    return nullptr;
}

bool CheckVtableHasNoDuplicates(const CustomTypeDef& customTypeDef)
{
    for (auto& [srcTy, tableMethods] : customTypeDef.GetVTable()) {
        const auto duplicate = CheckVtableCopy(tableMethods);
        if (duplicate) {
            Errorln(customTypeDef.GetIdentifier() + " has method duplication in vtable: '"
                + duplicate->GetSrcCodeIdentifier() + "'.");

            return false;
        }
    }

    return true;
}
} // namespace

namespace Codira::CHIR {
bool AST2CHIRCheckCustomTypeDef(
    const AST::Node& astNode, const CustomTypeDef& chirNode, const AST2CHIRNodeMap<Value>& globalCache)
{
    if (!astNode.IsNominalDecl()) {
        InternalError("unsupported decl");
        return false;
    }
    auto ret = true;
    auto& decl = static_cast<const AST::Decl&>(astNode);
    if (decl.identifier != chirNode.GetSrcCodeIdentifier()) {
        Errorln(chirNode.GetIdentifier() + "'srcIdentifier is expected to be " + decl.identifier + ".");
        ret = false;
    }
    if (decl.mangledName != chirNode.GetIdentifierWithoutPrefix()) {
        Errorln(chirNode.GetIdentifier() + "'identifier is expected to be " + decl.mangledName + ".");
        ret = false;
    }

    // NOTE: measurements are accumulated
    Utils::ProfileRecorder::Start("AST to CHIR Translation", "Checking VTable duplication");
    CheckVtableHasNoDuplicates(chirNode);
    Utils::ProfileRecorder::Stop("AST to CHIR Translation", "Checking VTable duplication");
    if (astNode.astKind == AST::ASTKind::CLASS_DECL || astNode.astKind == AST::ASTKind::INTERFACE_DECL) {
        return CheckClassLike(static_cast<const AST::ClassLikeDecl&>(decl), chirNode, globalCache) && ret;
    } else if (astNode.astKind == AST::ASTKind::ENUM_DECL) {
        return CheckEnum(static_cast<const AST::EnumDecl&>(decl), chirNode, globalCache) && ret;
    } else if (astNode.astKind == AST::ASTKind::STRUCT_DECL) {
        return CheckStruct(static_cast<const AST::StructDecl&>(decl), chirNode, globalCache) && ret;
    }
    return ret;
}

bool AST2CHIRCheckValue(const AST::Node& astNode, const Value& chirNode)
{
    if (!astNode.IsDecl()) {
        return true;
    }
    auto& decl = static_cast<const AST::Decl&>(astNode);
    if (astNode.astKind == AST::ASTKind::VAR_DECL) {
        return CheckVar(static_cast<const AST::VarDecl&>(decl), chirNode);
    } else if (astNode.astKind == AST::ASTKind::FUNC_DECL) {
        return CheckFunc(static_cast<const AST::FuncDecl&>(decl), chirNode);
    }
    return true;
}
} // namespace Codira::CHIR
