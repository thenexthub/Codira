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

#include <cstdio>
#include <fstream>
#include <string>

#include "gtest/gtest.h"

#define private public
#define STATIC_ASSERT static_assert

#include "Codira/AST/ASTCasting.h"
#include "Codira/Sema/TypeManager.h"

using namespace Codira;
using namespace AST;

static const std::vector<AST::TypeKind> ignoredKind = {
    TypeKind::TYPE_INVALID, TypeKind::TYPE_ANY, TypeKind::TYPE_QUEST, TypeKind::TYPE_INITIAL};

class CastTyTests : public testing::Test {
protected:
    static void SetUpTestCase()
    {
        tyMap.emplace(TypeKind::TYPE_CSTRING, TypeManager::GetCStringTy());
        for (size_t i = 0; i < TypeManager::primitiveTys.size(); ++i) {
            auto kind = static_cast<TypeKind>(i + static_cast<int32_t>(TYPE_PRIMITIVE_MIN));
            tyMap.emplace(kind, &TypeManager::primitiveTys[i]);
        }
        tyPool.emplace_back(MakeOwned<ArrayTy>(tyMap[TypeKind::TYPE_INT8], 1));
        tyMap.emplace(TypeKind::TYPE_ARRAY, tyPool.back().get());
        tyPool.emplace_back(MakeOwned<VArrayTy>(tyMap[TypeKind::TYPE_INT8], 1));
        tyMap.emplace(TypeKind::TYPE_VARRAY, tyPool.back().get());

        tyPool.emplace_back(MakeOwned<PointerTy>(tyMap[TypeKind::TYPE_INT8]));
        tyMap.emplace(TypeKind::TYPE_POINTER, tyPool.back().get());

        std::vector<Ptr<Ty>> elemTys{tyMap[TypeKind::TYPE_INT8], tyMap[TypeKind::TYPE_INT16]};
        tyPool.emplace_back(MakeOwned<TupleTy>(elemTys));
        tyMap.emplace(TypeKind::TYPE_TUPLE, tyPool.back().get());

        tyPool.emplace_back(MakeOwned<FuncTy>(elemTys, tyMap[TypeKind::TYPE_INT8]));
        tyMap.emplace(TypeKind::TYPE_FUNC, tyPool.back().get());

        tyPool.emplace_back(MakeOwned<UnionTy>(Utils::VecToSet(elemTys)));
        tyMap.emplace(TypeKind::TYPE_UNION, tyPool.back().get());

        tyPool.emplace_back(MakeOwned<IntersectionTy>(Utils::VecToSet(elemTys)));
        tyMap.emplace(TypeKind::TYPE_INTERSECTION, tyPool.back().get());

        auto id = MakeOwned<InterfaceDecl>();
        tyPool.emplace_back(MakeOwned<InterfaceTy>("interface", *id, elemTys));
        tyMap.emplace(TypeKind::TYPE_INTERFACE, tyPool.back().get());
        astPool.emplace_back(std::move(id));

        auto cd = MakeOwned<ClassDecl>();
        tyPool.emplace_back(MakeOwned<ClassTy>("class", *cd, elemTys));
        tyMap.emplace(TypeKind::TYPE_CLASS, tyPool.back().get());
        astPool.emplace_back(std::move(cd));

        auto ed = MakeOwned<EnumDecl>();
        tyPool.emplace_back(MakeOwned<EnumTy>("enum", *ed, elemTys));
        tyMap.emplace(TypeKind::TYPE_ENUM, tyPool.back().get());
        astPool.emplace_back(std::move(ed));

        auto sd = MakeOwned<StructDecl>();
        tyPool.emplace_back(MakeOwned<StructTy>("struct", *sd, elemTys));
        tyMap.emplace(TypeKind::TYPE_STRUCT, tyPool.back().get());
        astPool.emplace_back(std::move(sd));

        auto tad = MakeOwned<TypeAliasDecl>();
        tyPool.emplace_back(MakeOwned<TypeAliasTy>("typealias", *tad, elemTys));
        tyMap.emplace(TypeKind::TYPE, tyPool.back().get());
        astPool.emplace_back(std::move(tad));

        auto gpd = MakeOwned<GenericParamDecl>();
        tyPool.emplace_back(MakeOwned<GenericsTy>("T", *gpd));
        tyMap.emplace(TypeKind::TYPE_GENERICS, tyPool.back().get());
        astPool.emplace_back(std::move(gpd));
    }

