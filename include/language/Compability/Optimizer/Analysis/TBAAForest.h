//===-- TBAAForest.h - A TBAA tree for each function -----------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_ANALYSIS_TBAA_FOREST_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_ANALYSIS_TBAA_FOREST_H

#include "language/Compability/Optimizer/Dialect/FIROpsSupport.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/MLIRContext.h"
#include "toolchain/ADT/DenseMap.h"
#include <string>

namespace fir {

//===----------------------------------------------------------------------===//
// TBAATree
//===----------------------------------------------------------------------===//
/// Per-function TBAA tree. Each tree contains branches for data (of various
/// kinds) and descriptor access
struct TBAATree {
  //===----------------------------------------------------------------------===//
  // TBAAForrest::TBAATree::SubtreeState
  //===----------------------------------------------------------------------===//
  /// This contains a TBAA subtree based on some parent. New tags can be added
  /// under the parent using getTag.
  class SubtreeState {
    friend TBAATree; // only allow construction by TBAATree
  public:
    SubtreeState() = delete;
    SubtreeState(const SubtreeState &) = delete;
    SubtreeState(SubtreeState &&) = default;

    mlir::LLVM::TBAATagAttr getTag(toolchain::StringRef uniqueId) const;

    /// Create a TBAA tag pointing to the root of this subtree,
    /// i.e. all the children tags will alias with this tag.
    mlir::LLVM::TBAATagAttr getTag() const;

    mlir::LLVM::TBAATypeDescriptorAttr getRoot() const { return parent; }

  private:
    SubtreeState(mlir::MLIRContext *ctx, std::string name,
                 mlir::LLVM::TBAANodeAttr grandParent)
        : parentId{std::move(name)}, context(ctx) {
      parent = mlir::LLVM::TBAATypeDescriptorAttr::get(
          context, parentId, mlir::LLVM::TBAAMemberAttr::get(grandParent, 0));
    }

    const std::string parentId;
    mlir::MLIRContext *const context;
    mlir::LLVM::TBAATypeDescriptorAttr parent;
  };

  /// A subtree for POINTER/TARGET variables data.
  /// Any POINTER variable must use a tag that points
  /// to the root of this subtree.
  /// A TARGET dummy argument must also point to this root.
  SubtreeState targetDataTree;
  /// A subtree for global variables data (e.g. user module variables).
  SubtreeState globalDataTree;
  /// A subtree for variables allocated via fir.alloca or fir.allocmem.
  SubtreeState allocatedDataTree;
  /// A subtree for subprogram's dummy arguments.
  /// It only contains children for the dummy arguments
  /// that are not POINTER/TARGET. They all do not conflict
  /// with each other and with any other data access, except
  /// with unknown data accesses (FIR alias analysis uses
  /// SourceKind::Indirect for sources of such accesses).
  SubtreeState dummyArgDataTree;
  /// A subtree for global variables descriptors.
  SubtreeState directDataTree;
  mlir::LLVM::TBAATypeDescriptorAttr anyAccessDesc;
  mlir::LLVM::TBAATypeDescriptorAttr boxMemberTypeDesc;
  mlir::LLVM::TBAATypeDescriptorAttr anyDataTypeDesc;

  // Structure of the created tree:
  //   Function root
  //   |
  //   "any access"
  //   |
  //   |- "descriptor member"
  //   |- "any data access"
  //      |
  //      |- "dummy arg data"
  //      |- "target data"
  //         |
  //         |- "allocated data"
  //         |- "direct data"
  //         |- "global data"
  static TBAATree buildTree(mlir::StringAttr functionName);

private:
  TBAATree(mlir::LLVM::TBAATypeDescriptorAttr anyAccess,
           mlir::LLVM::TBAATypeDescriptorAttr dataRoot,
           mlir::LLVM::TBAATypeDescriptorAttr boxMemberTypeDesc);
};

//===----------------------------------------------------------------------===//
// TBAAForrest
//===----------------------------------------------------------------------===//
/// Collection of TBAATrees, usually indexed by function (so that each function
/// has a different TBAATree)
class TBAAForrest {
public:
  explicit TBAAForrest(bool separatePerFunction = true)
      : separatePerFunction{separatePerFunction} {}

  inline const TBAATree &operator[](mlir::func::FuncOp func) {
    return getFuncTree(func.getSymNameAttr());
  }
  inline const TBAATree &operator[](mlir::LLVM::LLVMFuncOp func) {
    // the external name conversion pass may rename some functions. Their old
    // name must be used so that we add to the tbaa tree added in the FIR pass
    mlir::Attribute attr = func->getAttr(getInternalFuncNameAttrName());
    if (attr) {
      return getFuncTree(mlir::cast<mlir::StringAttr>(attr));
    }
    return getFuncTree(func.getSymNameAttr());
  }
  // Returns the TBAA tree associated with the scope enclosed
  // within the given function. With MLIR inlining, there may
  // be multiple scopes within a single function. It is the caller's
  // responsibility to provide unique name for the scope.
  // If the scope string is empty, returns the TBAA tree for the
  // "root" scope of the given function.
  inline const TBAATree &getFuncTreeWithScope(mlir::func::FuncOp func,
                                              toolchain::StringRef scope) {
    mlir::StringAttr name = func.getSymNameAttr();
    if (!scope.empty())
      name = mlir::StringAttr::get(name.getContext(),
                                   toolchain::Twine(name) + " - " + scope);
    return getFuncTree(name);
  }

private:
  const TBAATree &getFuncTree(mlir::StringAttr symName) {
    if (!separatePerFunction)
      symName = mlir::StringAttr::get(symName.getContext(), "");
    if (!trees.contains(symName))
      trees.insert({symName, TBAATree::buildTree(symName)});
    return trees.at(symName);
  }

  // Should each function use a different tree?
  const bool separatePerFunction;
  // TBAA tree per function
  toolchain::DenseMap<mlir::StringAttr, TBAATree> trees;
};

} // namespace fir

#endif // FORTRAN_OPTIMIZER_ANALYSIS_TBAA_FOREST_H
