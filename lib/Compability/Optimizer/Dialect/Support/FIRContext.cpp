//===-- FIRContext.cpp ----------------------------------------------------===//
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

#include "language/Compability/Optimizer/Dialect/Support/FIRContext.h"
#include "language/Compability/Optimizer/Dialect/Support/KindMapping.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinOps.h"
#include "toolchain/TargetParser/Host.h"

void fir::setTargetTriple(mlir::ModuleOp mod, toolchain::StringRef triple) {
  auto target = fir::determineTargetTriple(triple);
  mod->setAttr(mlir::LLVM::LLVMDialect::getTargetTripleAttrName(),
               mlir::StringAttr::get(mod.getContext(), target));
}

toolchain::Triple fir::getTargetTriple(mlir::ModuleOp mod) {
  if (auto target = mod->getAttrOfType<mlir::StringAttr>(
          mlir::LLVM::LLVMDialect::getTargetTripleAttrName()))
    return toolchain::Triple(target.getValue());
  return toolchain::Triple(toolchain::sys::getDefaultTargetTriple());
}

static constexpr const char *kindMapName = "fir.kindmap";
static constexpr const char *defKindName = "fir.defaultkind";

void fir::setKindMapping(mlir::ModuleOp mod, fir::KindMapping &kindMap) {
  auto *ctx = mod.getContext();
  mod->setAttr(kindMapName, mlir::StringAttr::get(ctx, kindMap.mapToString()));
  auto defs = kindMap.defaultsToString();
  mod->setAttr(defKindName, mlir::StringAttr::get(ctx, defs));
}

fir::KindMapping fir::getKindMapping(mlir::ModuleOp mod) {
  auto *ctx = mod.getContext();
  if (auto defs = mod->getAttrOfType<mlir::StringAttr>(defKindName)) {
    auto defVals = fir::KindMapping::toDefaultKinds(defs.getValue());
    if (auto maps = mod->getAttrOfType<mlir::StringAttr>(kindMapName))
      return fir::KindMapping(ctx, maps.getValue(), defVals);
    return fir::KindMapping(ctx, defVals);
  }
  return fir::KindMapping(ctx);
}

fir::KindMapping fir::getKindMapping(mlir::Operation *op) {
  auto moduleOp = mlir::dyn_cast<mlir::ModuleOp>(op);
  if (moduleOp)
    return getKindMapping(moduleOp);

  moduleOp = op->getParentOfType<mlir::ModuleOp>();
  return getKindMapping(moduleOp);
}

static constexpr const char *targetCpuName = "fir.target_cpu";

void fir::setTargetCPU(mlir::ModuleOp mod, toolchain::StringRef cpu) {
  if (cpu.empty())
    return;

  auto *ctx = mod.getContext();
  mod->setAttr(targetCpuName, mlir::StringAttr::get(ctx, cpu));
}

toolchain::StringRef fir::getTargetCPU(mlir::ModuleOp mod) {
  if (auto attr = mod->getAttrOfType<mlir::StringAttr>(targetCpuName))
    return attr.getValue();

  return {};
}

static constexpr const char *tuneCpuName = "fir.tune_cpu";

void fir::setTuneCPU(mlir::ModuleOp mod, toolchain::StringRef cpu) {
  if (cpu.empty())
    return;

  auto *ctx = mod.getContext();

  mod->setAttr(tuneCpuName, mlir::StringAttr::get(ctx, cpu));
}

static constexpr const char *atomicIgnoreDenormalModeName =
    "fir.atomic_ignore_denormal_mode";

void fir::setAtomicIgnoreDenormalMode(mlir::ModuleOp mod, bool value) {
  if (value) {
    auto *ctx = mod.getContext();
    mod->setAttr(atomicIgnoreDenormalModeName, mlir::UnitAttr::get(ctx));
  } else {
    if (mod->hasAttr(atomicIgnoreDenormalModeName))
      mod->removeAttr(atomicIgnoreDenormalModeName);
  }
}

