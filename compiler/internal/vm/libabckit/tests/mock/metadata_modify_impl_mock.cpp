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

#ifndef ABCKIT_MODIFY_IMPL_MOCK
#define ABCKIT_MODIFY_IMPL_MOCK

#include "../../src/mock/abckit_mock.h"
#include "../../src/mock/mock_values.h"

#include "../../include/libabckit/c/metadata_core.h"

#include <gtest/gtest.h>

namespace libabckit::mock::metadata_modify {

// NOLINTBEGIN(readability-identifier-naming)

// ========================================
// File
// ========================================

// ========================================
// Module
// ========================================

// ========================================
// Class
// ========================================

// ========================================
// Interface
// ========================================

// ========================================
// Module Field
// ========================================

// ========================================
// Class Field
// ========================================

// ========================================
// Interface Field
// ========================================

// ========================================
// Enum Field
// ========================================

// ========================================
// AnnotationInterface
// ========================================

// ========================================
// Function
// ========================================

void FunctionSetGraph(AbckitCoreFunction *function, AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    EXPECT_TRUE(graph == DEFAULT_GRAPH);
}

// ========================================
// Annotation
// ========================================

// ========================================
// Type
// ========================================

AbckitType *CreateType(AbckitFile *file, AbckitTypeId id)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(id == DEFAULT_TYPE_ID);
    return DEFAULT_TYPE;
}

AbckitType *CreateReferenceType(AbckitFile *file, AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return DEFAULT_TYPE;
}

// ========================================
// Value
// ========================================

AbckitValue *CreateValueU1(AbckitFile *file, bool value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(value == DEFAULT_BOOL);
    return DEFAULT_VALUE;
}

AbckitValue *CreateValueInt(AbckitFile *file, int32_t value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(value == DEFAULT_I32);
    return DEFAULT_VALUE;
}

AbckitValue *CreateValueDouble(AbckitFile *file, double value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(value == DEFAULT_DOUBLE);
    return DEFAULT_VALUE;
}

AbckitValue *CreateValueString(AbckitFile *file, const char *value, size_t len)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(strncmp(value, DEFAULT_CONST_CHAR, DEFAULT_CONST_CHAR_SIZE) == 0);
    EXPECT_TRUE(len + 1 == DEFAULT_CONST_CHAR_SIZE);
    return DEFAULT_VALUE;
}

AbckitValue *CreateValueMethod(AbckitFile *file, AbckitCoreFunction *method)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(method == DEFAULT_CORE_FUNCTION);
    return DEFAULT_VALUE;
}

AbckitValue *CreateValueRecord(AbckitFile *file, AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return DEFAULT_VALUE;
}

AbckitValue *CreateLiteralArrayValue(AbckitFile *file, [[maybe_unused]] AbckitValue **value, size_t size)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(size == 1);
    return DEFAULT_VALUE;
}

// ========================================
// String
// ========================================

AbckitString *CreateString(AbckitFile *file, const char *value, size_t len)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(strncmp(value, DEFAULT_CONST_CHAR, DEFAULT_CONST_CHAR_SIZE) == 0);
    EXPECT_TRUE(len + 1 == DEFAULT_CONST_CHAR_SIZE);
    return DEFAULT_STRING;
}

// ========================================
// LiteralArray
// ========================================

AbckitLiteralArray *CreateLiteralArray(AbckitFile *file, AbckitLiteral **value, size_t size)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(*value == DEFAULT_LITERAL);
    EXPECT_TRUE(size == 1U);
    return DEFAULT_LITERAL_ARRAY;
}

AbckitLiteral *CreateLiteralBool(AbckitFile *file, bool value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(value == DEFAULT_BOOL);
    return DEFAULT_LITERAL;
}

AbckitLiteral *CreateLiteralU8(AbckitFile *file, uint8_t value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(value == DEFAULT_U8);
    return DEFAULT_LITERAL;
}

AbckitLiteral *CreateLiteralU16(AbckitFile *file, uint16_t value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(value == DEFAULT_U16);
    return DEFAULT_LITERAL;
}

