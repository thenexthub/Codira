//===- BuiltinDialectBytecode.h - MLIR Bytecode Implementation --*- C++ -*-===//
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
//
// This header defines hooks into the builtin dialect bytecode implementation.
//
//===----------------------------------------------------------------------===//

#ifndef LIB_MLIR_IR_BUILTINDIALECTBYTECODE_H
#define LIB_MLIR_IR_BUILTINDIALECTBYTECODE_H

namespace mlir {
class BuiltinDialect;

namespace builtin_dialect_detail {
/// Add the interfaces necessary for encoding the builtin dialect components in
/// bytecode.
void addBytecodeInterface(BuiltinDialect *dialect);
} // namespace builtin_dialect_detail
} // namespace mlir

#endif // LIB_MLIR_IR_BUILTINDIALECTBYTECODE_H
