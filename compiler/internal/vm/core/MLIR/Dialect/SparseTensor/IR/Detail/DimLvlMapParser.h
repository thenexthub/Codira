//===- DimLvlMapParser.h - `DimLvlMap` parser -------------------*- C++ -*-===//
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

#ifndef MLIR_DIALECT_SPARSETENSOR_IR_DETAIL_DIMLVLMAPPARSER_H
#define MLIR_DIALECT_SPARSETENSOR_IR_DETAIL_DIMLVLMAPPARSER_H

#include "DimLvlMap.h"
#include "LvlTypeParser.h"

namespace mlir {
namespace sparse_tensor {
namespace ir_detail {

///
/// Parses the Sparse Tensor Encoding Attribute (STEA).
///
/// General syntax is as follows,
///
///   [s0, ...]     // optional forward decl sym-vars
///   {l0, ...}     // optional forward decl lvl-vars
///   (
///     d0 = ...,   // dim-var = dim-exp
///     ...
///   ) -> (
///     l0 = ...,   // lvl-var = lvl-exp
///     ...
///   )
///
/// with simplifications when variables are implicit.
///
class DimLvlMapParser final {
public:
  explicit DimLvlMapParser(AsmParser &parser) : parser(parser) {}

  // Parses the input for a sparse tensor dimension-level map
  // and returns the map on success.
  FailureOr<DimLvlMap> parseDimLvlMap();

private:
  /// Client code should prefer using `parseVarUsage`
  /// and `parseVarBinding` rather than calling this method directly.
  OptionalParseResult parseVar(VarKind vk, bool isOptional,
                               Policy creationPolicy, VarInfo::ID &id,
                               bool &didCreate);

  /// Parses a variable occurence which is a *use* of that variable.
  /// When a valid variable name is currently unused, if
  /// `requireKnown=true`, an error is raised; if `requireKnown=false`,
  /// a new unbound variable will be created.
  FailureOr<VarInfo::ID> parseVarUsage(VarKind vk, bool requireKnown);

  /// Parses a variable occurence which is a *binding* of that variable.
  /// The `requireKnown` parameter is for handling the binding of
  /// forward-declared variables.
  FailureOr<VarInfo::ID> parseVarBinding(VarKind vk, bool requireKnown = false);

  /// Parses an optional variable binding. When the next token is
  /// not a valid variable name, this will bind a new unnamed variable.
  /// The returned `bool` indicates whether a variable name was parsed.
  FailureOr<std::pair<Var, bool>>
  parseOptionalVarBinding(VarKind vk, bool requireKnown = false);

  /// Binds the given variable: both updating the `VarEnv` itself, and
  /// the `{dims,lvls}AndSymbols` lists (which will be passed
  /// to `AsmParser::parseAffineExpr`). This method is already called by the
  /// `parseVarBinding`/`parseOptionalVarBinding` methods, therefore should
  /// not need to be called elsewhere.
  Var bindVar(toolchain::SMLoc loc, VarInfo::ID id);

  ParseResult parseSymbolBindingList();
  ParseResult parseLvlVarBindingList();
  ParseResult parseDimSpec();
  ParseResult parseDimSpecList();
  FailureOr<LvlVar> parseLvlVarBinding(bool requireLvlVarBinding);
  ParseResult parseLvlSpec(bool requireLvlVarBinding);
  ParseResult parseLvlSpecList();

  AsmParser &parser;
  LvlTypeParser lvlTypeParser;
  VarEnv env;
  // The parser maintains the `{dims,lvls}AndSymbols` lists to avoid
  // the O(n^2) cost of repeatedly constructing them inside of the
  // `parse{Dim,Lvl}Spec` methods.
  SmallVector<std::pair<StringRef, AffineExpr>, 4> dimsAndSymbols;
  SmallVector<std::pair<StringRef, AffineExpr>, 4> lvlsAndSymbols;
  SmallVector<DimSpec> dimSpecs;
  SmallVector<LvlSpec> lvlSpecs;
};

} // namespace ir_detail
} // namespace sparse_tensor
} // namespace mlir

#endif // MLIR_DIALECT_SPARSETENSOR_IR_DETAIL_DIMLVLMAPPARSER_H
