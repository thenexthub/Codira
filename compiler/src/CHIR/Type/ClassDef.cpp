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

#include "Codira/CHIR/Type/ClassDef.h"

#include "Codira/CHIR/CHIRCasting.h"
#include "Codira/CHIR/Expression/Terminator.h"
#include "Codira/CHIR/ToStringUtils.h"
#include "Codira/CHIR/Type/Type.h"
#include "Codira/CHIR/Value.h"
#include "Codira/Utils/CheckUtils.h"

#include <iostream>
#include <sstream>

using namespace Codira::CHIR;

ClassDef::ClassDef(std::string srcCodeIdentifier, std::string identifier,
    std::string pkgName, bool isClass)
    : CustomTypeDef(srcCodeIdentifier, identifier, pkgName, CustomDefKind::TYPE_CLASS), isClass(isClass)
{
}

ClassDef* ClassDef::GetSuperClassDef() const
{
    return superClassTy ? superClassTy->GetClassDef() : nullptr;
}

bool ClassDef::HasSuperClass() const
{
    return GetSuperClassDef() != nullptr;
}

void ClassDef::PrintAbstractMethod(std::stringstream& ss) const
{
    for (auto& method : abstractMethods) {
        PrintIndent(ss);
        ss << method.attributeInfo.ToString();
        ss << "func " << method.methodName << ": " << method.methodTy->ToString() << "\n";
    }
}

void ClassDef::SetSuperClassTy(ClassType& ty)
{
    superClassTy = &ty;
}

std::string ClassDef::ToString() const
{
    std::stringstream ss;
    PrintAttrAndTitle(ss);
    ss << " {";
    PrintComment(ss);
    ss << "\n";

    PrintLocalVar(ss);
    PrintStaticVar(ss);
    PrintMethod(ss);
    PrintAbstractMethod(ss);
    PrintVTable(ss);
    ss << "}";
    return ss.str();
}

bool ClassDef::IsAbstract() const
{
    return TestAttr(CHIR::Attribute::ABSTRACT);
}

bool ClassDef::IsInterface() const
{
    return !isClass;
}

bool ClassDef::IsClass() const
{
    return isClass;
}

void ClassDef::SetAnnotation(bool value)
{
    isAnnotation = value;
}

bool ClassDef::IsAnnotation() const
{
    return isAnnotation;
}

ClassType* ClassDef::GetSuperClassTy() const
{
    return superClassTy;
}

FuncBase* ClassDef::GetFinalizer() const
{
    for (auto m : methods) {
        if (m->GetFuncKind() == FuncKind::FINALIZER) {
            return m;
        }
    }
    return nullptr;
}

void ClassDef::AddAbstractMethod(AbstractMethodInfo methodInfo, bool recordOrder)
{
    auto mangledName = methodInfo.GetASTMangledName();
    CODEC_ASSERT(!mangledName.empty());
    if (recordOrder) {
        if (std::find(allMethodMangledNames.begin(), allMethodMangledNames.end(), mangledName) ==
            allMethodMangledNames.end()) {
            allMethodMangledNames.emplace_back(mangledName);
        }
    }
    abstractMethods.emplace_back(std::move(methodInfo));
}

std::vector<AbstractMethodInfo> ClassDef::GetAbstractMethods() const
{
    return abstractMethods;
}

void ClassDef::SetAbstractMethods(const std::vector<AbstractMethodInfo>& methods)
{
    abstractMethods = methods;
}

void ClassDef::SetType(CustomType& ty)
{
    CODEC_ASSERT(ty.GetTypeKind() == Type::TypeKind::TYPE_CLASS);
    type = &ty;
}

ClassType* ClassDef::GetType() const
{
    return StaticCast<ClassType>(type);
}

void ClassDef::PrintComment(std::stringstream& ss) const
{
    CustomTypeDef::PrintComment(ss);
    AddCommaOrNot(ss);
    if (ss.str().empty()) {
        ss << " // ";
    }
    ss << "isAnnotation: " << BoolToString(isAnnotation);
}

void ClassDef::AddMethod(FuncBase* method, bool recordOrder)
{
    CustomTypeDef::AddMethod(method);
    auto mangledName = method->GetIdentifierWithoutPrefix();
    CODEC_ASSERT(!mangledName.empty());
    if (recordOrder) {
        if (std::find(allMethodMangledNames.begin(), allMethodMangledNames.end(), mangledName) ==
            allMethodMangledNames.end()) {
            allMethodMangledNames.emplace_back(mangledName);
        }
    }
}

const std::vector<std::string>& ClassDef::GetAllMethodMangledNames() const
{
    return allMethodMangledNames;
}

void ClassDef::SetAllMethodMangledNames(const std::vector<std::string>& names)
{
    allMethodMangledNames = names;
}
