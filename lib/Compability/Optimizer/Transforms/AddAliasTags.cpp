//===- AddAliasTags.cpp ---------------------------------------------------===//
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

//===----------------------------------------------------------------------===//
/// \file
/// Adds TBAA alias tags to fir loads and stores, based on information from
/// fir::AliasAnalysis. More are added later in CodeGen - see fir::TBAABuilder
//===----------------------------------------------------------------------===//

#include "language/Compability/Optimizer/Analysis/AliasAnalysis.h"
#include "language/Compability/Optimizer/Analysis/TBAAForest.h"
#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "language/Compability/Optimizer/Dialect/FirAliasTagOpInterface.h"
#include "language/Compability/Optimizer/Transforms/Passes.h"
#include "mlir/IR/Dominance.h"
#include "mlir/Pass/Pass.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ADT/Twine.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/raw_ostream.h"
#include <optional>

namespace fir {
#define GEN_PASS_DEF_ADDALIASTAGS
#include "language/Compability/Optimizer/Transforms/Passes.h.inc"
} // namespace fir

#define DEBUG_TYPE "fir-add-alias-tags"

static toolchain::cl::opt<bool>
    enableDummyArgs("dummy-arg-tbaa", toolchain::cl::init(true), toolchain::cl::Hidden,
                    toolchain::cl::desc("Add TBAA tags to dummy arguments"));
static toolchain::cl::opt<bool>
    enableGlobals("globals-tbaa", toolchain::cl::init(true), toolchain::cl::Hidden,
                  toolchain::cl::desc("Add TBAA tags to global variables"));
static toolchain::cl::opt<bool>
    enableDirect("direct-tbaa", toolchain::cl::init(true), toolchain::cl::Hidden,
                 toolchain::cl::desc("Add TBAA tags to direct variables"));
static toolchain::cl::opt<bool>
    enableLocalAllocs("local-alloc-tbaa", toolchain::cl::init(true),
                      toolchain::cl::Hidden,
                      toolchain::cl::desc("Add TBAA tags to local allocations."));

// Engineering option to triage TBAA tags attachment for accesses
// of allocatable entities.
static toolchain::cl::opt<unsigned> localAllocsThreshold(
    "local-alloc-tbaa-threshold", toolchain::cl::init(0), toolchain::cl::ReallyHidden,
    toolchain::cl::desc("If present, stops generating TBAA tags for accesses of "
                   "local allocations after N accesses in a module"));

namespace {

/// Shared state per-module
class PassState {
public:
  PassState(mlir::DominanceInfo &domInfo,
            std::optional<unsigned> localAllocsThreshold)
      : domInfo(domInfo), localAllocsThreshold(localAllocsThreshold) {}
  /// memoised call to fir::AliasAnalysis::getSource
  inline const fir::AliasAnalysis::Source &getSource(mlir::Value value) {
    if (!analysisCache.contains(value))
      analysisCache.insert(
          {value, analysis.getSource(value, /*getInstantiationPoint=*/true)});
    return analysisCache[value];
  }

  /// get the per-function TBAATree for this function
  inline const fir::TBAATree &getFuncTree(mlir::func::FuncOp func) {
    return forrest[func];
  }
  inline const fir::TBAATree &getFuncTreeWithScope(mlir::func::FuncOp func,
                                                   fir::DummyScopeOp scope) {
    auto &scopeMap = scopeNames.at(func);
    return forrest.getFuncTreeWithScope(func, scopeMap.lookup(scope));
  }

  void processFunctionScopes(mlir::func::FuncOp func);
  // For the given fir.declare returns the dominating fir.dummy_scope
  // operation.
  fir::DummyScopeOp getDeclarationScope(fir::DeclareOp declareOp) const;
  // For the given fir.declare returns the outermost fir.dummy_scope
  // in the current function.
  fir::DummyScopeOp getOutermostScope(fir::DeclareOp declareOp) const;
  // Returns true, if the given type of a memref of a FirAliasTagOpInterface
  // operation is a descriptor or contains a descriptor
  // (e.g. !fir.ref<!fir.type<Derived{f:!fir.box<!fir.heap<f32>>}>>).
  bool typeReferencesDescriptor(mlir::Type type);

