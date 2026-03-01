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

#include "Codira/CHIR/Type/ExtendDef.h"

#include <sstream>
#include "Codira/CHIR/CHIRCasting.h"
#include "Codira/CHIR/ToStringUtils.h"
#include "Codira/Utils/CastingTemplate.h"

using namespace Codira;
using namespace Codira::CHIR;

ExtendDef::ExtendDef(
    const std::string& identifier, const std::string& pkgName, std::vector<GenericType*> genericParams)
    : CustomTypeDef("", identifier, pkgName, CustomDefKind::TYPE_EXTEND), genericParams(genericParams)
{
}

CustomTypeDef* ExtendDef::GetExtendedCustomTypeDef() const
{
    if (auto customTy = DynamicCast<const CustomType*>(extendedType); customTy) {
        return customTy->GetCustomTypeDef();
    }
    return nullptr;
}

void ExtendDef::PrintAttrAndTitle(std::stringstream& ss) const
{
    ss << attributeInfo.ToString();
    std::string extendedTyStr;
    CODEC_NULLPTR_CHECK(extendedType);
    if (auto customTy = DynamicCast<const CustomType*>(extendedType); customTy) {
        extendedTyStr = customTy->GetCustomTypeDef()->GetIdentifier() + GenericInsArgsToString(*customTy);
    } else {
        extendedTyStr = extendedType->ToString();
    }
    ss << CustomTypeKindToString(*this) << GenericDefArgsToString() << " " << extendedTyStr;
    PrintParent(ss);
}

void ExtendDef::PrintComment(std::stringstream& ss) const
{
    CustomTypeDef::PrintComment(ss);
    AddCommaOrNot(ss);
    if (ss.str().empty()) {
        ss << " // ";
    }
    ss << "id: " << identifier;
}

void ExtendDef::RemoveParent(ClassType& parent)
{
    implementedInterfaceTys.erase(
        std::remove(implementedInterfaceTys.begin(), implementedInterfaceTys.end(), &parent),
        implementedInterfaceTys.end());
}

Type* ExtendDef::GetExtendedType() const
{
    CODEC_NULLPTR_CHECK(extendedType);
    return extendedType;
}

Type* ExtendDef::GetType() const
{
    return GetExtendedType();
}

void ExtendDef::SetExtendedType(Type& ty)
{
    extendedType = &ty;
}

void ExtendDef::SetType(CustomType& ty)
{
    (void)ty;
    CODEC_ABORT(); // extend decl doesn't have type
}

std::vector<GenericType*> ExtendDef::GetGenericTypeParams() const
{
    return genericParams;
}
