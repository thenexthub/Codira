//====- LoweringHelpers.h - Lowering helper functions ---------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
// This file declares helper functions for lowering from CIR to LLVM or MLIR.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_CIR_LOWERINGHELPERS_H
#define LANGUAGE_CORE_CIR_LOWERINGHELPERS_H

#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/Transforms/DialectConversion.h"
#include "language/Core/CIR/Dialect/IR/CIRDialect.h"

mlir::DenseElementsAttr
convertStringAttrToDenseElementsAttr(cir::ConstArrayAttr attr, mlir::Type type);

template <typename StorageTy> StorageTy getZeroInitFromType(mlir::Type ty);
template <> mlir::APInt getZeroInitFromType(mlir::Type ty);
template <> mlir::APFloat getZeroInitFromType(mlir::Type ty);

template <typename AttrTy, typename StorageTy>
void convertToDenseElementsAttrImpl(cir::ConstArrayAttr attr,
                                    toolchain::SmallVectorImpl<StorageTy> &values);

template <typename AttrTy, typename StorageTy>
mlir::DenseElementsAttr
convertToDenseElementsAttr(cir::ConstArrayAttr attr,
                           const toolchain::SmallVectorImpl<int64_t> &dims,
                           mlir::Type type);

std::optional<mlir::Attribute>
lowerConstArrayAttr(cir::ConstArrayAttr constArr,
                    const mlir::TypeConverter *converter);

mlir::Value getConstAPInt(mlir::OpBuilder &bld, mlir::Location loc,
                          mlir::Type typ, const toolchain::APInt &val);

mlir::Value getConst(mlir::OpBuilder &bld, mlir::Location loc, mlir::Type typ,
                     unsigned val);

mlir::Value createShL(mlir::OpBuilder &bld, mlir::Value lhs, unsigned rhs);

mlir::Value createAShR(mlir::OpBuilder &bld, mlir::Value lhs, unsigned rhs);

mlir::Value createAnd(mlir::OpBuilder &bld, mlir::Value lhs,
                      const toolchain::APInt &rhs);

mlir::Value createLShR(mlir::OpBuilder &bld, mlir::Value lhs, unsigned rhs);
#endif