  // Returns true if we can attach a TBAA tag to an access of an allocatable
  // entities. It checks if localAllocsThreshold allows the next tag
  // attachment.
  bool attachLocalAllocTag();

private:
  mlir::DominanceInfo &domInfo;
  fir::AliasAnalysis analysis;
  toolchain::DenseMap<mlir::Value, fir::AliasAnalysis::Source> analysisCache;
  fir::TBAAForrest forrest;
  // Unique names for fir.dummy_scope operations within
  // the given function.
  toolchain::DenseMap<mlir::func::FuncOp,
                 toolchain::DenseMap<fir::DummyScopeOp, std::string>>
      scopeNames;
  // A map providing a vector of fir.dummy_scope operations
  // for the given function. The vectors are sorted according
  // to the dominance information.
  toolchain::DenseMap<mlir::func::FuncOp, toolchain::SmallVector<fir::DummyScopeOp, 16>>
      sortedScopeOperations;

  // Local pass cache for derived types that contain descriptor
  // member(s), to avoid the cost of isRecordWithDescriptorMember().
  toolchain::DenseSet<mlir::Type> typesContainingDescriptors;

  std::optional<unsigned> localAllocsThreshold;
};

// Process fir.dummy_scope operations in the given func:
// sort them according to the dominance information, and
// associate a unique (within the current function) scope name
// with each of them.
void PassState::processFunctionScopes(mlir::func::FuncOp func) {
  if (scopeNames.contains(func))
    return;

  auto &scopeMap = scopeNames[func];
  auto &scopeOps = sortedScopeOperations[func];
  func.walk([&](fir::DummyScopeOp op) { scopeOps.push_back(op); });
  toolchain::stable_sort(scopeOps, [&](const fir::DummyScopeOp &op1,
                                  const fir::DummyScopeOp &op2) {
    return domInfo.properlyDominates(&*op1, &*op2);
  });
  unsigned scopeId = 0;
  for (auto scope : scopeOps) {
    if (scopeId != 0) {
      std::string name = (toolchain::Twine("Scope ") + toolchain::Twine(scopeId)).str();
      LLVM_DEBUG(toolchain::dbgs() << "Creating scope '" << name << "':\n"
                              << scope << "\n");
      scopeMap.insert({scope, std::move(name)});
    }
    ++scopeId;
  }
}

fir::DummyScopeOp
PassState::getDeclarationScope(fir::DeclareOp declareOp) const {
  auto func = declareOp->getParentOfType<mlir::func::FuncOp>();
  assert(func && "fir.declare does not have parent func.func");
  auto &scopeOps = sortedScopeOperations.at(func);
  for (auto II = scopeOps.rbegin(), IE = scopeOps.rend(); II != IE; ++II) {
    if (domInfo.dominates(&**II, &*declareOp))
      return *II;
  }
  return nullptr;
}

fir::DummyScopeOp PassState::getOutermostScope(fir::DeclareOp declareOp) const {
  auto func = declareOp->getParentOfType<mlir::func::FuncOp>();
  assert(func && "fir.declare does not have parent func.func");
  auto &scopeOps = sortedScopeOperations.at(func);
  if (!scopeOps.empty())
    return scopeOps[0];
  return nullptr;
}

bool PassState::typeReferencesDescriptor(mlir::Type type) {
  type = fir::unwrapAllRefAndSeqType(type);
  if (mlir::isa<fir::BaseBoxType>(type))
    return true;

  if (mlir::isa<fir::RecordType>(type)) {
    if (typesContainingDescriptors.contains(type))
      return true;
    if (fir::isRecordWithDescriptorMember(type)) {
      typesContainingDescriptors.insert(type);
      return true;
    }
  }
  return false;
}

bool PassState::attachLocalAllocTag() {
  if (!localAllocsThreshold)
    return true;
  if (*localAllocsThreshold == 0) {
    LLVM_DEBUG(toolchain::dbgs().indent(2)
               << "WARN: not assigning TBAA tag for an allocated entity access "
                  "due to the threshold\n");
    return false;
  }
  --*localAllocsThreshold;
  return true;
}

class AddAliasTagsPass : public fir::impl::AddAliasTagsBase<AddAliasTagsPass> {
public:
  void runOnOperation() override;

private:
  /// The real workhorse of the pass. This is a runOnOperation() which
  /// operates on fir::FirAliasTagOpInterface, using some extra state
  void runOnAliasInterface(fir::FirAliasTagOpInterface op, PassState &state);
};

} // namespace

static fir::DeclareOp getDeclareOp(mlir::Value arg) {
  if (auto declare =
          mlir::dyn_cast_or_null<fir::DeclareOp>(arg.getDefiningOp()))
    return declare;
  for (mlir::Operation *use : arg.getUsers())
    if (fir::DeclareOp declare = mlir::dyn_cast<fir::DeclareOp>(use))
      return declare;
  return nullptr;
}

