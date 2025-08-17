//===- SetRuntimeCallAttributes.cpp ---------------------------------------===//
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
/// SetRuntimeCallAttributesPass looks for fir.call operations
/// that are calling into Fortran runtime, and tries to set different
/// attributes on them to enable more optimizations in LLVM backend
/// (granted that they are preserved all the way to LLVM IR).
/// This pass is currently only attaching fir.call wide atttributes,
/// such as ones corresponding to toolchain.memory, nosync, nocallbac, etc.
/// It is not designed to attach attributes to the arguments and the results
/// of a call.
//===----------------------------------------------------------------------===//
#include "language/Compability/Common/static-multimap-view.h"
#include "language/Compability/Optimizer/Builder/Runtime/RTBuilder.h"
#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "language/Compability/Optimizer/Dialect/FIROpsSupport.h"
#include "language/Compability/Optimizer/Support/InternalNames.h"
#include "language/Compability/Optimizer/Transforms/Passes.h"
#include "language/Compability/Runtime/io-api.h"
#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"

namespace fir {
#define GEN_PASS_DEF_SETRUNTIMECALLATTRIBUTES
#include "language/Compability/Optimizer/Transforms/Passes.h.inc"
} // namespace fir

#define DEBUG_TYPE "set-runtime-call-attrs"

using namespace language::Compability::runtime;
using namespace language::Compability::runtime::io;

#define mkIOKey(X) FirmkKey(IONAME(X))
#define mkRTKey(X) FirmkKey(RTNAME(X))

// Return LLVM dialect MemoryEffectsAttr for the given Fortran runtime call.
// This function is computing a generic value of this attribute
// by analyzing the arguments and their types.
// It tries to figure out if an "indirect" memory access is possible
// during this call. If it is not possible, then the memory effects
// are:
//   * other = NoModRef
//   * argMem = ModRef
//   * inaccessibleMem = ModRef
//
// Otherwise, it returns an empty attribute meaning ModRef for all kinds
// of memory.
//
// The attribute deduction is conservative in a sense that it applies
// to most of the runtime calls, but it may still be incorrect for some
// runtime calls.
static mlir::LLVM::MemoryEffectsAttr getGenericMemoryAttr(fir::CallOp callOp) {
  bool maybeIndirectAccess = false;
  for (auto arg : callOp.getArgOperands()) {
    mlir::Type argType = arg.getType();
    if (mlir::isa<fir::BaseBoxType>(argType)) {
      // If it is a null/absent box, then this particular call
      // cannot access memory indirectly through the box's
      // base_addr.
      auto def = arg.getDefiningOp();
      if (!mlir::isa_and_nonnull<fir::ZeroOp, fir::AbsentOp>(def)) {
        maybeIndirectAccess = true;
        break;
      }
    }
    if (auto refType = mlir::dyn_cast<fir::ReferenceType>(argType)) {
      if (!fir::isa_trivial(refType.getElementType())) {
        maybeIndirectAccess = true;
        break;
      }
    }
    if (auto ptrType = mlir::dyn_cast<mlir::LLVM::LLVMPointerType>(argType)) {
      maybeIndirectAccess = true;
      break;
    }
  }
  if (!maybeIndirectAccess) {
    return mlir::LLVM::MemoryEffectsAttr::get(
        callOp->getContext(),
        {/*other=*/mlir::LLVM::ModRefInfo::NoModRef,
         /*argMem=*/mlir::LLVM::ModRefInfo::ModRef,
         /*inaccessibleMem=*/mlir::LLVM::ModRefInfo::ModRef});
  }

  return {};
}

namespace {
class SetRuntimeCallAttributesPass
    : public fir::impl::SetRuntimeCallAttributesBase<
          SetRuntimeCallAttributesPass> {
public:
  void runOnOperation() override;
};

// A helper to match a type against a list of types.
template <typename T, typename... Ts>
constexpr bool IsAny = std::disjunction_v<std::is_same<T, Ts>...>;
} // end anonymous namespace

// MemoryAttrDesc type provides get() method for computing
// mlir::LLVM::MemoryEffectsAttr for the given Fortran runtime call.
// If needed, add specializations for particular runtime calls.
namespace {
// Default implementation just uses getGenericMemoryAttr().
// Note that it may be incorrect for some runtime calls.
template <typename KEY, typename Enable = void>
struct MemoryAttrDesc {
  static mlir::LLVM::MemoryEffectsAttr get(fir::CallOp callOp) {
    return getGenericMemoryAttr(callOp);
  }
};
} // end anonymous namespace

// NosyncAttrDesc type provides get() method for computing
// LLVM nosync attribute for the given call.
namespace {
// Default implementation always returns LLVM nosync.
// This should be true for the majority of the Fortran runtime calls.
template <typename KEY, typename Enable = void>
struct NosyncAttrDesc {
  static std::optional<mlir::NamedAttribute> get(fir::CallOp callOp) {
    // TODO: replace toolchain.nosync with an LLVM dialect callback.
    return mlir::NamedAttribute("toolchain.nosync",
                                mlir::UnitAttr::get(callOp->getContext()));
  }
};
} // end anonymous namespace

