//===-- Assign.h - generate assignment runtime API calls --------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_RUNTIME_ASSIGN_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_RUNTIME_ASSIGN_H

namespace mlir {
class Value;
class Location;
} // namespace mlir

namespace fir {
class FirOpBuilder;
}

namespace fir::runtime {

/// Generate runtime call to assign \p sourceBox to \p destBox.
/// \p destBox must be a fir.ref<fir.box<T>> and \p sourceBox a fir.box<T>.
/// \p destBox Fortran descriptor may be modified if destBox is an allocatable
/// according to Fortran allocatable assignment rules, otherwise it is not
/// modified.
void genAssign(fir::FirOpBuilder &builder, mlir::Location loc,
               mlir::Value destBox, mlir::Value sourceBox);

/// Generate runtime call to AssignPolymorphic \p sourceBox to \p destBox.
/// \p destBox must be a fir.ref<fir.box<T>> and \p sourceBox a fir.box<T>.
/// \p destBox Fortran descriptor may be modified if destBox is an allocatable
/// according to Fortran allocatable assignment rules.
void genAssignPolymorphic(fir::FirOpBuilder &builder, mlir::Location loc,
                          mlir::Value destBox, mlir::Value sourceBox);

/// Generate runtime call to AssignExplicitLengthCharacter to assign
/// \p sourceBox to \p destBox where \p destBox is a whole allocatable character
/// with explicit or assumed length. After the assignment, the length of
/// \p destBox will remain what it was, even if allocation or reallocation
/// occurred. For assignments to a whole allocatable with deferred length,
/// genAssign should be used.
/// \p destBox must be a fir.ref<fir.box<T>> and \p sourceBox a fir.box<T>.
/// \p destBox Fortran descriptor may be modified if destBox is an allocatable
/// according to Fortran allocatable assignment rules.
void genAssignExplicitLengthCharacter(fir::FirOpBuilder &builder,
                                      mlir::Location loc, mlir::Value destBox,
                                      mlir::Value sourceBox);

/// Generate runtime call to assign \p sourceBox to \p destBox.
/// \p destBox must be a fir.ref<fir.box<T>> and \p sourceBox a fir.box<T>.
/// \p destBox Fortran descriptor may be modified if destBox is an allocatable
/// according to Fortran allocatable assignment rules, otherwise it is not
/// modified.
void genAssignTemporary(fir::FirOpBuilder &builder, mlir::Location loc,
                        mlir::Value destBox, mlir::Value sourceBox);

/// Generate runtime call to "CopyInAssign" runtime API.
void genCopyInAssign(fir::FirOpBuilder &builder, mlir::Location loc,
                     mlir::Value tempBoxAddr, mlir::Value varBoxAddr);
/// Generate runtime call to "CopyOutAssign" runtime API.
void genCopyOutAssign(fir::FirOpBuilder &builder, mlir::Location loc,
                      mlir::Value varBoxAddr, mlir::Value tempBoxAddr);

} // namespace fir::runtime
#endif // FORTRAN_OPTIMIZER_BUILDER_RUNTIME_ASSIGN_H
