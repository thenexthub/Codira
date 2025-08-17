//===-- Lower/Runtime.h -- Fortran runtime codegen interface ----*- C++ -*-===//
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
// Builder routines for constructing the FIR dialect of MLIR. As FIR is a
// dialect of MLIR, it makes extensive use of MLIR interfaces and MLIR's coding
// style (https://mlir.toolchain.org/getting_started/DeveloperGuide/) is used in this
// module.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_LOWER_RUNTIME_H
#define LANGUAGE_COMPABILITY_LOWER_RUNTIME_H

#include <optional>

namespace mlir {
class Location;
class Value;
} // namespace mlir

namespace fir {
class CharBoxValue;
class FirOpBuilder;
} // namespace fir

namespace language::Compability {

namespace parser {
struct EventPostStmt;
struct EventWaitStmt;
struct LockStmt;
struct NotifyWaitStmt;
struct PauseStmt;
struct StopStmt;
struct SyncAllStmt;
struct SyncImagesStmt;
struct SyncMemoryStmt;
struct SyncTeamStmt;
struct UnlockStmt;
} // namespace parser

namespace lower {

class AbstractConverter;

// Lowering of Fortran statement related runtime (other than IO and maths)

void genNotifyWaitStatement(AbstractConverter &,
                            const parser::NotifyWaitStmt &);
void genEventPostStatement(AbstractConverter &, const parser::EventPostStmt &);
void genEventWaitStatement(AbstractConverter &, const parser::EventWaitStmt &);
void genLockStatement(AbstractConverter &, const parser::LockStmt &);
void genFailImageStatement(AbstractConverter &);
void genStopStatement(AbstractConverter &, const parser::StopStmt &);
void genSyncAllStatement(AbstractConverter &, const parser::SyncAllStmt &);
void genSyncImagesStatement(AbstractConverter &,
                            const parser::SyncImagesStmt &);
void genSyncMemoryStatement(AbstractConverter &,
                            const parser::SyncMemoryStmt &);
void genSyncTeamStatement(AbstractConverter &, const parser::SyncTeamStmt &);
void genUnlockStatement(AbstractConverter &, const parser::UnlockStmt &);
void genPauseStatement(AbstractConverter &, const parser::PauseStmt &);

void genPointerAssociate(fir::FirOpBuilder &, mlir::Location,
                         mlir::Value pointer, mlir::Value target);
void genPointerAssociateRemapping(fir::FirOpBuilder &, mlir::Location,
                                  mlir::Value pointer, mlir::Value target,
                                  mlir::Value bounds, bool isMonomorphic);
void genPointerAssociateLowerBounds(fir::FirOpBuilder &, mlir::Location,
                                    mlir::Value pointer, mlir::Value target,
                                    mlir::Value lbounds);
} // namespace lower
} // namespace language::Compability

#endif // FORTRAN_LOWER_RUNTIME_H
