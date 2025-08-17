//===-- Optimizer/Support/FIRContext.h --------------------------*- C++ -*-===//
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
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//
/// Setters and getters for associating context with an instance of a ModuleOp.
/// The context is typically set by the tool and needed in later stages to
/// determine how to correctly generate code.
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_SUPPORT_FIRCONTEXT_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_SUPPORT_FIRCONTEXT_H

#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/TargetParser/Triple.h"

namespace mlir {
class ModuleOp;
class Operation;
} // namespace mlir

namespace fir {
class KindMapping;
struct NameUniquer;

/// Set the target triple for the module. `triple` must not be deallocated while
/// module `mod` is still live.
void setTargetTriple(mlir::ModuleOp mod, toolchain::StringRef triple);

/// Get the Triple instance from the Module or return the default Triple.
toolchain::Triple getTargetTriple(mlir::ModuleOp mod);

/// Set the kind mapping for the module. `kindMap` must not be deallocated while
/// module `mod` is still live.
void setKindMapping(mlir::ModuleOp mod, KindMapping &kindMap);

/// Get the KindMapping instance from the Module. If none was set, returns a
/// default.
KindMapping getKindMapping(mlir::ModuleOp mod);

/// Get the KindMapping instance that is in effect for the specified
/// operation. The KindMapping is taken from the operation itself,
/// if the operation is a ModuleOp, or from its parent ModuleOp.
/// If a ModuleOp cannot be reached, the function returns default KindMapping.
KindMapping getKindMapping(mlir::Operation *op);

/// Set the target CPU for the module. `cpu` must not be deallocated while
/// module `mod` is still live.
void setTargetCPU(mlir::ModuleOp mod, toolchain::StringRef cpu);

/// Get the target CPU string from the Module or return a null reference.
toolchain::StringRef getTargetCPU(mlir::ModuleOp mod);

/// Sets whether Denormal Mode can be ignored or not for lowering of floating
/// point atomic operations.
void setAtomicIgnoreDenormalMode(mlir::ModuleOp mod, bool value);
/// Gets whether Denormal Mode can be ignored or not for lowering of floating
/// point atomic operations.
bool getAtomicIgnoreDenormalMode(mlir::ModuleOp mod);
/// Sets whether fine grained memory can be used or not for lowering of atomic
/// operations.
void setAtomicFineGrainedMemory(mlir::ModuleOp mod, bool value);
/// Gets whether fine grained memory can be used or not for lowering of atomic
/// operations.
bool getAtomicFineGrainedMemory(mlir::ModuleOp mod);
/// Sets whether remote memory can be used or not for lowering of atomic
/// operations.
void setAtomicRemoteMemory(mlir::ModuleOp mod, bool value);
/// Gets whether remote memory can be used or not for lowering of atomic
/// operations.
bool getAtomicRemoteMemory(mlir::ModuleOp mod);

/// Set the tune CPU for the module. `cpu` must not be deallocated while
/// module `mod` is still live.
void setTuneCPU(mlir::ModuleOp mod, toolchain::StringRef cpu);

/// Get the tune CPU string from the Module or return a null reference.
toolchain::StringRef getTuneCPU(mlir::ModuleOp mod);

/// Set the target features for the module.
void setTargetFeatures(mlir::ModuleOp mod, toolchain::StringRef features);

/// Get the target features from the Module.
mlir::LLVM::TargetFeaturesAttr getTargetFeatures(mlir::ModuleOp mod);

/// Set the compiler identifier for the module.
void setIdent(mlir::ModuleOp mod, toolchain::StringRef ident);

/// Get the compiler identifier from the Module.
toolchain::StringRef getIdent(mlir::ModuleOp mod);

/// Set the command line used in this invocation.
void setCommandline(mlir::ModuleOp mod, toolchain::StringRef cmdLine);

/// Get the command line used in this invocation.
toolchain::StringRef getCommandline(mlir::ModuleOp mod);

/// Helper for determining the target from the host, etc. Tools may use this
/// function to provide a consistent interpretation of the `--target=<string>`
/// command-line option.
/// An empty string ("") or "default" will specify that the default triple
/// should be used. "native" will specify that the host machine be used to
/// construct the triple.
std::string determineTargetTriple(toolchain::StringRef triple);

} // namespace fir

#endif // FORTRAN_OPTIMIZER_SUPPORT_FIRCONTEXT_H