bool fir::getAtomicIgnoreDenormalMode(mlir::ModuleOp mod) {
  return mod->hasAttr(atomicIgnoreDenormalModeName);
}

static constexpr const char *atomicFineGrainedMemoryName =
    "fir.atomic_fine_grained_memory";

void fir::setAtomicFineGrainedMemory(mlir::ModuleOp mod, bool value) {
  if (value) {
    auto *ctx = mod.getContext();
    mod->setAttr(atomicFineGrainedMemoryName, mlir::UnitAttr::get(ctx));
  } else {
    if (mod->hasAttr(atomicFineGrainedMemoryName))
      mod->removeAttr(atomicFineGrainedMemoryName);
  }
}

bool fir::getAtomicFineGrainedMemory(mlir::ModuleOp mod) {
  return mod->hasAttr(atomicFineGrainedMemoryName);
}

static constexpr const char *atomicRemoteMemoryName =
    "fir.atomic_remote_memory";

void fir::setAtomicRemoteMemory(mlir::ModuleOp mod, bool value) {
  if (value) {
    auto *ctx = mod.getContext();
    mod->setAttr(atomicRemoteMemoryName, mlir::UnitAttr::get(ctx));
  } else {
    if (mod->hasAttr(atomicRemoteMemoryName))
      mod->removeAttr(atomicRemoteMemoryName);
  }
}

bool fir::getAtomicRemoteMemory(mlir::ModuleOp mod) {
  return mod->hasAttr(atomicRemoteMemoryName);
}

toolchain::StringRef fir::getTuneCPU(mlir::ModuleOp mod) {
  if (auto attr = mod->getAttrOfType<mlir::StringAttr>(tuneCpuName))
    return attr.getValue();

  return {};
}

static constexpr const char *targetFeaturesName = "fir.target_features";

void fir::setTargetFeatures(mlir::ModuleOp mod, toolchain::StringRef features) {
  if (features.empty())
    return;

  auto *ctx = mod.getContext();
  mod->setAttr(targetFeaturesName,
               mlir::LLVM::TargetFeaturesAttr::get(ctx, features));
}

mlir::LLVM::TargetFeaturesAttr fir::getTargetFeatures(mlir::ModuleOp mod) {
  if (auto attr = mod->getAttrOfType<mlir::LLVM::TargetFeaturesAttr>(
          targetFeaturesName))
    return attr;

  return {};
}

void fir::setIdent(mlir::ModuleOp mod, toolchain::StringRef ident) {
  if (ident.empty())
    return;

  mlir::MLIRContext *ctx = mod.getContext();
  mod->setAttr(mlir::LLVM::LLVMDialect::getIdentAttrName(),
               mlir::StringAttr::get(ctx, ident));
}

toolchain::StringRef fir::getIdent(mlir::ModuleOp mod) {
  if (auto attr = mod->getAttrOfType<mlir::StringAttr>(
          mlir::LLVM::LLVMDialect::getIdentAttrName()))
    return attr;
  return {};
}

void fir::setCommandline(mlir::ModuleOp mod, toolchain::StringRef cmdLine) {
  if (cmdLine.empty())
    return;

  mlir::MLIRContext *ctx = mod.getContext();
  mod->setAttr(mlir::LLVM::LLVMDialect::getCommandlineAttrName(),
               mlir::StringAttr::get(ctx, cmdLine));
}

toolchain::StringRef fir::getCommandline(mlir::ModuleOp mod) {
  if (auto attr = mod->getAttrOfType<mlir::StringAttr>(
          mlir::LLVM::LLVMDialect::getCommandlineAttrName()))
    return attr;
  return {};
}

std::string fir::determineTargetTriple(toolchain::StringRef triple) {
  // Treat "" or "default" as stand-ins for the default machine.
  if (triple.empty() || triple == "default")
    return toolchain::sys::getDefaultTargetTriple();
  // Treat "native" as stand-in for the host machine.
  if (triple == "native")
    return toolchain::sys::getProcessTriple();
  // TODO: normalize the triple?
  return triple.str();
}
