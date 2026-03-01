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

#include "annotation.h"

namespace ark::pandasm {

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) big switch case
std::unique_ptr<ScalarValue> InitScalarValue(const ScalarValue &scVal)
{
    std::unique_ptr<ScalarValue> copyVal;
    switch (scVal.GetType()) {
        case Value::Type::U1: {
            copyVal = std::make_unique<ScalarValue>(ScalarValue::Create<Value::Type::U1>(scVal.GetValue<uint8_t>()));
            break;
        }
        case Value::Type::U8: {
            copyVal = std::make_unique<ScalarValue>(ScalarValue::Create<Value::Type::U8>(scVal.GetValue<uint8_t>()));
            break;
        }
        case Value::Type::U16: {
            copyVal = std::make_unique<ScalarValue>(ScalarValue::Create<Value::Type::U16>(scVal.GetValue<uint16_t>()));
            break;
        }
        case Value::Type::U32: {
            copyVal = std::make_unique<ScalarValue>(ScalarValue::Create<Value::Type::U32>(scVal.GetValue<uint32_t>()));
            break;
        }
        case Value::Type::U64: {
            copyVal = std::make_unique<ScalarValue>(ScalarValue::Create<Value::Type::U64>(scVal.GetValue<uint64_t>()));
            break;
        }
        case Value::Type::I8: {
            copyVal = std::make_unique<ScalarValue>(ScalarValue::Create<Value::Type::I8>(scVal.GetValue<int8_t>()));
            break;
        }
        case Value::Type::I16: {
            copyVal = std::make_unique<ScalarValue>(ScalarValue::Create<Value::Type::I16>(scVal.GetValue<int16_t>()));
            break;
        }
        case Value::Type::I32: {
            copyVal = std::make_unique<ScalarValue>(ScalarValue::Create<Value::Type::I32>(scVal.GetValue<int32_t>()));
            break;
        }
        case Value::Type::I64: {
            copyVal = std::make_unique<ScalarValue>(ScalarValue::Create<Value::Type::I64>(scVal.GetValue<int64_t>()));
            break;
        }
        case Value::Type::F32: {
            copyVal = std::make_unique<ScalarValue>(ScalarValue::Create<Value::Type::F32>(scVal.GetValue<float>()));
            break;
        }
        case Value::Type::F64: {
            copyVal = std::make_unique<ScalarValue>(ScalarValue::Create<Value::Type::F64>(scVal.GetValue<double>()));
            break;
        }
        case Value::Type::STRING: {
            copyVal =
                std::make_unique<ScalarValue>(ScalarValue::Create<Value::Type::STRING>(scVal.GetValue<std::string>()));
            break;
        }
        case Value::Type::STRING_NULLPTR: {
            copyVal = std::make_unique<ScalarValue>(
                ScalarValue::Create<Value::Type::STRING_NULLPTR>(scVal.GetValue<int32_t>()));
            break;
        }
        case Value::Type::RECORD: {
            copyVal = std::make_unique<ScalarValue>(ScalarValue::Create<Value::Type::RECORD>(scVal.GetValue<Type>()));
            break;
        }
        case Value::Type::METHOD: {
            copyVal = std::make_unique<ScalarValue>(
                ScalarValue::Create<Value::Type::METHOD>(scVal.GetValue<std::string>(), scVal.IsStatic()));
            break;
        }
        case Value::Type::ENUM: {
            copyVal =
                std::make_unique<ScalarValue>(ScalarValue::Create<Value::Type::ENUM>(scVal.GetValue<std::string>()));
            break;
        }
        case Value::Type::ANNOTATION: {
            copyVal = std::make_unique<ScalarValue>(
                ScalarValue::Create<Value::Type::ANNOTATION>(scVal.GetValue<AnnotationData>()));
            break;
        }
        case Value::Type::LITERALARRAY: {
            copyVal = std::make_unique<ScalarValue>(
                ScalarValue::Create<Value::Type::LITERALARRAY>(scVal.GetValue<std::string>()));
            break;
        }
        default: {
            UNREACHABLE();
            copyVal = nullptr;
            break;
        }
    }
    return copyVal;
}

