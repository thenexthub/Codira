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

#include <gtest/gtest.h>

#include "annotation.h"
#include "annotation.cpp"

using namespace testing::ext;

namespace panda::pandasm {
class AnnotationDataTest : public testing::Test {
};

namespace {
constexpr const std::string_view TSTYPE_ANNO_RECORD_NAME = "_TestAnnotation";
constexpr const std::string_view TSTYPE_ANNO_ELEMENT_NAME = "_TestInstruction";
}

/**
 * @tc.name: annotation_test_001
 * @tc.desc: Verify the SetOrAddElementByIndex function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AnnotationDataTest, annotation_test_001, TestSize.Level1)
{
    ScalarValue insn_order(ScalarValue::Create<panda::pandasm::Value::Type::I32>(1));
    std::vector<panda::pandasm::ScalarValue> elements;
    elements.emplace_back(std::move(insn_order));

    ArrayValue array_value(panda::pandasm::Value::Type::I32, elements);
    AnnotationElement anno_element(TSTYPE_ANNO_ELEMENT_NAME, std::make_unique<ArrayValue>(array_value));
    AnnotationData annotationData(TSTYPE_ANNO_RECORD_NAME);
    annotationData.AddElement(std::move(anno_element));

    size_t ele_idx = 0;
    annotationData.SetOrAddElementByIndex(ele_idx, std::move(anno_element));
    EXPECT_EQ(annotationData.GetElements().size(), elements.size());

    ele_idx = elements.size();
    annotationData.SetOrAddElementByIndex(ele_idx, std::move(anno_element));
    EXPECT_GT(annotationData.GetElements().size(), elements.size());

    ScalarValue insn_order1(ScalarValue::Create<panda::pandasm::Value::Type::U1>(1U));
    elements.emplace_back(std::move(insn_order1));
    ArrayValue array_value_U1(panda::pandasm::Value::Type::U1, elements);
    AnnotationElement anno_element_U1(TSTYPE_ANNO_ELEMENT_NAME, std::make_unique<ArrayValue>(array_value_U1));
    anno_element = anno_element_U1;
    EXPECT_EQ(anno_element.GetName(), "_TestInstruction");
}

/**
 * @tc.name: annotation_test_002
 * @tc.desc: Verify the GetArrayTypeAsChar function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AnnotationDataTest, annotation_test_002, TestSize.Level1)
{
    char type = '1';
    char type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::U1);
    EXPECT_EQ(type_u1, type);

    type = '2';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::I8);
    EXPECT_EQ(type_u1, type);

    type = '3';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::U8);
    EXPECT_EQ(type_u1, type);

    type = '4';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::I16);
    EXPECT_EQ(type_u1, type);

    type = '5';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::U16);
    EXPECT_EQ(type_u1, type);

    type = '6';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::I32);
    EXPECT_EQ(type_u1, type);

    type = '7';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::U32);
    EXPECT_EQ(type_u1, type);

    type = '8';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::I64);
    EXPECT_EQ(type_u1, type);

    type = '9';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::U64);
    EXPECT_EQ(type_u1, type);

    type = 'A';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::F32);
    EXPECT_EQ(type_u1, type);

    type = 'B';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::F64);
    EXPECT_EQ(type_u1, type);

    type = 'C';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::STRING);
    EXPECT_EQ(type_u1, type);

    type = 'D';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::RECORD);
    EXPECT_EQ(type_u1, type);

    type = 'E';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::METHOD);
    EXPECT_EQ(type_u1, type);

    type = 'F';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::ENUM);
    EXPECT_EQ(type_u1, type);

    type = 'G';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::ANNOTATION);
    EXPECT_EQ(type_u1, type);

    type = 'H';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::ARRAY);
    EXPECT_EQ(type_u1, type);

    type = 'I';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::VOID);
    EXPECT_EQ(type_u1, type);

    type = 'J';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::METHOD_HANDLE);
    EXPECT_EQ(type_u1, type);

    type = '*';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::STRING_NULLPTR);
    EXPECT_EQ(type_u1, type);

    type = '#';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::LITERALARRAY);
    EXPECT_EQ(type_u1, type);

    type = '0';
    type_u1 = Value::GetTypeAsChar(panda::pandasm::Value::Type::UNKNOWN);
    EXPECT_EQ(type_u1, type);

    type = 'K';
    type_u1 = Value::GetArrayTypeAsChar(panda::pandasm::Value::Type::U1);
    EXPECT_EQ(type_u1, type);

    type = 'L';
    type_u1 = Value::GetArrayTypeAsChar(panda::pandasm::Value::Type::I8);
    EXPECT_EQ(type_u1, type);

    type = 'M';
    type_u1 = Value::GetArrayTypeAsChar(panda::pandasm::Value::Type::U8);
    EXPECT_EQ(type_u1, type);

    type = 'N';
    type_u1 = Value::GetArrayTypeAsChar(panda::pandasm::Value::Type::I16);
    EXPECT_EQ(type_u1, type);

    type = 'O';
    type_u1 = Value::GetArrayTypeAsChar(panda::pandasm::Value::Type::U16);
    EXPECT_EQ(type_u1, type);

    type = 'P';
    type_u1 = Value::GetArrayTypeAsChar(panda::pandasm::Value::Type::I32);
    EXPECT_EQ(type_u1, type);

    type = 'Q';
    type_u1 = Value::GetArrayTypeAsChar(panda::pandasm::Value::Type::U32);
    EXPECT_EQ(type_u1, type);

    type = 'R';
    type_u1 = Value::GetArrayTypeAsChar(panda::pandasm::Value::Type::I64);
    EXPECT_EQ(type_u1, type);

    type = 'S';
    type_u1 = Value::GetArrayTypeAsChar(panda::pandasm::Value::Type::U64);
    EXPECT_EQ(type_u1, type);

    type = 'T';
    type_u1 = Value::GetArrayTypeAsChar(panda::pandasm::Value::Type::F32);
    EXPECT_EQ(type_u1, type);

    type = 'U';
    type_u1 = Value::GetArrayTypeAsChar(panda::pandasm::Value::Type::F64);
    EXPECT_EQ(type_u1, type);

    type = 'V';
    type_u1 = Value::GetArrayTypeAsChar(panda::pandasm::Value::Type::STRING);
    EXPECT_EQ(type_u1, type);

    type = 'W';
    type_u1 = Value::GetArrayTypeAsChar(panda::pandasm::Value::Type::RECORD);
    EXPECT_EQ(type_u1, type);

    type = 'X';
    type_u1 = Value::GetArrayTypeAsChar(panda::pandasm::Value::Type::METHOD);
    EXPECT_EQ(type_u1, type);

    type = 'Y';
    type_u1 = Value::GetArrayTypeAsChar(panda::pandasm::Value::Type::ENUM);
    EXPECT_EQ(type_u1, type);

    type = 'Z';
    type_u1 = Value::GetArrayTypeAsChar(panda::pandasm::Value::Type::ANNOTATION);
    EXPECT_EQ(type_u1, type);

    type = '@';
    type_u1 = Value::GetArrayTypeAsChar(panda::pandasm::Value::Type::METHOD_HANDLE);
    EXPECT_EQ(type_u1, type);

    type = '0';
    type_u1 = Value::GetArrayTypeAsChar(panda::pandasm::Value::Type::UNKNOWN);
    EXPECT_EQ(type_u1, type);
}

/**
 * @tc.name: annotation_test_003
 * @tc.desc: Verify the GetCharAsType function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AnnotationDataTest, annotation_test_003, TestSize.Level1)
{
    panda::pandasm::Value::Type type = Value::GetCharAsType('1');
    EXPECT_EQ(panda::pandasm::Value::Type::U1, type);

    type = Value::GetCharAsType('2');
    EXPECT_EQ(panda::pandasm::Value::Type::I8, type);

    type = Value::GetCharAsType('3');
    EXPECT_EQ(panda::pandasm::Value::Type::U8, type);

    type = Value::GetCharAsType('4');
    EXPECT_EQ(panda::pandasm::Value::Type::I16, type);

    type = Value::GetCharAsType('5');
    EXPECT_EQ(panda::pandasm::Value::Type::U16, type);

    type = Value::GetCharAsType('6');
    EXPECT_EQ(panda::pandasm::Value::Type::I32, type);

    type = Value::GetCharAsType('7');
    EXPECT_EQ(panda::pandasm::Value::Type::U32, type);

    type = Value::GetCharAsType('8');
    EXPECT_EQ(panda::pandasm::Value::Type::I64, type);

    type = Value::GetCharAsType('9');
    EXPECT_EQ(panda::pandasm::Value::Type::U64, type);

    type = Value::GetCharAsType('A');
    EXPECT_EQ(panda::pandasm::Value::Type::F32, type);

    type = Value::GetCharAsType('B');
    EXPECT_EQ(panda::pandasm::Value::Type::F64, type);

    type = Value::GetCharAsType('C');
    EXPECT_EQ(panda::pandasm::Value::Type::STRING, type);

    type = Value::GetCharAsType('D');
    EXPECT_EQ(panda::pandasm::Value::Type::RECORD, type);

    type = Value::GetCharAsType('E');
    EXPECT_EQ(panda::pandasm::Value::Type::METHOD, type);

    type = Value::GetCharAsType('F');
    EXPECT_EQ(panda::pandasm::Value::Type::ENUM, type);

    type = Value::GetCharAsType('G');
    EXPECT_EQ(panda::pandasm::Value::Type::ANNOTATION, type);

    type = Value::GetCharAsType('H');
    EXPECT_EQ(panda::pandasm::Value::Type::ARRAY, type);

    type = Value::GetCharAsType('I');
    EXPECT_EQ(panda::pandasm::Value::Type::VOID, type);

    type = Value::GetCharAsType('J');
    EXPECT_EQ(panda::pandasm::Value::Type::METHOD_HANDLE, type);

    type = Value::GetCharAsType('*');
    EXPECT_EQ(panda::pandasm::Value::Type::STRING_NULLPTR, type);

    type = Value::GetCharAsType('0');
    EXPECT_EQ(panda::pandasm::Value::Type::UNKNOWN, type);

    type = Value::GetCharAsArrayType('K');
    EXPECT_EQ(panda::pandasm::Value::Type::U1, type);

    type = Value::GetCharAsArrayType('L');
    EXPECT_EQ(panda::pandasm::Value::Type::I8, type);

    type = Value::GetCharAsArrayType('M');
    EXPECT_EQ(panda::pandasm::Value::Type::U8, type);

    type = Value::GetCharAsArrayType('N');
    EXPECT_EQ(panda::pandasm::Value::Type::I16, type);

    type = Value::GetCharAsArrayType('O');
    EXPECT_EQ(panda::pandasm::Value::Type::U16, type);

    type = Value::GetCharAsArrayType('P');
    EXPECT_EQ(panda::pandasm::Value::Type::I32, type);

    type = Value::GetCharAsArrayType('Q');
    EXPECT_EQ(panda::pandasm::Value::Type::U32, type);

    type = Value::GetCharAsArrayType('R');
    EXPECT_EQ(panda::pandasm::Value::Type::I64, type);

    type = Value::GetCharAsArrayType('S');
    EXPECT_EQ(panda::pandasm::Value::Type::U64, type);

    type = Value::GetCharAsArrayType('T');
    EXPECT_EQ(panda::pandasm::Value::Type::F32, type);

    type = Value::GetCharAsArrayType('U');
    EXPECT_EQ(panda::pandasm::Value::Type::F64, type);

    type = Value::GetCharAsArrayType('V');
    EXPECT_EQ(panda::pandasm::Value::Type::STRING, type);

    type = Value::GetCharAsArrayType('W');
    EXPECT_EQ(panda::pandasm::Value::Type::RECORD, type);

    type = Value::GetCharAsArrayType('X');
    EXPECT_EQ(panda::pandasm::Value::Type::METHOD, type);

    type = Value::GetCharAsArrayType('Y');
    EXPECT_EQ(panda::pandasm::Value::Type::ENUM, type);

    type = Value::GetCharAsArrayType('Z');
    EXPECT_EQ(panda::pandasm::Value::Type::ANNOTATION, type);

    type = Value::GetCharAsArrayType('@');
    EXPECT_EQ(panda::pandasm::Value::Type::METHOD_HANDLE, type);

    type = Value::GetCharAsArrayType('0');
    EXPECT_EQ(panda::pandasm::Value::Type::UNKNOWN, type);
}

/**
 * @tc.name: annotation_test_004
 * @tc.desc: Verify the TypeToString function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AnnotationDataTest, annotation_test_004, TestSize.Level1)
{
    std::string type = "u1";
    std::string type_u1 = AnnotationElement::TypeToString(panda::pandasm::Value::Type::U1);
    EXPECT_EQ(type_u1, type);

    type = "i8";
    type_u1 = AnnotationElement::TypeToString(panda::pandasm::Value::Type::I8);
    EXPECT_EQ(type_u1, type);

    type = "u8";
    type_u1 = AnnotationElement::TypeToString(panda::pandasm::Value::Type::U8);
    EXPECT_EQ(type_u1, type);

    type = "i16";
    type_u1 = AnnotationElement::TypeToString(panda::pandasm::Value::Type::I16);
    EXPECT_EQ(type_u1, type);

    type = "u16";
    type_u1 = AnnotationElement::TypeToString(panda::pandasm::Value::Type::U16);
    EXPECT_EQ(type_u1, type);

    type = "i32";
    type_u1 = AnnotationElement::TypeToString(panda::pandasm::Value::Type::I32);
    EXPECT_EQ(type_u1, type);

    type = "u32";
    type_u1 = AnnotationElement::TypeToString(panda::pandasm::Value::Type::U32);
    EXPECT_EQ(type_u1, type);

    type = "i64";
    type_u1 = AnnotationElement::TypeToString(panda::pandasm::Value::Type::I64);
    EXPECT_EQ(type_u1, type);

    type = "u64";
    type_u1 = AnnotationElement::TypeToString(panda::pandasm::Value::Type::U64);
    EXPECT_EQ(type_u1, type);

    type = "f32";
    type_u1 = AnnotationElement::TypeToString(panda::pandasm::Value::Type::F32);
    EXPECT_EQ(type_u1, type);

    type = "f64";
    type_u1 = AnnotationElement::TypeToString(panda::pandasm::Value::Type::F64);
    EXPECT_EQ(type_u1, type);

    type = "string";
    type_u1 = AnnotationElement::TypeToString(panda::pandasm::Value::Type::STRING);
    EXPECT_EQ(type_u1, type);

    type = "class";
    type_u1 = AnnotationElement::TypeToString(panda::pandasm::Value::Type::RECORD);
    EXPECT_EQ(type_u1, type);

    type = "method";
    type_u1 = AnnotationElement::TypeToString(panda::pandasm::Value::Type::METHOD);
    EXPECT_EQ(type_u1, type);

    type = "enum";
    type_u1 = AnnotationElement::TypeToString(panda::pandasm::Value::Type::ENUM);
    EXPECT_EQ(type_u1, type);

    type = "annotation";
    type_u1 = AnnotationElement::TypeToString(panda::pandasm::Value::Type::ANNOTATION);
    EXPECT_EQ(type_u1, type);

    type = "array";
    type_u1 = AnnotationElement::TypeToString(panda::pandasm::Value::Type::ARRAY);
    EXPECT_EQ(type_u1, type);

    type = "void";
    type_u1 = AnnotationElement::TypeToString(panda::pandasm::Value::Type::VOID);
    EXPECT_EQ(type_u1, type);
}

/**
 * @tc.name: annotation_test_005
 * @tc.desc: Verify the InitScalarValue function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AnnotationDataTest, annotation_test_005, TestSize.Level1)
{
    ScalarValue insn_order(ScalarValue::Create<panda::pandasm::Value::Type::I32>(1));
    std::unique_ptr<ScalarValue> value = panda::pandasm::InitScalarValue(insn_order);
    EXPECT_NE(value, nullptr);

    ScalarValue insn_order_u8(ScalarValue::Create<panda::pandasm::Value::Type::U8>(1U));
    value = panda::pandasm::InitScalarValue(insn_order_u8);
    EXPECT_NE(value, nullptr);

    ScalarValue insn_order_u16(ScalarValue::Create<panda::pandasm::Value::Type::U16>(1U));
    value = panda::pandasm::InitScalarValue(insn_order_u16);
    EXPECT_NE(value, nullptr);

    ScalarValue insn_order_u32(ScalarValue::Create<panda::pandasm::Value::Type::U32>(1U));
    value = panda::pandasm::InitScalarValue(insn_order_u32);
    EXPECT_NE(value, nullptr);

    ScalarValue insn_order_u64(ScalarValue::Create<panda::pandasm::Value::Type::U64>(1U));
    value = panda::pandasm::InitScalarValue(insn_order_u64);
    EXPECT_NE(value, nullptr);

    ScalarValue insn_order_i8(ScalarValue::Create<panda::pandasm::Value::Type::I8>(1));
    value = panda::pandasm::InitScalarValue(insn_order_i8);
    EXPECT_NE(value, nullptr);

    ScalarValue insn_order_i16(ScalarValue::Create<panda::pandasm::Value::Type::I16>(1));
    value = panda::pandasm::InitScalarValue(insn_order_i16);
    EXPECT_NE(value, nullptr);

    ScalarValue insn_order_i64(ScalarValue::Create<panda::pandasm::Value::Type::I64>(1));
    value = panda::pandasm::InitScalarValue(insn_order_i64);
    EXPECT_NE(value, nullptr);

    ScalarValue insn_order_f32(ScalarValue::Create<panda::pandasm::Value::Type::F32>(1));
    value = panda::pandasm::InitScalarValue(insn_order_f32);
    EXPECT_NE(value, nullptr);

    ScalarValue insn_order_f64(ScalarValue::Create<panda::pandasm::Value::Type::F64>(1));
    value = panda::pandasm::InitScalarValue(insn_order_f64);
    EXPECT_NE(value, nullptr);

    ScalarValue insn_order_string(ScalarValue::Create<panda::pandasm::Value::Type::STRING>("test"));
    value = panda::pandasm::InitScalarValue(insn_order_string);
    EXPECT_NE(value, nullptr);

    ScalarValue insn_order_strings(ScalarValue::Create<panda::pandasm::Value::Type::STRING_NULLPTR>(1));
    value = panda::pandasm::InitScalarValue(insn_order_strings);
    EXPECT_NE(value, nullptr);

    panda::pandasm::Type type;
    ScalarValue insn_order_record(ScalarValue::Create<panda::pandasm::Value::Type::RECORD>(type));
    value = panda::pandasm::InitScalarValue(insn_order_record);
    EXPECT_NE(value, nullptr);

    ScalarValue insn_order_method(ScalarValue::Create<panda::pandasm::Value::Type::METHOD>("test"));
    value = panda::pandasm::InitScalarValue(insn_order_method);
    EXPECT_NE(value, nullptr);

    ScalarValue insn_order_enum(ScalarValue::Create<panda::pandasm::Value::Type::ENUM>("test"));
    value = panda::pandasm::InitScalarValue(insn_order_enum);
    EXPECT_NE(value, nullptr);

    ScalarValue insn_orders(ScalarValue::Create<panda::pandasm::Value::Type::I32>(1));
    std::vector<panda::pandasm::ScalarValue> elements;
    elements.emplace_back(std::move(insn_orders));

    ArrayValue array_value(panda::pandasm::Value::Type::I32, elements);
    AnnotationElement anno_element(TSTYPE_ANNO_ELEMENT_NAME, std::make_unique<ArrayValue>(array_value));
    AnnotationData annotationData(TSTYPE_ANNO_RECORD_NAME);
    ScalarValue insn_order_anno(ScalarValue::Create<panda::pandasm::Value::Type::ANNOTATION>(annotationData));
    value = panda::pandasm::InitScalarValue(insn_order_anno);
    EXPECT_NE(value, nullptr);

    ScalarValue insn_order_lite(ScalarValue::Create<panda::pandasm::Value::Type::LITERALARRAY>("test"));
    value = panda::pandasm::InitScalarValue(insn_order_lite);
    EXPECT_NE(value, nullptr);

    ArrayValue array_values(panda::pandasm::Value::Type::LITERALARRAY, elements);
    AnnotationElement element(TSTYPE_ANNO_ELEMENT_NAME, std::make_unique<ArrayValue>(array_values));
    std::unique_ptr<Value> val = panda::pandasm::making_value(element);
    EXPECT_NE(val, nullptr);

    elements.clear();
    ArrayValue array_values1(panda::pandasm::Value::Type::LITERALARRAY, elements);
    AnnotationElement element1(TSTYPE_ANNO_ELEMENT_NAME, std::make_unique<ArrayValue>(array_values1));
    std::unique_ptr<Value> val1 = panda::pandasm::making_value(element1);
    EXPECT_NE(val1, nullptr);
}
}