// NocallbackAttrDesc type provides get() method for computing
// LLVM nocallback attribute for the given call.
namespace {
// Default implementation always returns LLVM nocallback.
// It must be specialized for Fortran runtime functions that may call
// user functions during their execution (e.g. defined IO, assignment).
template <typename KEY, typename Enable = void>
struct NocallbackAttrDesc {
  static std::optional<mlir::NamedAttribute> get(fir::CallOp callOp) {
    // TODO: replace toolchain.nocallback with an LLVM dialect callback.
    return mlir::NamedAttribute("toolchain.nocallback",
                                mlir::UnitAttr::get(callOp->getContext()));
  }
};

// Derived types IO may call back into a Fortran module.
// This specialization is conservative for Input/OutputDerivedType,
// and it might be improved by checking if the NonTbpDefinedIoTable
// pointer argument is null.
template <typename KEY>
struct NocallbackAttrDesc<
    KEY, std::enable_if_t<
             IsAny<KEY, mkIOKey(OutputDerivedType), mkIOKey(InputDerivedType),
                   mkIOKey(OutputNamelist), mkIOKey(InputNamelist)>>> {
  static std::optional<mlir::NamedAttribute> get(fir::CallOp) {
    return std::nullopt;
  }
};
} // end anonymous namespace

namespace {
// RuntimeFunction provides different callbacks that compute values
// of fir.call attributes for a Fortran runtime function.
struct RuntimeFunction {
  using MemoryAttrGeneratorTy = mlir::LLVM::MemoryEffectsAttr (*)(fir::CallOp);
  using NamedAttrGeneratorTy =
      std::optional<mlir::NamedAttribute> (*)(fir::CallOp);
  using Key = std::string_view;
  constexpr operator Key() const { return key; }
  Key key;
  MemoryAttrGeneratorTy memoryAttrGenerator;
  NamedAttrGeneratorTy nosyncAttrGenerator;
  NamedAttrGeneratorTy nocallbackAttrGenerator;
};

// Helper type to create a RuntimeFunction descriptor given
// the KEY and a function name.
template <typename KEY>
struct RuntimeFactory {
  static constexpr RuntimeFunction create(const char name[]) {
    // GCC 7 does not recognize this as a constant expression:
    //   ((const char *)RuntimeFunction<>::name) == nullptr
    // This comparison comes from the basic_string_view(const char *)
    // constructor. We have to use the other constructor
    // that takes explicit length parameter.
    return RuntimeFunction{
        std::string_view{name, std::char_traits<char>::length(name)},
        MemoryAttrDesc<KEY>::get, NosyncAttrDesc<KEY>::get,
        NocallbackAttrDesc<KEY>::get};
  }
};
} // end anonymous namespace

#define KNOWN_IO_FUNC(X) RuntimeFactory<mkIOKey(X)>::create(mkIOKey(X)::name)
#define KNOWN_RUNTIME_FUNC(X)                                                  \
  RuntimeFactory<mkRTKey(X)>::create(mkRTKey(X)::name)

// A table of RuntimeFunction descriptors for all recognized
// Fortran runtime functions.
static constexpr RuntimeFunction runtimeFuncsTable[] = {
#include "language/Compability/Optimizer/Transforms/RuntimeFunctions.inc"
};

static constexpr language::Compability::common::StaticMultimapView<RuntimeFunction>
    runtimeFuncs(runtimeFuncsTable);
static_assert(runtimeFuncs.Verify() && "map must be sorted");

// Set attributes for the given Fortran runtime call.
// The symbolTable is used to cache the name lookups in the module.
static void setRuntimeCallAttributes(fir::CallOp callOp,
                                     mlir::SymbolTableCollection &symbolTable) {
  auto iface = mlir::cast<mlir::CallOpInterface>(callOp.getOperation());
  auto funcOp = mlir::dyn_cast_or_null<mlir::func::FuncOp>(
      iface.resolveCallableInTable(&symbolTable));

  if (!funcOp || !funcOp->hasAttrOfType<mlir::UnitAttr>(
                     fir::FIROpsDialect::getFirRuntimeAttrName()))
    return;

  toolchain::StringRef name = funcOp.getName();
  if (auto range = runtimeFuncs.equal_range(name);
      range.first != range.second) {
    // There should not be duplicate entries.
    assert(range.first + 1 == range.second);
    const RuntimeFunction &desc = *range.first;
    LLVM_DEBUG(toolchain::dbgs()
               << "Identified runtime function call: " << desc.key << '\n');
    if (mlir::LLVM::MemoryEffectsAttr memoryAttr =
            desc.memoryAttrGenerator(callOp))
      callOp->setAttr(fir::FIROpsDialect::getFirCallMemoryAttrName(),
                      memoryAttr);
    if (auto attr = desc.nosyncAttrGenerator(callOp))
      callOp->setAttr(attr->getName(), attr->getValue());
    if (auto attr = desc.nocallbackAttrGenerator(callOp))
      callOp->setAttr(attr->getName(), attr->getValue());
    LLVM_DEBUG(toolchain::dbgs() << "Operation with attrs: " << callOp << '\n');
  }
}

void SetRuntimeCallAttributesPass::runOnOperation() {
  mlir::func::FuncOp funcOp = getOperation();
  // Exit early for declarations to skip the debug output for them.
  if (funcOp.isDeclaration())
    return;
  LLVM_DEBUG(toolchain::dbgs() << "=== Begin " DEBUG_TYPE " ===\n");
  LLVM_DEBUG(toolchain::dbgs() << "Func-name:" << funcOp.getSymName() << "\n");

  mlir::SymbolTableCollection symbolTable;
  funcOp.walk([&](fir::CallOp callOp) {
    setRuntimeCallAttributes(callOp, symbolTable);
  });
  LLVM_DEBUG(toolchain::dbgs() << "=== End " DEBUG_TYPE " ===\n");
}
