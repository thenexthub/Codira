//===- DILineTableFromLocations.cpp - -------------------------------------===//
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

#include "mlir/Dialect/LLVMIR/Transforms/Passes.h"

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "vm/core/BinaryFormat/Dwarf.h"
#include "vm/core/Support/Path.h"

namespace mlir {
namespace LLVM {
#define GEN_PASS_DEF_DISCOPEFORLLVMFUNCOPPASS
#include "mlir/Dialect/LLVMIR/Transforms/Passes.h.inc"
} // namespace LLVM
} // namespace mlir

using namespace mlir;

/// Attempt to extract a filename for the given loc.
static FileLineColLoc extractFileLoc(Location loc) {
  if (auto fileLoc = dyn_cast<FileLineColLoc>(loc))
    return fileLoc;
  if (auto nameLoc = dyn_cast<NameLoc>(loc))
    return extractFileLoc(nameLoc.getChildLoc());
  if (auto opaqueLoc = dyn_cast<OpaqueLoc>(loc))
    return extractFileLoc(opaqueLoc.getFallbackLocation());
  if (auto fusedLoc = dyn_cast<FusedLoc>(loc)) {
    for (auto loc : fusedLoc.getLocations()) {
      if (auto fileLoc = extractFileLoc(loc))
        return fileLoc;
    }
  }
  if (auto callerLoc = dyn_cast<CallSiteLoc>(loc))
    return extractFileLoc(callerLoc.getCaller());
  return FileLineColLoc();
}

/// Creates a DISubprogramAttr with the provided compile unit and attaches it
/// to the function. Does nothing when the function already has an attached
/// subprogram.
static void addScopeToFunction(LLVM::LLVMFuncOp llvmFunc,
                               LLVM::DICompileUnitAttr compileUnitAttr) {

  Location loc = llvmFunc.getLoc();
  if (loc->findInstanceOf<FusedLocWith<LLVM::DISubprogramAttr>>())
    return;

  MLIRContext *context = llvmFunc->getContext();

  // Filename and line associate to the function.
  LLVM::DIFileAttr fileAttr;
  int64_t line = 1;
  if (FileLineColLoc fileLoc = extractFileLoc(loc)) {
    line = fileLoc.getLine();
    StringRef inputFilePath = fileLoc.getFilename().getValue();
    fileAttr =
        LLVM::DIFileAttr::get(context, toolchain::sys::path::filename(inputFilePath),
                              toolchain::sys::path::parent_path(inputFilePath));
  } else {
    fileAttr = compileUnitAttr
                   ? compileUnitAttr.getFile()
                   : LLVM::DIFileAttr::get(context, "<unknown>", "");
  }
  auto subroutineTypeAttr =
      LLVM::DISubroutineTypeAttr::get(context, toolchain::dwarf::DW_CC_normal, {});

  // Figure out debug information (`subprogramFlags` and `compileUnitAttr`) to
  // attach to the function definition / declaration. External functions are
  // declarations only and are defined in a different compile unit, so mark
  // them appropriately in `subprogramFlags` and set an empty `compileUnitAttr`.
  DistinctAttr id;
  auto subprogramFlags = LLVM::DISubprogramFlags::Optimized;
  if (!llvmFunc.isExternal()) {
    id = DistinctAttr::create(UnitAttr::get(context));
    subprogramFlags |= LLVM::DISubprogramFlags::Definition;
  } else {
    compileUnitAttr = {};
  }
  auto funcNameAttr = llvmFunc.getNameAttr();
  auto subprogramAttr = LLVM::DISubprogramAttr::get(
      context, id, compileUnitAttr, fileAttr, funcNameAttr, funcNameAttr,
      fileAttr,
      /*line=*/line, /*scopeLine=*/line, subprogramFlags, subroutineTypeAttr,
      /*retainedNodes=*/{}, /*annotations=*/{});
  llvmFunc->setLoc(FusedLoc::get(context, {loc}, subprogramAttr));
}

// Get a nested loc for inlined functions.
static Location getNestedLoc(Operation *op, LLVM::DIScopeAttr scopeAttr,
                             Location calleeLoc) {
  auto calleeFileName = extractFileLoc(calleeLoc).getFilename();
  auto *context = op->getContext();
  LLVM::DIFileAttr calleeFileAttr =
      LLVM::DIFileAttr::get(context, toolchain::sys::path::filename(calleeFileName),
                            toolchain::sys::path::parent_path(calleeFileName));
  auto lexicalBlockFileAttr = LLVM::DILexicalBlockFileAttr::get(
      context, scopeAttr, calleeFileAttr, /*discriminator=*/0);
  Location loc = calleeLoc;
  // Recurse if the callee location is again a call site.
  if (auto callSiteLoc = dyn_cast<CallSiteLoc>(calleeLoc)) {
    auto nestedLoc = callSiteLoc.getCallee();
    loc = getNestedLoc(op, lexicalBlockFileAttr, nestedLoc);
  }
  return FusedLoc::get(context, {loc}, lexicalBlockFileAttr);
}

