//===- SPIRVParsingUtils.h - MLIR SPIR-V Dialect Parsing Utilities --------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/SPIRV/IR/SPIRVAttributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/OpImplementation.h"

#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/ADT/FunctionExtras.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/StringRef.h"

#include <type_traits>

namespace mlir::spirv {
namespace AttrNames {

inline constexpr char kClusterSize[] = "cluster_size"; // no ODS generation
inline constexpr char kControl[] = "control";          // no ODS generation
inline constexpr char kFnNameAttrName[] = "fn";        // no ODS generation
inline constexpr char kSpecIdAttrName[] = "spec_id";   // no ODS generation

} // namespace AttrNames

template <typename Ty>
ArrayAttr getStrArrayAttrForEnumList(Builder &builder, ArrayRef<Ty> enumValues,
                                     function_ref<StringRef(Ty)> stringifyFn) {
  if (enumValues.empty()) {
    return nullptr;
  }
  SmallVector<StringRef, 1> enumValStrs;
  enumValStrs.reserve(enumValues.size());
  for (auto val : enumValues) {
    enumValStrs.emplace_back(stringifyFn(val));
  }
  return builder.getStrArrayAttr(enumValStrs);
}

/// Parses the next keyword in `parser` as an enumerant of the given
/// `EnumClass`.
template <typename EnumClass, typename ParserType>
ParseResult
parseEnumKeywordAttr(EnumClass &value, ParserType &parser,
                     StringRef attrName = spirv::attributeName<EnumClass>()) {
  StringRef keyword;
  auto loc = parser.getCurrentLocation();
  if (parser.parseKeyword(&keyword))
    return failure();

  if (std::optional<EnumClass> attr =
          spirv::symbolizeEnum<EnumClass>(keyword)) {
    value = *attr;
    return success();
  }
  return parser.emitError(loc, "invalid ")
         << attrName << " attribute specification: " << keyword;
}

/// Parses the next string attribute in `parser` as an enumerant of the given
/// `EnumClass`.
template <typename EnumClass>
ParseResult
parseEnumStrAttr(EnumClass &value, OpAsmParser &parser,
                 StringRef attrName = spirv::attributeName<EnumClass>()) {
  static_assert(std::is_enum_v<EnumClass>);
  Attribute attrVal;
  NamedAttrList attr;
  auto loc = parser.getCurrentLocation();
  if (parser.parseAttribute(attrVal, parser.getBuilder().getNoneType(),
                            attrName, attr))
    return failure();
  if (!isa<StringAttr>(attrVal))
    return parser.emitError(loc, "expected ")
           << attrName << " attribute specified as string";
  auto attrOptional =
      spirv::symbolizeEnum<EnumClass>(cast<StringAttr>(attrVal).getValue());
  if (!attrOptional)
    return parser.emitError(loc, "invalid ")
           << attrName << " attribute specification: " << attrVal;
  value = *attrOptional;
  return success();
}

/// Parses the next string attribute in `parser` as an enumerant of the given
/// `EnumClass` and inserts the enumerant into `state` as an 32-bit integer
/// attribute with the enum class's name as attribute name.
template <typename EnumAttrClass,
          typename EnumClass = typename EnumAttrClass::ValueType>
ParseResult
parseEnumStrAttr(EnumClass &value, OpAsmParser &parser, OperationState &state,
                 StringRef attrName = spirv::attributeName<EnumClass>()) {
  static_assert(std::is_enum_v<EnumClass>);
  if (parseEnumStrAttr(value, parser, attrName))
    return failure();
  state.addAttribute(attrName,
                     parser.getBuilder().getAttr<EnumAttrClass>(value));
  return success();
}

/// Parses the next keyword in `parser` as an enumerant of the given `EnumClass`
/// and inserts the enumerant into `state` as an 32-bit integer attribute with
/// the enum class's name as attribute name.
template <typename EnumAttrClass,
          typename EnumClass = typename EnumAttrClass::ValueType>
ParseResult
parseEnumKeywordAttr(EnumClass &value, OpAsmParser &parser,
                     OperationState &state,
                     StringRef attrName = spirv::attributeName<EnumClass>()) {
  static_assert(std::is_enum_v<EnumClass>);
  if (parseEnumKeywordAttr(value, parser))
    return failure();
  state.addAttribute(attrName,
                     parser.getBuilder().getAttr<EnumAttrClass>(value));
  return success();
}

ParseResult parseVariableDecorations(OpAsmParser &parser,
                                     OperationState &state);

} // namespace mlir::spirv
