//===- TemporaryStack.h --- temporary stack runtime API calls ---*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_RUNTIME_TEMPORARYSTACK_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_RUNTIME_TEMPORARYSTACK_H

namespace mlir {
class Value;
class Location;
} // namespace mlir

namespace fir {
class FirOpBuilder;
}

namespace fir::runtime {

mlir::Value genCreateValueStack(mlir::Location loc, fir::FirOpBuilder &builder);

void genPushValue(mlir::Location loc, fir::FirOpBuilder &builder,
                  mlir::Value opaquePtr, mlir::Value boxValue);
void genValueAt(mlir::Location loc, fir::FirOpBuilder &builder,
                mlir::Value opaquePtr, mlir::Value i, mlir::Value retValueBox);

void genDestroyValueStack(mlir::Location loc, fir::FirOpBuilder &builder,
                          mlir::Value opaquePtr);

mlir::Value genCreateDescriptorStack(mlir::Location loc,
                                     fir::FirOpBuilder &builder);

void genPushDescriptor(mlir::Location loc, fir::FirOpBuilder &builder,
                       mlir::Value opaquePtr, mlir::Value boxValue);
void genDescriptorAt(mlir::Location loc, fir::FirOpBuilder &builder,
                     mlir::Value opaquePtr, mlir::Value i,
                     mlir::Value retValueBox);

void genDestroyDescriptorStack(mlir::Location loc, fir::FirOpBuilder &builder,
                               mlir::Value opaquePtr);
} // namespace fir::runtime
#endif // FORTRAN_OPTIMIZER_BUILDER_RUNTIME_TEMPORARYSTACK_H
