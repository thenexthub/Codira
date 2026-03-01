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

#include "Codira/CHIR/Type/EnumDef.h"

#include <sstream>
#include "Codira/CHIR/CHIRCasting.h"
#include "Codira/CHIR/Expression/Terminator.h"
#include "Codira/CHIR/ToStringUtils.h"
#include "Codira/CHIR/Type/Type.h"
#include "Codira/CHIR/Value.h"

using namespace Codira::CHIR;

bool EnumDef::IsAllCtorsTrivial() const
{
    // if enum all ctor does not have params, it is a trivial enum
    for (auto& ctor : ctors) {
        if (ctor.funcType->GetParamTypes().size() != 0) {
            return false;
        }
    }
    return true;
}

static std::string PrintParamTypes(const std::vector<Type*>& paramTypes)
{
    if (paramTypes.empty()) {
        return "";
    }
    std::string str;
    str += "(";
    for (size_t i = 0; i < paramTypes.size(); ++i) {
        str += paramTypes[i]->ToString();
        if (i != paramTypes.size() - 1) {
            str += ", ";
        }
    }
    str += ")";
    return str;
}

void EnumDef::PrintConstructor(std::stringstream& ss) const
{
    for (auto& ctor : ctors) {
        PrintIndent(ss);
        ss << ctor.name << PrintParamTypes(ctor.funcType->GetParamTypes()) << "\n";
    }
    ss << "\n";
}

void EnumDef::PrintAttrAndTitle(std::stringstream& ss) const
{
    ss << attributeInfo.ToString();
    if (!IsExhaustive()) {
        ss << "[nonExhaustive] ";
    }
    ss << CustomTypeKindToString(*this) << " " << GetIdentifier() << GenericDefArgsToString();
    PrintParent(ss);
}

std::string EnumDef::ToString() const
{
    /* [public][generic][...] enum XXX {      // loc: xxx, genericDecl: xxx
       ^^^^^^^^^^^^^^ attr    ^^^^^^^^^ title  ^^^^^^^^^^^^^^^^^^ comment
           constructor
           method
           vtable
       }
    */
    std::stringstream ss;
    PrintAttrAndTitle(ss);
    ss << " {";
    PrintComment(ss);
    ss << "\n";
    PrintConstructor(ss); // has a \n in the end
    PrintMethod(ss);      // has a \n in the end
    PrintVTable(ss);      // has a \n in the end
    ss << "}";
    return ss.str();
}

void EnumDef::AddCtor(EnumCtorInfo ctor)
{
    ctors.emplace_back(ctor);
}

std::vector<EnumCtorInfo> EnumDef::GetCtors() const
{
    return ctors;
}

void EnumDef::SetCtors(const std::vector<EnumCtorInfo>& items)
{
    ctors = items;
}

EnumCtorInfo EnumDef::GetCtor(size_t index) const
{
    CODEC_ASSERT(ctors.size() > index);
    return ctors[index];
}

void EnumDef::SetType(CustomType& ty)
{
    CODEC_ASSERT(ty.GetTypeKind() == Type::TypeKind::TYPE_ENUM);
    type = &ty;
}

EnumType* EnumDef::GetType() const
{
    return StaticCast<EnumType>(type);
}

bool EnumDef::IsExhaustive() const
{
    return !nonExhaustive;
}