    static std::map<AST::TypeKind, Ptr<AST::Ty>> tyMap;
    static std::vector<OwnedPtr<AST::Ty>> tyPool;
    static std::vector<OwnedPtr<AST::Node>> astPool;
};

std::map<AST::TypeKind, Ptr<AST::Ty>> CastTyTests::tyMap = {};
std::vector<OwnedPtr<AST::Ty>> CastTyTests::tyPool = {};
std::vector<OwnedPtr<AST::Node>> CastTyTests::astPool = {};

TEST_F(CastTyTests, VerifyCastingCount)
{
    // Expect casting size is hardcode, should be changed manually when any new Ty is added.
    // NOTE: 'TYPE_INITIAL' is the lase enum value in 'TypeKind'.
    size_t totalCount = static_cast<uint8_t>(TypeKind::TYPE_INITIAL) + 1;
    EXPECT_TRUE(totalCount > ignoredKind.size());
    size_t size = totalCount - ignoredKind.size();
    EXPECT_EQ(size, 33);
    EXPECT_EQ(tyMap.size(), size);

    // Added ty's kind should same with key.
    for (auto [kind, ty] : tyMap) {
        EXPECT_EQ(ty->kind, kind);
    }
}

TEST_F(CastTyTests, VerifyMonoCasting)
{
    for (auto [_, ty] : tyMap) {
#define DEFINE_TY(TYPE, KIND)                                                                                          \
    EXPECT_EQ(DynamicCast<TYPE*>(ty), dynamic_cast<TYPE*>(ty.get()));                                                  \
    EXPECT_EQ(Is<TYPE*>(ty), dynamic_cast<TYPE*>(ty.get()) != nullptr);                                                \
    EXPECT_EQ(Is<TYPE>(ty), dynamic_cast<TYPE*>(ty.get()) != nullptr);

#include "TyKind.inc"
#undef DEFINE_TY
    }
}

TEST_F(CastTyTests, VerifyIntermediateCasting)
{
    for (auto [_, ty] : tyMap) {
        EXPECT_EQ(DynamicCast<ClassLikeTy*>(ty), dynamic_cast<ClassLikeTy*>(ty.get()));
        EXPECT_EQ(DynamicCast<PrimitiveTy*>(ty), dynamic_cast<PrimitiveTy*>(ty.get()));
        EXPECT_EQ(DynamicCast<RefEnumTy*>(ty), dynamic_cast<RefEnumTy*>(ty.get()));
        EXPECT_EQ(DynamicCast<ClassThisTy*>(ty), dynamic_cast<ClassThisTy*>(ty.get()));
        EXPECT_EQ(Is<ClassLikeTy*>(ty), dynamic_cast<ClassLikeTy*>(ty.get()) != nullptr);
        EXPECT_EQ(Is<ClassThisTy*>(ty), dynamic_cast<ClassThisTy*>(ty.get()) != nullptr);
        EXPECT_EQ(Is<PrimitiveTy*>(ty), dynamic_cast<PrimitiveTy*>(ty.get()) != nullptr);
        EXPECT_EQ(Is<RefEnumTy*>(ty), dynamic_cast<RefEnumTy*>(ty.get()) != nullptr);
    }
    auto ed = MakeOwned<EnumDecl>();
    RefEnumTy refEnumTy("enum", *ed, {});
    EXPECT_EQ(DynamicCast<EnumTy*>(&refEnumTy), dynamic_cast<EnumTy*>(&refEnumTy));
    EXPECT_EQ(DynamicCast<RefEnumTy*>(&refEnumTy), dynamic_cast<RefEnumTy*>(&refEnumTy));
    EXPECT_EQ(Is<EnumTy*>(&refEnumTy), dynamic_cast<EnumTy*>(&refEnumTy) != nullptr);
    EXPECT_EQ(Is<RefEnumTy*>(&refEnumTy), dynamic_cast<RefEnumTy*>(&refEnumTy) != nullptr);

    auto cd = MakeOwned<ClassDecl>();
    ClassThisTy thisTy("this", *cd, {});
    EXPECT_EQ(DynamicCast<ClassTy*>(&thisTy), dynamic_cast<ClassTy*>(&thisTy));
    EXPECT_EQ(DynamicCast<ClassThisTy*>(&thisTy), dynamic_cast<ClassThisTy*>(&thisTy));
    EXPECT_EQ(Is<ClassTy*>(&thisTy), dynamic_cast<ClassTy*>(&thisTy) != nullptr);
    EXPECT_EQ(Is<ClassThisTy*>(&thisTy), dynamic_cast<ClassThisTy*>(&thisTy) != nullptr);
}