std::unique_ptr<Value> MakingValue(const AnnotationElement &annElem)
{
    std::unique_ptr<Value> copyVal;
    switch (annElem.GetValue()->GetType()) {
        case Value::Type::U1:
        case Value::Type::U8:
        case Value::Type::U16:
        case Value::Type::U32:
        case Value::Type::U64:
        case Value::Type::I8:
        case Value::Type::I16:
        case Value::Type::I32:
        case Value::Type::I64:
        case Value::Type::F32:
        case Value::Type::F64:
        case Value::Type::STRING:
        case Value::Type::STRING_NULLPTR:
        case Value::Type::RECORD:
        case Value::Type::ENUM:
        case Value::Type::ANNOTATION:
        case Value::Type::LITERALARRAY: {
            copyVal = InitScalarValue(*static_cast<ScalarValue *>(annElem.GetValue()));
            break;
        }
        case Value::Type::METHOD: {
            copyVal = InitScalarValue(*static_cast<ScalarValue *>(annElem.GetValue()));
            break;
        }
        case Value::Type::ARRAY: {
            Value::Type cType;
            auto *elemArr = static_cast<ArrayValue *>(annElem.GetValue());
            if (elemArr->GetValues().empty()) {
                cType = Value::Type::VOID;
            } else {
                cType = elemArr->GetValues().front().GetType();
            }
            std::vector<ScalarValue> scVals;
            for (const auto &scVal : elemArr->GetValues()) {
                scVals.push_back(*InitScalarValue(scVal));
            }
            copyVal = std::make_unique<ArrayValue>(cType, std::move(scVals));
            break;
        }
        default: {
            UNREACHABLE();
            copyVal = nullptr;
            break;
        }
    }
    return copyVal;
}

AnnotationElement::AnnotationElement(const AnnotationElement &annElem)
{
    this->value_ = MakingValue(annElem);
    this->name_ = annElem.GetName();
}

AnnotationElement &AnnotationElement::operator=(const AnnotationElement &annElem)
{
    if (this == &annElem) {
        return *this;
    }

    this->value_ = MakingValue(annElem);
    this->name_ = annElem.GetName();
    return *this;
}

ScalarValue *Value::GetAsScalar()
{
    ASSERT(!IsArray());
    return static_cast<ScalarValue *>(this);
}

const ScalarValue *Value::GetAsScalar() const
{
    ASSERT(!IsArray());
    return static_cast<const ScalarValue *>(this);
}

ArrayValue *Value::GetAsArray()
{
    ASSERT(IsArray());
    return static_cast<ArrayValue *>(this);
}

const ArrayValue *Value::GetAsArray() const
{
    ASSERT(IsArray());
    return static_cast<const ArrayValue *>(this);
}

/* static */
std::string AnnotationElement::TypeToString(Value::Type type)
{
    switch (type) {
        case Value::Type::U1:
            return "u1";
        case Value::Type::I8:
            return "i8";
        case Value::Type::U8:
            return "u8";
        case Value::Type::I16:
            return "i16";
        case Value::Type::U16:
            return "u16";
        case Value::Type::I32:
            return "i32";
        case Value::Type::U32:
            return "u32";
        case Value::Type::I64:
            return "i64";
        case Value::Type::U64:
            return "u64";
        case Value::Type::F32:
            return "f32";
        case Value::Type::F64:
            return "f64";
        case Value::Type::STRING:
            return "string";
        case Value::Type::RECORD:
            return "record";
        case Value::Type::METHOD:
            return "method";
        case Value::Type::ENUM:
            return "enum";
        case Value::Type::ANNOTATION:
            return "annotation";
        case Value::Type::ARRAY:
            return "array";
        case Value::Type::VOID:
            return "void";
        default: {
            UNREACHABLE();
            return "unknown";
        }
    }
}

void AnnotationData::DeleteAnnotationElementByName(const std::string_view &annotationElemName)
{
    auto annotation_elem_iter =
        std::find_if(elements_.begin(), elements_.end(), [&](pandasm::AnnotationElement &annotation_element) -> bool {
            return annotation_element.GetName() == annotationElemName;
        });
    if (annotation_elem_iter != elements_.end()) {
        (void)elements_.erase(annotation_elem_iter);
    }
}

void AnnotationData::EnumerateAnnotationElements(const std::function<void(AnnotationElement &)> &callback)
{
    for (auto &element : elements_) {
        callback(element);
    }
}

}  // namespace ark::pandasm