/// Adds DILexicalBlockFileAttr for operations with CallSiteLoc and operations
/// from different files than their containing function.
static void setLexicalBlockFileAttr(Operation *op) {
  Location opLoc = op->getLoc();

  if (auto callSiteLoc = dyn_cast<CallSiteLoc>(opLoc)) {
    auto callerLoc = callSiteLoc.getCaller();
    auto calleeLoc = callSiteLoc.getCallee();
    LLVM::DIScopeAttr scopeAttr;
    // We assemble the full inline stack so the parent of this loc must be a
    // function
    auto funcOp = op->getParentOfType<LLVM::LLVMFuncOp>();
    if (auto funcOpLoc = toolchain::dyn_cast_if_present<FusedLoc>(funcOp.getLoc())) {
      scopeAttr = cast<LLVM::DISubprogramAttr>(funcOpLoc.getMetadata());
      op->setLoc(
          CallSiteLoc::get(getNestedLoc(op, scopeAttr, calleeLoc), callerLoc));
    }

    return;
  }

  auto funcOp = op->getParentOfType<LLVM::LLVMFuncOp>();
  if (!funcOp)
    return;

  FileLineColLoc opFileLoc = extractFileLoc(opLoc);
  if (!opFileLoc)
    return;

  FileLineColLoc funcFileLoc = extractFileLoc(funcOp.getLoc());
  if (!funcFileLoc)
    return;

  StringRef opFile = opFileLoc.getFilename().getValue();
  StringRef funcFile = funcFileLoc.getFilename().getValue();

  // Handle cross-file operations: add DILexicalBlockFileAttr when the
  // operation's source file differs from its containing function.
  if (opFile != funcFile) {
    auto funcOpLoc = toolchain::dyn_cast_if_present<FusedLoc>(funcOp.getLoc());
    if (!funcOpLoc)
      return;
    auto scopeAttr = dyn_cast<LLVM::DISubprogramAttr>(funcOpLoc.getMetadata());
    if (!scopeAttr)
      return;

    auto *context = op->getContext();
    LLVM::DIFileAttr opFileAttr =
        LLVM::DIFileAttr::get(context, toolchain::sys::path::filename(opFile),
                              toolchain::sys::path::parent_path(opFile));

    LLVM::DILexicalBlockFileAttr lexicalBlockFileAttr =
        LLVM::DILexicalBlockFileAttr::get(context, scopeAttr, opFileAttr, 0);

    Location newLoc = FusedLoc::get(context, {opLoc}, lexicalBlockFileAttr);
    op->setLoc(newLoc);
  }
}

namespace {
/// Add a debug info scope to LLVMFuncOp that are missing it.
struct DIScopeForLLVMFuncOpPass
    : public LLVM::impl::DIScopeForLLVMFuncOpPassBase<
          DIScopeForLLVMFuncOpPass> {
  using Base::Base;

  void runOnOperation() override {
    ModuleOp module = getOperation();
    Location loc = module.getLoc();

    MLIRContext *context = &getContext();
    if (!context->getLoadedDialect<LLVM::LLVMDialect>()) {
      emitError(loc, "LLVM dialect is not loaded.");
      return signalPassFailure();
    }

    // Find a DICompileUnitAttr attached to a parent (the module for example),
    // otherwise create a default one.
    LLVM::DICompileUnitAttr compileUnitAttr;
    if (auto fusedCompileUnitAttr =
            module->getLoc()
                ->findInstanceOf<FusedLocWith<LLVM::DICompileUnitAttr>>()) {
      compileUnitAttr = fusedCompileUnitAttr.getMetadata();
    } else {
      LLVM::DIFileAttr fileAttr;
      if (FileLineColLoc fileLoc = extractFileLoc(loc)) {
        StringRef inputFilePath = fileLoc.getFilename().getValue();
        fileAttr = LLVM::DIFileAttr::get(
            context, toolchain::sys::path::filename(inputFilePath),
            toolchain::sys::path::parent_path(inputFilePath));
      } else {
        fileAttr = LLVM::DIFileAttr::get(context, "<unknown>", "");
      }

      compileUnitAttr = LLVM::DICompileUnitAttr::get(
          DistinctAttr::create(UnitAttr::get(context)), toolchain::dwarf::DW_LANG_C,
          fileAttr, StringAttr::get(context, "MLIR"),
          /*isOptimized=*/true, emissionKind);
    }

    module.walk<WalkOrder::PreOrder>([&](Operation *op) -> void {
      if (auto funcOp = dyn_cast<LLVM::LLVMFuncOp>(op)) {
        // Create subprograms for each function with the same distinct compile
        // unit.
        addScopeToFunction(funcOp, compileUnitAttr);
      } else {
        setLexicalBlockFileAttr(op);
      }
    });
  }
};

} // end anonymous namespace