AbckitLiteral *CreateLiteralMethodAffiliate(AbckitFile *file, uint16_t value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(value == DEFAULT_U16);
    return DEFAULT_LITERAL;
}
AbckitLiteral *CreateLiteralU32(AbckitFile *file, uint32_t value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(value == DEFAULT_U32);
    return DEFAULT_LITERAL;
}

AbckitLiteral *CreateLiteralU64(AbckitFile *file, uint64_t value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(value == DEFAULT_U64);
    return DEFAULT_LITERAL;
}

AbckitLiteral *CreateLiteralFloat(AbckitFile *file, float value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(value == DEFAULT_FLOAT);
    return DEFAULT_LITERAL;
}

AbckitLiteral *CreateLiteralDouble(AbckitFile *file, double value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(value == DEFAULT_DOUBLE);
    return DEFAULT_LITERAL;
}

AbckitLiteral *CreateLiteralLiteralArray(AbckitFile *file, AbckitLiteralArray *litarr)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(litarr == DEFAULT_LITERAL_ARRAY);
    return DEFAULT_LITERAL;
}

AbckitLiteral *CreateLiteralString(AbckitFile *file, const char *value, size_t len)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(strncmp(value, DEFAULT_CONST_CHAR, DEFAULT_CONST_CHAR_SIZE) == 0);
    EXPECT_TRUE(len + 1 == DEFAULT_CONST_CHAR_SIZE);
    return DEFAULT_LITERAL;
}

AbckitLiteral *CreateLiteralNullValue(AbckitFile *file)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    return DEFAULT_LITERAL;
}

AbckitLiteral *CreateLiteralMethod(AbckitFile *file, AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_LITERAL;
}

void TypeSetName(AbckitType *type, const char *name, size_t /*len*/)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    EXPECT_TRUE(name == DEFAULT_CONST_CHAR);
}

void TypeSetRank(AbckitType *type, size_t rank)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    EXPECT_TRUE(rank == DEFAULT_SIZE_T);
}

AbckitType *CreateUnionType(AbckitFile *file, AbckitType **types, size_t size)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(types == DEFAULT_TYPE_PTR);
    EXPECT_TRUE(size == DEFAULT_SIZE_T);
    return DEFAULT_TYPE;
}
// NOLINTEND(readability-identifier-naming)

static AbckitModifyApi g_modifyApiImpl = {

    // ========================================
    // File
    // ========================================

    // ========================================
    // Module
    // ========================================

    // ========================================
    // Class
    // ========================================

    // ========================================
    // Interface
    // ========================================

    // ========================================
    // Module Field
    // ========================================

    // ========================================
    // Class Field
    // ========================================

    // ========================================
    // Interface Field
    // ========================================

    // ========================================
    // Enum Field
    // ========================================

    // ========================================
    // AnnotationInterface
    // ========================================

    // ========================================
    // Function
    // ========================================

    FunctionSetGraph,

    // ========================================
    // Annotation
    // ========================================

    // ========================================
    // Type
    // ========================================

    CreateType,
    CreateReferenceType,
    TypeSetName,
    TypeSetRank,
    CreateUnionType,

    // ========================================
    // Value
    // ========================================

    CreateValueU1,
    CreateValueInt,
    CreateValueDouble,
    CreateValueString,
    CreateValueMethod,
    CreateValueRecord,
    CreateLiteralArrayValue,

    // ========================================
    // String
    // ========================================

    CreateString,

    // ========================================
    // LiteralArray
    // ========================================

    CreateLiteralArray,

    // ========================================
    // LiteralArray
    // ========================================

    CreateLiteralBool,
    CreateLiteralU8,
    CreateLiteralU16,
    CreateLiteralMethodAffiliate,
    CreateLiteralU32,
    CreateLiteralU64,
    CreateLiteralFloat,
    CreateLiteralDouble,
    CreateLiteralLiteralArray,
    CreateLiteralString,
    CreateLiteralMethod,
    CreateLiteralNullValue,
};

}  // namespace libabckit::mock::metadata_modify

AbckitModifyApi const *AbckitGetMockModifyApiImpl([[maybe_unused]] AbckitApiVersion version)
{
    return &libabckit::mock::metadata_modify::g_modifyApiImpl;
}

#endif  // ABCKIT_MODIFY_IMPL_MOCK
