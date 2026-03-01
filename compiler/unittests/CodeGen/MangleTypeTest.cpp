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

#include "CGTest.h"
#include "Codira/CHIR/Type/Type.h"
#include "Codira/CodeGen/CGUtils.h"

using TypeKind = Codira::CHIR::Type::TypeKind;
using namespace Codira::CodeGen;

TEST_F(MangleTypeTest, BuiltinTypes)
{
    EXPECT_EQ(MangleType(*int8Ty), "a");
    EXPECT_EQ(MangleType(*int16Ty), "s");
    EXPECT_EQ(MangleType(*int32Ty), "i");
    EXPECT_EQ(MangleType(*int64Ty), "l");
    EXPECT_EQ(MangleType(*intNativeTy), "q");

    EXPECT_EQ(MangleType(*uint8Ty), "h");
    EXPECT_EQ(MangleType(*uint16Ty), "t");
    EXPECT_EQ(MangleType(*uint32Ty), "j");
    EXPECT_EQ(MangleType(*uint64Ty), "m");
    EXPECT_EQ(MangleType(*uintNativeTy), "r");

    EXPECT_EQ(MangleType(*float16Ty), "Dh");
    EXPECT_EQ(MangleType(*float32Ty), "f");
    EXPECT_EQ(MangleType(*float64Ty), "d");

    EXPECT_EQ(MangleType(*runeTy), "c");
    EXPECT_EQ(MangleType(*boolTy), "b");

    EXPECT_EQ(MangleType(*unitTy), "u");
    EXPECT_EQ(MangleType(*nothingTy), "n");
    EXPECT_EQ(MangleType(*cstringTy), "k");
}

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
TEST_F(MangleTypeTest, CustomTypes)
{
    // construct a ClassType a.Alpha
    auto classDef = builder.CreateClass(defaultLoc, "Alpha", "_CN1a5AlphaE", "a", true, true);
    auto classTy = builder.GetType<ClassType>(classDef);
    EXPECT_EQ(MangleType(*classTy), "_CCN1a5AlphaE");
    // construct the StructType: a.SomeStruct
    auto structDef = builder.CreateStruct(defaultLoc, "Some", "_CN1a10SomeStructE", "a", true);
    auto structTy = builder.GetType<StructType>(structDef);
    EXPECT_EQ(MangleType(*structTy), "Rrecord._CN1a10SomeStructE");
}
#endif

TEST_F(MangleTypeTest, FuncTypes)
{
    // construct a funcTy (Int64, Int64) -> Int64
    auto funcTy = builder.GetType<FuncType>(std::vector<Type*>{int64Ty, int64Ty}, int64Ty);
    EXPECT_EQ(MangleType(*funcTy), "lll");
}

TEST_F(MangleTypeTest, TupleTypes)
{
    // construct a tupleType (Int8, Int16, Int32)
    auto tupleTy = builder.GetType<TupleType>(std::vector<Type*>{int8Ty, int16Ty, int32Ty});
    EXPECT_EQ(MangleType(*tupleTy), "T3_asiE");
    // construct a tupleType ((Int8, Int16, Int32), Int64)
    auto nestedTupleTy = builder.GetType<TupleType>(std::vector<Type*>{tupleTy, int64Ty});
    EXPECT_EQ(MangleType(*nestedTupleTy), "T2_T3_asiElE");
}

TEST_F(MangleTypeTest, RawArrayTypes)
{
    // construct a RawArrayType Int8[3]
    auto rawArrayTy = builder.GetType<RawArrayType>(int8Ty, 3);
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    EXPECT_EQ(MangleType(*rawArrayTy), "A3_aE");
#endif
}

TEST_F(MangleTypeTest, RefTypes)
{
    // construct a RefType of Int8
    auto refTy = builder.GetType<RefType>(int8Ty);
    EXPECT_EQ(MangleType(*refTy), "a");
}