/// Get the name of a function argument using the "fir.bindc_name" attribute,
/// or ""
static std::string getFuncArgName(mlir::Value arg) {
  // first try getting the name from the fir.declare
  if (fir::DeclareOp declare = getDeclareOp(arg))
    return declare.getUniqName().str();

  // get from attribute on function argument
  // always succeeds because arg is a function argument
  mlir::BlockArgument blockArg = mlir::cast<mlir::BlockArgument>(arg);
  assert(blockArg.getOwner() && blockArg.getOwner()->isEntryBlock() &&
         "arg is a function argument");
  mlir::FunctionOpInterface func = mlir::dyn_cast<mlir::FunctionOpInterface>(
      blockArg.getOwner()->getParentOp());
  assert(func && "This is not a function argument");
  mlir::StringAttr attr = func.getArgAttrOfType<mlir::StringAttr>(
      blockArg.getArgNumber(), "fir.bindc_name");
  if (!attr)
    return "";
  return attr.str();
}

void AddAliasTagsPass::runOnAliasInterface(fir::FirAliasTagOpInterface op,
                                           PassState &state) {
  mlir::func::FuncOp func = op->getParentOfType<mlir::func::FuncOp>();
  if (!func)
    return;

  toolchain::SmallVector<mlir::Value> accessedOperands = op.getAccessedOperands();
  assert(accessedOperands.size() == 1 &&
         "load and store only access one address");
  mlir::Value memref = accessedOperands.front();

  // Skip boxes and derived types that contain descriptors.
  // The box accesses get an "any descriptor access" tag in TBAABuilder
  // (CodeGen). The derived types accesses get "any access" tag
  // (because they access both the data and the descriptor(s)).
  // Note that it would be incorrect to attach any "data" access
  // tag to the derived type accesses here, because the tags
  // attached to the descriptor accesses in CodeGen will make
  // them non-conflicting with any descriptor accesses.
  if (state.typeReferencesDescriptor(memref.getType()))
    return;

  LLVM_DEBUG(toolchain::dbgs() << "Analysing " << op << "\n");

  const fir::AliasAnalysis::Source &source = state.getSource(memref);

  // Process the scopes, if not processed yet.
  state.processFunctionScopes(func);

  fir::DummyScopeOp scopeOp;
  if (auto declOp = source.origin.instantiationPoint) {
    // If the source is a dummy argument within some fir.dummy_scope,
    // then find the corresponding innermost scope to be used for finding
    // the right TBAA tree.
    auto declareOp = mlir::dyn_cast<fir::DeclareOp>(declOp);
    assert(declareOp && "Instantiation point must be fir.declare");
    if (auto dummyScope = declareOp.getDummyScope())
      scopeOp = mlir::cast<fir::DummyScopeOp>(dummyScope.getDefiningOp());
    if (!scopeOp)
      scopeOp = state.getDeclarationScope(declareOp);
  }

  mlir::LLVM::TBAATagAttr tag;
  // TBAA for dummy arguments
  if (enableDummyArgs &&
      source.kind == fir::AliasAnalysis::SourceKind::Argument) {
    LLVM_DEBUG(toolchain::dbgs().indent(2)
               << "Found reference to dummy argument at " << *op << "\n");
    std::string name = getFuncArgName(toolchain::cast<mlir::Value>(source.origin.u));
    // If it is a TARGET or POINTER, then we do not care about the name,
    // because the tag points to the root of the subtree currently.
    if (source.isTargetOrPointer()) {
      tag = state.getFuncTreeWithScope(func, scopeOp).targetDataTree.getTag();
    } else if (!name.empty()) {
      tag = state.getFuncTreeWithScope(func, scopeOp)
                .dummyArgDataTree.getTag(name);
    } else {
      LLVM_DEBUG(toolchain::dbgs().indent(2)
                 << "WARN: couldn't find a name for dummy argument " << *op
                 << "\n");
      tag = state.getFuncTreeWithScope(func, scopeOp).dummyArgDataTree.getTag();
    }

    // TBAA for global variables without descriptors
  } else if (enableGlobals &&
             source.kind == fir::AliasAnalysis::SourceKind::Global &&
             !source.isBoxData()) {
    mlir::SymbolRefAttr glbl = toolchain::cast<mlir::SymbolRefAttr>(source.origin.u);
    const char *name = glbl.getRootReference().data();
    LLVM_DEBUG(toolchain::dbgs().indent(2) << "Found reference to global " << name
                                      << " at " << *op << "\n");
    if (source.isPointer())
      tag = state.getFuncTreeWithScope(func, scopeOp).targetDataTree.getTag();
    else
      tag =
          state.getFuncTreeWithScope(func, scopeOp).globalDataTree.getTag(name);

    // TBAA for global variables with descriptors
  } else if (enableDirect &&
             source.kind == fir::AliasAnalysis::SourceKind::Global &&
             source.isBoxData()) {
    if (auto glbl = toolchain::dyn_cast<mlir::SymbolRefAttr>(source.origin.u)) {
      const char *name = glbl.getRootReference().data();
      LLVM_DEBUG(toolchain::dbgs().indent(2) << "Found reference to direct " << name
                                        << " at " << *op << "\n");
      if (source.isPointer())
        tag = state.getFuncTreeWithScope(func, scopeOp).targetDataTree.getTag();
      else
        tag = state.getFuncTreeWithScope(func, scopeOp)
                  .directDataTree.getTag(name);
    } else {
      LLVM_DEBUG(toolchain::dbgs().indent(2) << "Can't get name for direct "
                                        << source << " at " << *op << "\n");
    }

    // TBAA for local allocations
  } else if (enableLocalAllocs &&
             source.kind == fir::AliasAnalysis::SourceKind::Allocate) {
    std::optional<toolchain::StringRef> name;
    mlir::Operation *sourceOp =
        toolchain::cast<mlir::Value>(source.origin.u).getDefiningOp();
    bool unknownAllocOp = false;
    if (auto alloc = mlir::dyn_cast_or_null<fir::AllocaOp>(sourceOp))
      name = alloc.getUniqName();
    else if (auto alloc = mlir::dyn_cast_or_null<fir::AllocMemOp>(sourceOp))
      name = alloc.getUniqName();
    else
      unknownAllocOp = true;

    if (auto declOp = source.origin.instantiationPoint) {
      // Use the outermost scope for local allocations,
      // because using the innermost scope may result
      // in incorrect TBAA, when calls are inlined in MLIR.
      auto declareOp = mlir::dyn_cast<fir::DeclareOp>(declOp);
      assert(declareOp && "Instantiation point must be fir.declare");
      scopeOp = state.getOutermostScope(declareOp);
    }

    if (unknownAllocOp) {
      LLVM_DEBUG(toolchain::dbgs().indent(2)
                 << "WARN: unknown defining op for SourceKind::Allocate " << *op
                 << "\n");
    } else if (source.isPointer() && state.attachLocalAllocTag()) {
      LLVM_DEBUG(toolchain::dbgs().indent(2)
                 << "Found reference to allocation at " << *op << "\n");
      tag = state.getFuncTreeWithScope(func, scopeOp).targetDataTree.getTag();
    } else if (name && state.attachLocalAllocTag()) {
      LLVM_DEBUG(toolchain::dbgs().indent(2) << "Found reference to allocation "
                                        << name << " at " << *op << "\n");
      tag = state.getFuncTreeWithScope(func, scopeOp)
                .allocatedDataTree.getTag(*name);
    } else if (state.attachLocalAllocTag()) {
      LLVM_DEBUG(toolchain::dbgs().indent(2)
                 << "WARN: couldn't find a name for allocation " << *op
                 << "\n");
      tag =
          state.getFuncTreeWithScope(func, scopeOp).allocatedDataTree.getTag();
    }
  } else {
    if (source.kind != fir::AliasAnalysis::SourceKind::Argument &&
        source.kind != fir::AliasAnalysis::SourceKind::Allocate &&
        source.kind != fir::AliasAnalysis::SourceKind::Global)
      LLVM_DEBUG(toolchain::dbgs().indent(2)
                 << "WARN: unsupported value: " << source << "\n");
  }

  if (tag)
    op.setTBAATags(mlir::ArrayAttr::get(&getContext(), tag));
}

void AddAliasTagsPass::runOnOperation() {
  LLVM_DEBUG(toolchain::dbgs() << "=== Begin " DEBUG_TYPE " ===\n");

  // MLIR forbids storing state in a pass because different instances might be
  // used in different threads.
  // Instead this pass stores state per mlir::ModuleOp (which is what MLIR
  // thinks the pass operates on), then the real work of the pass is done in
  // runOnAliasInterface
  auto &domInfo = getAnalysis<mlir::DominanceInfo>();
  PassState state(domInfo, localAllocsThreshold.getPosition()
                               ? std::optional<unsigned>(localAllocsThreshold)
                               : std::nullopt);

  mlir::ModuleOp mod = getOperation();
  mod.walk(
      [&](fir::FirAliasTagOpInterface op) { runOnAliasInterface(op, state); });

  LLVM_DEBUG(toolchain::dbgs() << "=== End " DEBUG_TYPE " ===\n");
}
