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

#include "Format/NodeFormatter/Expr/PrimitiveTypeExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

namespace {
const std::unordered_map<TypeKind, std::string> TYPEKIND_TO_STRING_MAP{
    {TypeKind::TYPE_UNIT, "Unit"},
    {TypeKind::TYPE_INT8, "Int8"},
    {TypeKind::TYPE_INT16, "Int16"},
    {TypeKind::TYPE_INT32, "Int32"},
    {TypeKind::TYPE_INT64, "Int64"},
    {TypeKind::TYPE_INT_NATIVE, "IntNative"},
    {TypeKind::TYPE_UINT8, "UInt8"},
    {TypeKind::TYPE_UINT16, "UInt16"},
    {TypeKind::TYPE_UINT32, "UInt32"},
    {TypeKind::TYPE_UINT64, "UInt64"},
    {TypeKind::TYPE_UINT_NATIVE, "UIntNative"},
    {TypeKind::TYPE_FLOAT16, "Float16"},
    {TypeKind::TYPE_FLOAT32, "Float32"},
    {TypeKind::TYPE_FLOAT64, "Float64"},
    {TypeKind::TYPE_RUNE, "Rune"},
    {TypeKind::TYPE_BOOLEAN, "Bool"},
    {TypeKind::TYPE_IDEAL_INT, "Int"},
    {TypeKind::TYPE_IDEAL_FLOAT, "Float"},
    {TypeKind::TYPE_TUPLE, "Tuple"},
    {TypeKind::TYPE_ENUM, "Enum"},
    {TypeKind::TYPE_FUNC, "Function"},
    {TypeKind::TYPE_ARRAY, "Array"},
    {TypeKind::TYPE_CLASS, "Class"},
    {TypeKind::TYPE_INTERFACE, "Interface"},
    {TypeKind::TYPE, "TypeAlias"},
    {TypeKind::TYPE_STRUCT, "Struct"},
    {TypeKind::TYPE_NOTHING, "Nothing"}
};
}

void Codira::Format::PrimitiveTypeExprFormatter::ASTToDoc(
    Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto primitiveTypeExpr = StaticAs<ASTKind::PRIMITIVE_TYPE_EXPR>(node);
    AddPrimitiveTypeExpr(doc, *primitiveTypeExpr, level);
}

void Codira::Format::PrimitiveTypeExprFormatter::AddPrimitiveTypeExpr(
    Doc& doc, const Codira::AST::PrimitiveTypeExpr& primitiveTypeExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    auto iter = TYPEKIND_TO_STRING_MAP.find(primitiveTypeExpr.typeKind);
    if (iter != TYPEKIND_TO_STRING_MAP.end()) {
        std::string typeStr = iter->second;
        doc.members.emplace_back(DocType::STRING, level, typeStr);
    } else {
        Error("Can't find type kind, Please report this to Codira Tools team.");
    }
}
} // namespace Codira::Format
