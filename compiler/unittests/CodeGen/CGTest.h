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
 * This file declares the BasicTest in CG.
 */

#ifndef CODIRA_CG_TEST_H
#define CODIRA_CG_TEST_H
#include <gtest/gtest.h>

#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/CHIR/CHIRContext.h"
#include "Codira/CHIR/Type/ClassDef.h"
#include "Codira/CHIR/Type/Type.h"

using namespace Codira::CHIR;

class CGTestTemplate : public ::testing::Test {
protected:
    CGTestTemplate() : Test(){};
    void SetUp() override{};
    void TearDown() override{};
};

// To construct CHIR types
class MangleTypeTest : public CGTestTemplate {
protected:
    MangleTypeTest() : cctx(&fileNameMap), builder(cctx)
    {
        int8Ty = builder.GetInt8Ty();
        int16Ty = builder.GetInt16Ty();
        int32Ty = builder.GetInt32Ty();
        int64Ty = builder.GetInt64Ty();
        intNativeTy = builder.GetIntNativeTy();

        uint8Ty = builder.GetUInt8Ty();
        uint16Ty = builder.GetUInt16Ty();
        uint32Ty = builder.GetUInt32Ty();
        uint64Ty = builder.GetUInt64Ty();
        uintNativeTy = builder.GetUIntNativeTy();

        float16Ty = builder.GetFloat16Ty();
        float32Ty = builder.GetFloat32Ty();
        float64Ty = builder.GetFloat64Ty();

        runeTy = builder.GetType<RuneType>();
        boolTy = builder.GetType<BooleanType>();
        unitTy = builder.GetType<UnitType>();
        nothingTy = builder.GetType<NothingType>();
        cstringTy = builder.GetType<CStringType>();
    }

    std::unordered_map<unsigned int, std::string> fileNameMap;
    CHIRContext cctx;
    CHIRBuilder builder;

    Type *int8Ty, *int16Ty, *int32Ty, *int64Ty, *intNativeTy, *uint8Ty, *uint16Ty, *uint32Ty, *uint64Ty, *uintNativeTy;
    Type *float16Ty, *float32Ty, *float64Ty;
    Type* runeTy;
    Type* boolTy;
    Type* unitTy;
    Type* nothingTy;
    Type* cstringTy;
    const std::string testFile{"test.code"};
    DebugLocation defaultLoc{testFile, 1, {1, 1}, {1, 1}, {0}};
};
#endif // CODIRA_CG_TEST_H
