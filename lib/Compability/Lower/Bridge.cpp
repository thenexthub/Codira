//===-- Bridge.cpp -- bridge to lower to MLIR -----------------------------===//
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

#include "language/Compability/Lower/Bridge.h"

#include "language/Compability/Lower/Allocatable.h"
#include "language/Compability/Lower/CUDA.h"
#include "language/Compability/Lower/CallInterface.h"
#include "language/Compability/Lower/Coarray.h"
#include "language/Compability/Lower/ConvertCall.h"
#include "language/Compability/Lower/ConvertExpr.h"
#include "language/Compability/Lower/ConvertExprToHLFIR.h"
#include "language/Compability/Lower/ConvertType.h"
#include "language/Compability/Lower/ConvertVariable.h"
#include "language/Compability/Lower/DirectivesCommon.h"
#include "language/Compability/Lower/HostAssociations.h"
#include "language/Compability/Lower/IO.h"
#include "language/Compability/Lower/IterationSpace.h"
#include "language/Compability/Lower/Mangler.h"
#include "language/Compability/Lower/OpenACC.h"
#include "language/Compability/Lower/OpenMP.h"
#include "language/Compability/Lower/PFTBuilder.h"
#include "language/Compability/Lower/Runtime.h"
#include "language/Compability/Lower/StatementContext.h"
#include "language/Compability/Lower/Support/ReductionProcessor.h"
#include "language/Compability/Lower/Support/Utils.h"
#include "language/Compability/Optimizer/Builder/BoxValue.h"
#include "language/Compability/Optimizer/Builder/CUFCommon.h"
#include "language/Compability/Optimizer/Builder/Character.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Runtime/Assign.h"
#include "language/Compability/Optimizer/Builder/Runtime/Character.h"
#include "language/Compability/Optimizer/Builder/Runtime/Derived.h"
#include "language/Compability/Optimizer/Builder/Runtime/EnvironmentDefaults.h"
#include "language/Compability/Optimizer/Builder/Runtime/Exceptions.h"
#include "language/Compability/Optimizer/Builder/Runtime/Main.h"
#include "language/Compability/Optimizer/Builder/Runtime/Ragged.h"
#include "language/Compability/Optimizer/Builder/Runtime/Stop.h"
#include "language/Compability/Optimizer/Builder/Todo.h"
#include "language/Compability/Optimizer/Dialect/CUF/Attributes/CUFAttr.h"
#include "language/Compability/Optimizer/Dialect/CUF/CUFOps.h"
#include "language/Compability/Optimizer/Dialect/FIRAttr.h"
#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "language/Compability/Optimizer/Dialect/Support/FIRContext.h"
#include "language/Compability/Optimizer/HLFIR/HLFIROps.h"
#include "language/Compability/Optimizer/Support/DataLayout.h"
#include "language/Compability/Optimizer/Support/FatalError.h"
#include "language/Compability/Optimizer/Support/InternalNames.h"
#include "language/Compability/Optimizer/Transforms/Passes.h"
#include "language/Compability/Parser/parse-tree.h"
#include "language/Compability/Runtime/iostat-consts.h"
#include "language/Compability/Semantics/openmp-dsa.h"
#include "language/Compability/Semantics/runtime-type-info.h"
#include "language/Compability/Semantics/symbol.h"
#include "language/Compability/Semantics/tools.h"
#include "language/Compability/Support/Flags.h"
#include "language/Compability/Support/Version.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/Matchers.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Parser/Parser.h"
#include "mlir/Support/StateStack.h"
#include "mlir/Transforms/RegionUtils.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringSet.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Target/TargetMachine.h"
#include <optional>

#define DEBUG_TYPE "flang-lower-bridge"

static toolchain::cl::opt<bool> dumpBeforeFir(
    "fdebug-dump-pre-fir", toolchain::cl::init(false),
    toolchain::cl::desc("dump the Pre-FIR tree prior to FIR generation"));

static toolchain::cl::opt<bool> forceLoopToExecuteOnce(
    "always-execute-loop-body", toolchain::cl::init(false),
    toolchain::cl::desc("force the body of a loop to execute at least once"));

namespace {
/// Information for generating a structured or unstructured increment loop.
struct IncrementLoopInfo {
  template <typename T>
  explicit IncrementLoopInfo(language::Compability::semantics::Symbol &sym, const T &lower,
                             const T &upper, const std::optional<T> &step,
                             bool isConcurrent = false)
      : loopVariableSym{&sym}, lowerExpr{language::Compability::semantics::GetExpr(lower)},
        upperExpr{language::Compability::semantics::GetExpr(upper)},
        stepExpr{language::Compability::semantics::GetExpr(step)},
        isConcurrent{isConcurrent} {}

  IncrementLoopInfo(IncrementLoopInfo &&) = default;
  IncrementLoopInfo &operator=(IncrementLoopInfo &&x) = default;

  bool isStructured() const { return !headerBlock; }

  mlir::Type getLoopVariableType() const {
    assert(loopVariable && "must be set");
    return fir::unwrapRefType(loopVariable.getType());
  }

  bool hasLocalitySpecs() const {
    return !localSymList.empty() || !localInitSymList.empty() ||
           !reduceSymList.empty() || !sharedSymList.empty();
  }

  // Data members common to both structured and unstructured loops.
  const language::Compability::semantics::Symbol *loopVariableSym;
  const language::Compability::lower::SomeExpr *lowerExpr;
  const language::Compability::lower::SomeExpr *upperExpr;
  const language::Compability::lower::SomeExpr *stepExpr;
  const language::Compability::lower::SomeExpr *maskExpr = nullptr;
  bool isConcurrent;
  toolchain::SmallVector<const language::Compability::semantics::Symbol *> localSymList;
  toolchain::SmallVector<const language::Compability::semantics::Symbol *> localInitSymList;
  toolchain::SmallVector<const language::Compability::semantics::Symbol *> reduceSymList;
  toolchain::SmallVector<fir::ReduceOperationEnum> reduceOperatorList;
  toolchain::SmallVector<const language::Compability::semantics::Symbol *> sharedSymList;
  mlir::Value loopVariable = nullptr;

  // Data members for structured loops.
  mlir::Operation *loopOp = nullptr;

  // Data members for unstructured loops.
  bool hasRealControl = false;
  mlir::Value tripVariable = nullptr;
  mlir::Value stepVariable = nullptr;
  mlir::Block *headerBlock = nullptr; // loop entry and test block
  mlir::Block *maskBlock = nullptr;   // concurrent loop mask block
  mlir::Block *bodyBlock = nullptr;   // first loop body block
  mlir::Block *exitBlock = nullptr;   // loop exit target block
};

/// Information to support stack management, object deallocation, and
/// object finalization at early and normal construct exits.
struct ConstructContext {
  explicit ConstructContext(language::Compability::lower::pft::Evaluation &eval,
                            language::Compability::lower::StatementContext &stmtCtx)
      : eval{eval}, stmtCtx{stmtCtx} {}

  language::Compability::lower::pft::Evaluation &eval;     // construct eval
  language::Compability::lower::StatementContext &stmtCtx; // construct exit code
  std::optional<hlfir::Entity> selector;     // construct selector, if any.
  bool pushedScope = false; // was a scoped pushed for this construct?
};

/// Helper to gather the lower bounds of array components with non deferred
/// shape when they are not all ones. Return an empty array attribute otherwise.
static mlir::DenseI64ArrayAttr
gatherComponentNonDefaultLowerBounds(mlir::Location loc,
                                     mlir::MLIRContext *mlirContext,
                                     const language::Compability::semantics::Symbol &sym) {
  if (language::Compability::semantics::IsAllocatableOrObjectPointer(&sym))
    return {};
  mlir::DenseI64ArrayAttr lbs_attr;
  if (const auto *objDetails =
          sym.detailsIf<language::Compability::semantics::ObjectEntityDetails>()) {
    toolchain::SmallVector<std::int64_t> lbs;
    bool hasNonDefaultLbs = false;
    for (const language::Compability::semantics::ShapeSpec &bounds : objDetails->shape())
      if (auto lb = bounds.lbound().GetExplicit()) {
        if (auto constant = language::Compability::evaluate::ToInt64(*lb)) {
          hasNonDefaultLbs |= (*constant != 1);
          lbs.push_back(*constant);
        } else {
          TODO(loc, "generate fir.dt_component for length parametrized derived "
                    "types");
        }
      }
    if (hasNonDefaultLbs) {
      assert(static_cast<int>(lbs.size()) == sym.Rank() &&
             "expected component bounds to be constant or deferred");
      lbs_attr = mlir::DenseI64ArrayAttr::get(mlirContext, lbs);
    }
  }
  return lbs_attr;
}

// Helper class to generate name of fir.global containing component explicit
// default value for objects, and initial procedure target for procedure pointer
// components.
static mlir::FlatSymbolRefAttr gatherComponentInit(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const language::Compability::semantics::Symbol &sym, fir::RecordType derivedType) {
  mlir::MLIRContext *mlirContext = &converter.getMLIRContext();
  // Return procedure target mangled name for procedure pointer components.
  if (const auto *procPtr =
          sym.detailsIf<language::Compability::semantics::ProcEntityDetails>()) {
    if (std::optional<const language::Compability::semantics::Symbol *> maybeInitSym =
            procPtr->init()) {
      // So far, do not make distinction between p => NULL() and p without init,
      // f18 always initialize pointers to NULL anyway.
      if (!*maybeInitSym)
        return {};
      return mlir::FlatSymbolRefAttr::get(mlirContext,
                                          converter.mangleName(**maybeInitSym));
    }
  }

  const auto *objDetails =
      sym.detailsIf<language::Compability::semantics::ObjectEntityDetails>();
  if (!objDetails || !objDetails->init().has_value())
    return {};
  // Object component initial value. Semantic package component object default
  // value into compiler generated symbols that are lowered as read-only
  // fir.global. Get the name of this global.
  std::string name = fir::NameUniquer::getComponentInitName(
      derivedType.getName(), toStringRef(sym.name()));
  return mlir::FlatSymbolRefAttr::get(mlirContext, name);
}

/// Helper class to generate the runtime type info global data and the
/// fir.type_info operations that contain the dipatch tables (if any).
/// The type info global data is required to describe the derived type to the
/// runtime so that it can operate over it.
/// It must be ensured these operations will be generated for every derived type
/// lowered in the current translated unit. However, these operations
/// cannot be generated before FuncOp have been created for functions since the
/// initializers may take their address (e.g for type bound procedures). This
/// class allows registering all the required type info while it is not
/// possible to create GlobalOp/TypeInfoOp, and to generate this data afte
/// function lowering.
class TypeInfoConverter {
  /// Store the location and symbols of derived type info to be generated.
  /// The location of the derived type instantiation is also stored because
  /// runtime type descriptor symbols are compiler generated and cannot be
  /// mapped to user code on their own.
  struct TypeInfo {
    language::Compability::semantics::SymbolRef symbol;
    const language::Compability::semantics::DerivedTypeSpec &typeSpec;
    fir::RecordType type;
    mlir::Location loc;
  };

public:
  void registerTypeInfo(language::Compability::lower::AbstractConverter &converter,
                        mlir::Location loc,
                        language::Compability::semantics::SymbolRef typeInfoSym,
                        const language::Compability::semantics::DerivedTypeSpec &typeSpec,
                        fir::RecordType type) {
    if (seen.contains(typeInfoSym))
      return;
    seen.insert(typeInfoSym);
    currentTypeInfoStack->emplace_back(
        TypeInfo{typeInfoSym, typeSpec, type, loc});
    return;
  }

  void createTypeInfo(language::Compability::lower::AbstractConverter &converter) {
    createTypeInfoForTypeDescriptorBuiltinType(converter);
    while (!registeredTypeInfoA.empty()) {
      currentTypeInfoStack = &registeredTypeInfoB;
      for (const TypeInfo &info : registeredTypeInfoA)
        createTypeInfoOpAndGlobal(converter, info);
      registeredTypeInfoA.clear();
      currentTypeInfoStack = &registeredTypeInfoA;
      for (const TypeInfo &info : registeredTypeInfoB)
        createTypeInfoOpAndGlobal(converter, info);
      registeredTypeInfoB.clear();
    }
  }

private:
  void createTypeInfoOpAndGlobal(language::Compability::lower::AbstractConverter &converter,
                                 const TypeInfo &info) {
    if (!converter.getLoweringOptions().getSkipExternalRttiDefinition())
      language::Compability::lower::createRuntimeTypeInfoGlobal(converter, info.symbol.get());
    createTypeInfoOp(converter, info);
  }

  void createTypeInfoForTypeDescriptorBuiltinType(
      language::Compability::lower::AbstractConverter &converter) {
    if (registeredTypeInfoA.empty())
      return;
    auto builtinTypeInfoType = toolchain::cast<fir::RecordType>(
        converter.genType(registeredTypeInfoA[0].symbol.get()));
    converter.getFirOpBuilder().createTypeInfoOp(
        registeredTypeInfoA[0].loc, builtinTypeInfoType,
        /*parentType=*/fir::RecordType{});
  }

  void createTypeInfoOp(language::Compability::lower::AbstractConverter &converter,
                        const TypeInfo &info) {
    fir::RecordType parentType{};
    if (const language::Compability::semantics::DerivedTypeSpec *parent =
            language::Compability::evaluate::GetParentTypeSpec(info.typeSpec))
      parentType = mlir::cast<fir::RecordType>(converter.genType(*parent));

    fir::FirOpBuilder &builder = converter.getFirOpBuilder();
    fir::TypeInfoOp dt;
    mlir::OpBuilder::InsertPoint insertPointIfCreated;
    std::tie(dt, insertPointIfCreated) =
        builder.createTypeInfoOp(info.loc, info.type, parentType);
    if (!insertPointIfCreated.isSet())
      return; // fir.type_info was already built in a previous call.

    // Set init, destroy, and nofinal attributes.
    if (!info.typeSpec.HasDefaultInitialization(/*ignoreAllocatable=*/false,
                                                /*ignorePointer=*/false))
      dt->setAttr(dt.getNoInitAttrName(), builder.getUnitAttr());
    if (!info.typeSpec.HasDestruction())
      dt->setAttr(dt.getNoDestroyAttrName(), builder.getUnitAttr());
    if (!language::Compability::semantics::MayRequireFinalization(info.typeSpec))
      dt->setAttr(dt.getNoFinalAttrName(), builder.getUnitAttr());

    const language::Compability::semantics::Scope &derivedScope =
        DEREF(info.typeSpec.GetScope());

    // Fill binding table region if the derived type has bindings.
    language::Compability::semantics::SymbolVector bindings =
        language::Compability::semantics::CollectBindings(derivedScope);
    if (!bindings.empty()) {
      builder.createBlock(&dt.getDispatchTable());
      for (const language::Compability::semantics::SymbolRef &binding : bindings) {
        const auto &details =
            binding.get().get<language::Compability::semantics::ProcBindingDetails>();
        std::string tbpName = binding.get().name().ToString();
        if (details.numPrivatesNotOverridden() > 0)
          tbpName += "."s + std::to_string(details.numPrivatesNotOverridden());
        std::string bindingName = converter.mangleName(details.symbol());
        fir::DTEntryOp::create(
            builder, info.loc,
            mlir::StringAttr::get(builder.getContext(), tbpName),
            mlir::SymbolRefAttr::get(builder.getContext(), bindingName));
      }
      fir::FirEndOp::create(builder, info.loc);
    }
    // Gather info about components that is not reflected in fir.type and may be
    // needed later: component initial values and array component non default
    // lower bounds.
    mlir::Block *componentInfo = nullptr;
    for (const auto &componentName :
         info.typeSpec.typeSymbol()
             .get<language::Compability::semantics::DerivedTypeDetails>()
             .componentNames()) {
      auto scopeIter = derivedScope.find(componentName);
      assert(scopeIter != derivedScope.cend() &&
             "failed to find derived type component symbol");
      const language::Compability::semantics::Symbol &component = scopeIter->second.get();
      mlir::FlatSymbolRefAttr init_val =
          gatherComponentInit(info.loc, converter, component, info.type);
      mlir::DenseI64ArrayAttr lbs = gatherComponentNonDefaultLowerBounds(
          info.loc, builder.getContext(), component);
      if (init_val || lbs) {
        if (!componentInfo)
          componentInfo = builder.createBlock(&dt.getComponentInfo());
        auto compName = mlir::StringAttr::get(builder.getContext(),
                                              toStringRef(component.name()));
        fir::DTComponentOp::create(builder, info.loc, compName, lbs, init_val);
      }
    }
    if (componentInfo)
      fir::FirEndOp::create(builder, info.loc);
    builder.restoreInsertionPoint(insertPointIfCreated);
  }

  /// Store the front-end data that will be required to generate the type info
  /// for the derived types that have been converted to fir.type<>. There are
  /// two stacks since the type info may visit new types, so the new types must
  /// be added to a new stack.
  toolchain::SmallVector<TypeInfo> registeredTypeInfoA;
  toolchain::SmallVector<TypeInfo> registeredTypeInfoB;
  toolchain::SmallVector<TypeInfo> *currentTypeInfoStack = &registeredTypeInfoA;
  /// Track symbols symbols processed during and after the registration
  /// to avoid infinite loops between type conversions and global variable
  /// creation.
  toolchain::SmallSetVector<language::Compability::semantics::SymbolRef, 32> seen;
};

using IncrementLoopNestInfo = toolchain::SmallVector<IncrementLoopInfo, 8>;
} // namespace

//===----------------------------------------------------------------------===//
// FirConverter
//===----------------------------------------------------------------------===//

namespace {

/// Traverse the pre-FIR tree (PFT) to generate the FIR dialect of MLIR.
class FirConverter : public language::Compability::lower::AbstractConverter {
public:
  explicit FirConverter(language::Compability::lower::LoweringBridge &bridge)
      : language::Compability::lower::AbstractConverter(bridge.getLoweringOptions()),
        bridge{bridge}, foldingContext{bridge.createFoldingContext()},
        mlirSymbolTable{bridge.getModule()} {}
  virtual ~FirConverter() = default;

  /// Convert the PFT to FIR.
  void run(language::Compability::lower::pft::Program &pft) {
    // Preliminary translation pass.

    // Lower common blocks, taking into account initialization and the largest
    // size of all instances of each common block. This is done before lowering
    // since the global definition may differ from any one local definition.
    lowerCommonBlocks(pft.getCommonBlocks());

    // - Declare all functions that have definitions so that definition
    //   signatures prevail over call site signatures.
    // - Define module variables and OpenMP/OpenACC declarative constructs so
    //   they are available before lowering any function that may use them.
    bool hasMainProgram = false;
    const language::Compability::semantics::Symbol *globalOmpRequiresSymbol = nullptr;
    createBuilderOutsideOfFuncOpAndDo([&]() {
      for (language::Compability::lower::pft::Program::Units &u : pft.getUnits()) {
        language::Compability::common::visit(
            language::Compability::common::visitors{
                [&](language::Compability::lower::pft::FunctionLikeUnit &f) {
                  if (f.isMainProgram())
                    hasMainProgram = true;
                  declareFunction(f);
                  if (!globalOmpRequiresSymbol)
                    globalOmpRequiresSymbol = f.getScope().symbol();
                },
                [&](language::Compability::lower::pft::ModuleLikeUnit &m) {
                  lowerModuleDeclScope(m);
                  for (language::Compability::lower::pft::ContainedUnit &unit :
                       m.containedUnitList)
                    if (auto *f =
                            std::get_if<language::Compability::lower::pft::FunctionLikeUnit>(
                                &unit))
                      declareFunction(*f);
                },
                [&](language::Compability::lower::pft::BlockDataUnit &b) {
                  if (!globalOmpRequiresSymbol)
                    globalOmpRequiresSymbol = b.symTab.symbol();
                },
                [&](language::Compability::lower::pft::CompilerDirectiveUnit &d) {},
                [&](language::Compability::lower::pft::OpenACCDirectiveUnit &d) {},
            },
            u);
      }
    });

    // Create definitions of intrinsic module constants.
    createBuilderOutsideOfFuncOpAndDo(
        [&]() { createIntrinsicModuleDefinitions(pft); });

    // Primary translation pass.
    for (language::Compability::lower::pft::Program::Units &u : pft.getUnits()) {
      language::Compability::common::visit(
          language::Compability::common::visitors{
              [&](language::Compability::lower::pft::FunctionLikeUnit &f) { lowerFunc(f); },
              [&](language::Compability::lower::pft::ModuleLikeUnit &m) { lowerMod(m); },
              [&](language::Compability::lower::pft::BlockDataUnit &b) {},
              [&](language::Compability::lower::pft::CompilerDirectiveUnit &d) {},
              [&](language::Compability::lower::pft::OpenACCDirectiveUnit &d) {},
          },
          u);
    }

    // Once all the code has been translated, create global runtime type info
    // data structures for the derived types that have been processed, as well
    // as fir.type_info operations for the dispatch tables.
    createBuilderOutsideOfFuncOpAndDo(
        [&]() { typeInfoConverter.createTypeInfo(*this); });

    // Generate the `main` entry point if necessary
    if (hasMainProgram)
      createBuilderOutsideOfFuncOpAndDo([&]() {
        fir::runtime::genMain(*builder, toLocation(),
                              bridge.getEnvironmentDefaults(),
                              getFoldingContext().languageFeatures().IsEnabled(
                                  language::Compability::common::LanguageFeature::CUDA));
      });

    finalizeOpenMPLowering(globalOmpRequiresSymbol);
  }

  /// Declare a function.
  void declareFunction(language::Compability::lower::pft::FunctionLikeUnit &funit) {
    CHECK(builder && "declareFunction called with uninitialized builder");
    setCurrentPosition(funit.getStartingSourceLoc());
    for (int entryIndex = 0, last = funit.entryPointList.size();
         entryIndex < last; ++entryIndex) {
      funit.setActiveEntry(entryIndex);
      // Calling CalleeInterface ctor will build a declaration
      // mlir::func::FuncOp with no other side effects.
      // TODO: when doing some compiler profiling on real apps, it may be worth
      // to check it's better to save the CalleeInterface instead of recomputing
      // it later when lowering the body. CalleeInterface ctor should be linear
      // with the number of arguments, so it is not awful to do it that way for
      // now, but the linear coefficient might be non negligible. Until
      // measured, stick to the solution that impacts the code less.
      language::Compability::lower::CalleeInterface{funit, *this};
    }
    funit.setActiveEntry(0);

    // Compute the set of host associated entities from the nested functions.
    toolchain::SetVector<const language::Compability::semantics::Symbol *> escapeHost;
    for (language::Compability::lower::pft::ContainedUnit &unit : funit.containedUnitList)
      if (auto *f = std::get_if<language::Compability::lower::pft::FunctionLikeUnit>(&unit))
        collectHostAssociatedVariables(*f, escapeHost);
    funit.setHostAssociatedSymbols(escapeHost);

    // Declare internal procedures
    for (language::Compability::lower::pft::ContainedUnit &unit : funit.containedUnitList)
      if (auto *f = std::get_if<language::Compability::lower::pft::FunctionLikeUnit>(&unit))
        declareFunction(*f);
  }

  /// Get the scope that is defining or using \p sym. The returned scope is not
  /// the ultimate scope, since this helper does not traverse use association.
  /// This allows capturing module variables that are referenced in an internal
  /// procedure but whose use statement is inside the host program.
  const language::Compability::semantics::Scope &
  getSymbolHostScope(const language::Compability::semantics::Symbol &sym) {
    const language::Compability::semantics::Symbol *hostSymbol = &sym;
    while (const auto *details =
               hostSymbol->detailsIf<language::Compability::semantics::HostAssocDetails>())
      hostSymbol = &details->symbol();
    return hostSymbol->owner();
  }

  /// Collects the canonical list of all host associated symbols. These bindings
  /// must be aggregated into a tuple which can then be added to each of the
  /// internal procedure declarations and passed at each call site.
  void collectHostAssociatedVariables(
      language::Compability::lower::pft::FunctionLikeUnit &funit,
      toolchain::SetVector<const language::Compability::semantics::Symbol *> &escapees) {
    const language::Compability::semantics::Scope *internalScope =
        funit.getSubprogramSymbol().scope();
    assert(internalScope && "internal procedures symbol must create a scope");
    auto addToListIfEscapee = [&](const language::Compability::semantics::Symbol &sym) {
      const language::Compability::semantics::Symbol &ultimate = sym.GetUltimate();
      const auto *namelistDetails =
          ultimate.detailsIf<language::Compability::semantics::NamelistDetails>();
      if (ultimate.has<language::Compability::semantics::ObjectEntityDetails>() ||
          language::Compability::semantics::IsProcedurePointer(ultimate) ||
          language::Compability::semantics::IsDummy(sym) || namelistDetails) {
        const language::Compability::semantics::Scope &symbolScope = getSymbolHostScope(sym);
        if (symbolScope.kind() ==
                language::Compability::semantics::Scope::Kind::MainProgram ||
            symbolScope.kind() == language::Compability::semantics::Scope::Kind::Subprogram)
          if (symbolScope != *internalScope &&
              symbolScope.Contains(*internalScope)) {
            if (namelistDetails) {
              // So far, namelist symbols are processed on the fly in IO and
              // the related namelist data structure is not added to the symbol
              // map, so it cannot be passed to the internal procedures.
              // Instead, all the symbols of the host namelist used in the
              // internal procedure must be considered as host associated so
              // that IO lowering can find them when needed.
              for (const auto &namelistObject : namelistDetails->objects())
                escapees.insert(&*namelistObject);
            } else {
              escapees.insert(&ultimate);
            }
          }
      }
    };
    language::Compability::lower::pft::visitAllSymbols(funit, addToListIfEscapee);
  }

  //===--------------------------------------------------------------------===//
  // AbstractConverter overrides
  //===--------------------------------------------------------------------===//

  mlir::Value getSymbolAddress(language::Compability::lower::SymbolRef sym) override final {
    return lookupSymbol(sym).getAddr();
  }

  fir::ExtendedValue symBoxToExtendedValue(
      const language::Compability::lower::SymbolBox &symBox) override final {
    return symBox.match(
        [](const language::Compability::lower::SymbolBox::Intrinsic &box)
            -> fir::ExtendedValue { return box.getAddr(); },
        [](const language::Compability::lower::SymbolBox::None &) -> fir::ExtendedValue {
          toolchain::report_fatal_error("symbol not mapped");
        },
        [&](const fir::FortranVariableOpInterface &x) -> fir::ExtendedValue {
          return hlfir::translateToExtendedValue(getCurrentLocation(),
                                                 getFirOpBuilder(), x);
        },
        [](const auto &box) -> fir::ExtendedValue { return box; });
  }

  fir::ExtendedValue
  getSymbolExtendedValue(const language::Compability::semantics::Symbol &sym,
                         language::Compability::lower::SymMap *symMap) override final {
    language::Compability::lower::SymbolBox sb = lookupSymbol(sym, symMap);
    if (!sb) {
      LLVM_DEBUG(toolchain::dbgs() << "unknown symbol: " << sym << "\nmap: "
                              << (symMap ? *symMap : localSymbols) << '\n');
      fir::emitFatalError(getCurrentLocation(),
                          "symbol is not mapped to any IR value");
    }
    return symBoxToExtendedValue(sb);
  }

  mlir::Value impliedDoBinding(toolchain::StringRef name) override final {
    mlir::Value val = localSymbols.lookupImpliedDo(name);
    if (!val)
      fir::emitFatalError(toLocation(), "ac-do-variable has no binding");
    return val;
  }

  void copySymbolBinding(language::Compability::lower::SymbolRef src,
                         language::Compability::lower::SymbolRef target) override final {
    localSymbols.copySymbolBinding(src, target);
  }

  /// Add the symbol binding to the inner-most level of the symbol map and
  /// return true if it is not already present. Otherwise, return false.
  bool bindIfNewSymbol(language::Compability::lower::SymbolRef sym,
                       const fir::ExtendedValue &exval) {
    if (shallowLookupSymbol(sym))
      return false;
    bindSymbol(sym, exval);
    return true;
  }

  void bindSymbol(language::Compability::lower::SymbolRef sym,
                  const fir::ExtendedValue &exval) override final {
    addSymbol(sym, exval, /*forced=*/true);
  }

  void
  overrideExprValues(const language::Compability::lower::ExprToValueMap *map) override final {
    exprValueOverrides = map;
  }

  const language::Compability::lower::ExprToValueMap *getExprOverrides() override final {
    return exprValueOverrides;
  }

  bool lookupLabelSet(language::Compability::lower::SymbolRef sym,
                      language::Compability::lower::pft::LabelSet &labelSet) override final {
    language::Compability::lower::pft::FunctionLikeUnit &owningProc =
        *getEval().getOwningProcedure();
    auto iter = owningProc.assignSymbolLabelMap.find(sym);
    if (iter == owningProc.assignSymbolLabelMap.end())
      return false;
    labelSet = iter->second;
    return true;
  }

  language::Compability::lower::pft::Evaluation *
  lookupLabel(language::Compability::lower::pft::Label label) override final {
    language::Compability::lower::pft::FunctionLikeUnit &owningProc =
        *getEval().getOwningProcedure();
    return owningProc.labelEvaluationMap.lookup(label);
  }

  fir::ExtendedValue
  genExprAddr(const language::Compability::lower::SomeExpr &expr,
              language::Compability::lower::StatementContext &context,
              mlir::Location *locPtr = nullptr) override final {
    mlir::Location loc = locPtr ? *locPtr : toLocation();
    if (lowerToHighLevelFIR())
      return language::Compability::lower::convertExprToAddress(loc, *this, expr,
                                                  localSymbols, context);
    return language::Compability::lower::createSomeExtendedAddress(loc, *this, expr,
                                                     localSymbols, context);
  }

  fir::ExtendedValue
  genExprValue(const language::Compability::lower::SomeExpr &expr,
               language::Compability::lower::StatementContext &context,
               mlir::Location *locPtr = nullptr) override final {
    mlir::Location loc = locPtr ? *locPtr : toLocation();
    if (lowerToHighLevelFIR())
      return language::Compability::lower::convertExprToValue(loc, *this, expr, localSymbols,
                                                context);
    return language::Compability::lower::createSomeExtendedExpression(loc, *this, expr,
                                                        localSymbols, context);
  }

  fir::ExtendedValue
  genExprBox(mlir::Location loc, const language::Compability::lower::SomeExpr &expr,
             language::Compability::lower::StatementContext &stmtCtx) override final {
    if (lowerToHighLevelFIR())
      return language::Compability::lower::convertExprToBox(loc, *this, expr, localSymbols,
                                              stmtCtx);
    return language::Compability::lower::createBoxValue(loc, *this, expr, localSymbols,
                                          stmtCtx);
  }

  language::Compability::evaluate::FoldingContext &getFoldingContext() override final {
    return foldingContext;
  }

  mlir::Type genType(const language::Compability::lower::SomeExpr &expr) override final {
    return language::Compability::lower::translateSomeExprToFIRType(*this, expr);
  }
  mlir::Type genType(const language::Compability::lower::pft::Variable &var) override final {
    return language::Compability::lower::translateVariableToFIRType(*this, var);
  }
  mlir::Type genType(language::Compability::lower::SymbolRef sym) override final {
    return language::Compability::lower::translateSymbolToFIRType(*this, sym);
  }
  mlir::Type
  genType(language::Compability::common::TypeCategory tc, int kind,
          toolchain::ArrayRef<std::int64_t> lenParameters) override final {
    return language::Compability::lower::getFIRType(&getMLIRContext(), tc, kind,
                                      lenParameters);
  }
  mlir::Type
  genType(const language::Compability::semantics::DerivedTypeSpec &tySpec) override final {
    return language::Compability::lower::translateDerivedTypeToFIRType(*this, tySpec);
  }
  mlir::Type genType(language::Compability::common::TypeCategory tc) override final {
    return language::Compability::lower::getFIRType(
        &getMLIRContext(), tc, bridge.getDefaultKinds().GetDefaultKind(tc), {});
  }

  language::Compability::lower::TypeConstructionStack &
  getTypeConstructionStack() override final {
    return typeConstructionStack;
  }

  bool
  isPresentShallowLookup(const language::Compability::semantics::Symbol &sym) override final {
    return bool(shallowLookupSymbol(sym));
  }

  bool createHostAssociateVarClone(const language::Compability::semantics::Symbol &sym,
                                   bool skipDefaultInit) override final {
    mlir::Location loc = genLocation(sym.name());
    mlir::Type symType = genType(sym);
    const auto *details = sym.detailsIf<language::Compability::semantics::HostAssocDetails>();
    assert(details && "No host-association found");
    const language::Compability::semantics::Symbol &hsym = details->symbol();
    mlir::Type hSymType = genType(hsym.GetUltimate());
    language::Compability::lower::SymbolBox hsb =
        lookupSymbol(hsym, /*symMap=*/nullptr, /*forceHlfirBase=*/true);

    auto allocate = [&](toolchain::ArrayRef<mlir::Value> shape,
                        toolchain::ArrayRef<mlir::Value> typeParams) -> mlir::Value {
      mlir::Value allocVal = builder->allocateLocal(
          loc,
          language::Compability::semantics::IsAllocatableOrObjectPointer(&hsym.GetUltimate())
              ? hSymType
              : symType,
          mangleName(sym), toStringRef(sym.GetUltimate().name()),
          /*pinned=*/true, shape, typeParams,
          sym.GetUltimate().attrs().test(language::Compability::semantics::Attr::TARGET));
      return allocVal;
    };

    fir::ExtendedValue hexv = symBoxToExtendedValue(hsb);
    fir::ExtendedValue exv = hexv.match(
        [&](const fir::BoxValue &box) -> fir::ExtendedValue {
          const language::Compability::semantics::DeclTypeSpec *type = sym.GetType();
          if (type && type->IsPolymorphic())
            TODO(loc, "create polymorphic host associated copy");
          // Create a contiguous temp with the same shape and length as
          // the original variable described by a fir.box.
          toolchain::SmallVector<mlir::Value> extents =
              fir::factory::getExtents(loc, *builder, hexv);
          if (box.isDerivedWithLenParameters())
            TODO(loc, "get length parameters from derived type BoxValue");
          if (box.isCharacter()) {
            mlir::Value len = fir::factory::readCharLen(*builder, loc, box);
            mlir::Value temp = allocate(extents, {len});
            return fir::CharArrayBoxValue{temp, len, extents};
          }
          return fir::ArrayBoxValue{allocate(extents, {}), extents};
        },
        [&](const fir::MutableBoxValue &box) -> fir::ExtendedValue {
          // Allocate storage for a pointer/allocatble descriptor.
          // No shape/lengths to be passed to the alloca.
          return fir::MutableBoxValue(allocate({}, {}), {}, {});
        },
        [&](const auto &) -> fir::ExtendedValue {
          mlir::Value temp =
              allocate(fir::factory::getExtents(loc, *builder, hexv),
                       fir::factory::getTypeParams(loc, *builder, hexv));
          return fir::substBase(hexv, temp);
        });

    // Initialise cloned allocatable
    hexv.match(
        [&](const fir::MutableBoxValue &box) -> void {
          const auto new_box = exv.getBoxOf<fir::MutableBoxValue>();
          if (language::Compability::semantics::IsPointer(sym.GetUltimate())) {
            // Establish the pointer descriptors. The rank and type code/size
            // at least must be set properly for later inquiry of the pointer
            // to work, and new pointers are always given disassociated status
            // by flang for safety, even if this is not required by the
            // language.
            auto empty = fir::factory::createUnallocatedBox(
                *builder, loc, new_box->getBoxTy(), box.nonDeferredLenParams(),
                {});
            fir::StoreOp::create(*builder, loc, empty, new_box->getAddr());
            return;
          }
          // Copy allocation status of Allocatables, creating new storage if
          // needed.

          // allocate if allocated
          mlir::Value isAllocated =
              fir::factory::genIsAllocatedOrAssociatedTest(*builder, loc, box);
          auto if_builder = builder->genIfThenElse(loc, isAllocated);
          if_builder.genThen([&]() {
            std::string name = mangleName(sym) + ".alloc";
            fir::ExtendedValue read = fir::factory::genMutableBoxRead(
                *builder, loc, box, /*mayBePolymorphic=*/false);
            if (auto read_arr_box = read.getBoxOf<fir::ArrayBoxValue>()) {
              fir::factory::genInlinedAllocation(*builder, loc, *new_box,
                                                 read_arr_box->getLBounds(),
                                                 read_arr_box->getExtents(),
                                                 /*lenParams=*/{}, name,
                                                 /*mustBeHeap=*/true);
            } else if (auto read_char_arr_box =
                           read.getBoxOf<fir::CharArrayBoxValue>()) {
              fir::factory::genInlinedAllocation(
                  *builder, loc, *new_box, read_char_arr_box->getLBounds(),
                  read_char_arr_box->getExtents(), read_char_arr_box->getLen(),
                  name,
                  /*mustBeHeap=*/true);
            } else if (auto read_char_box =
                           read.getBoxOf<fir::CharBoxValue>()) {
              fir::factory::genInlinedAllocation(*builder, loc, *new_box,
                                                 /*lbounds=*/{},
                                                 /*extents=*/{},
                                                 read_char_box->getLen(), name,
                                                 /*mustBeHeap=*/true);
            } else {
              fir::factory::genInlinedAllocation(
                  *builder, loc, *new_box, box.getMutableProperties().lbounds,
                  box.getMutableProperties().extents,
                  box.nonDeferredLenParams(), name,
                  /*mustBeHeap=*/true);
            }
          });
          if_builder.genElse([&]() {
            // nullify box
            auto empty = fir::factory::createUnallocatedBox(
                *builder, loc, new_box->getBoxTy(),
                new_box->nonDeferredLenParams(), {});
            fir::StoreOp::create(*builder, loc, empty, new_box->getAddr());
          });
          if_builder.end();
        },
        [&](const auto &) -> void {
          // Always initialize allocatable component descriptor, even when the
          // value is later copied from the host (e.g. firstprivate) because the
          // assignment from the host to the copy will fail if the component
          // descriptors are not initialized.
          if (skipDefaultInit && !hlfir::mayHaveAllocatableComponent(hSymType))
            return;
          // Initialize local/private derived types with default
          // initialization (Fortran 2023 section 11.1.7.5 and OpenMP 5.2
          // section 5.3). Pointer and allocatable components, when allowed,
          // also need to be established so that flang runtime can later work
          // with them.
          if (const language::Compability::semantics::DeclTypeSpec *declTypeSpec =
                  sym.GetType())
            if (const language::Compability::semantics::DerivedTypeSpec *derivedTypeSpec =
                    declTypeSpec->AsDerived())
              if (derivedTypeSpec->HasDefaultInitialization(
                      /*ignoreAllocatable=*/false, /*ignorePointer=*/false)) {
                mlir::Value box = builder->createBox(loc, exv);
                fir::runtime::genDerivedTypeInitialize(*builder, loc, box);
              }
        });

    return bindIfNewSymbol(sym, exv);
  }

  void createHostAssociateVarCloneDealloc(
      const language::Compability::semantics::Symbol &sym) override final {
    mlir::Location loc = genLocation(sym.name());
    language::Compability::lower::SymbolBox hsb =
        lookupSymbol(sym, /*symMap=*/nullptr, /*forceHlfirBase=*/true);

    fir::ExtendedValue hexv = symBoxToExtendedValue(hsb);
    hexv.match(
        [&](const fir::MutableBoxValue &new_box) -> void {
          // Do not process pointers
          if (language::Compability::semantics::IsPointer(sym.GetUltimate())) {
            return;
          }
          // deallocate allocated in createHostAssociateVarClone value
          language::Compability::lower::genDeallocateIfAllocated(*this, new_box, loc);
        },
        [&](const auto &) -> void {
          // Do nothing
        });
  }

  void copyVar(mlir::Location loc, mlir::Value dst, mlir::Value src,
               fir::FortranVariableFlagsEnum attrs) override final {
    bool isAllocatable =
        bitEnumContainsAny(attrs, fir::FortranVariableFlagsEnum::allocatable);
    bool isPointer =
        bitEnumContainsAny(attrs, fir::FortranVariableFlagsEnum::pointer);

    copyVarHLFIR(loc, language::Compability::lower::SymbolBox::Intrinsic{dst},
                 language::Compability::lower::SymbolBox::Intrinsic{src}, isAllocatable,
                 isPointer, language::Compability::semantics::Symbol::Flags());
  }

  void
  copyHostAssociateVar(const language::Compability::semantics::Symbol &sym,
                       mlir::OpBuilder::InsertPoint *copyAssignIP = nullptr,
                       bool hostIsSource = true) override final {
    // 1) Fetch the original copy of the variable.
    assert(sym.has<language::Compability::semantics::HostAssocDetails>() &&
           "No host-association found");
    const language::Compability::semantics::Symbol &hsym = sym.GetUltimate();
    language::Compability::lower::SymbolBox hsb = lookupOneLevelUpSymbol(hsym);
    assert(hsb && "Host symbol box not found");

    // 2) Fetch the copied one that will mask the original.
    language::Compability::lower::SymbolBox sb = shallowLookupSymbol(sym);
    assert(sb && "Host-associated symbol box not found");
    assert(hsb.getAddr() != sb.getAddr() &&
           "Host and associated symbol boxes are the same");

    // 3) Perform the assignment.
    mlir::OpBuilder::InsertionGuard guard(*builder);
    if (copyAssignIP && copyAssignIP->isSet())
      builder->restoreInsertionPoint(*copyAssignIP);
    else
      builder->setInsertionPointAfter(sb.getAddr().getDefiningOp());

    language::Compability::lower::SymbolBox *lhs_sb, *rhs_sb;
    if (!hostIsSource) {
      lhs_sb = &hsb;
      rhs_sb = &sb;
    } else {
      lhs_sb = &sb;
      rhs_sb = &hsb;
    }

    copyVar(sym, *lhs_sb, *rhs_sb, sym.flags());
  }

  void genEval(language::Compability::lower::pft::Evaluation &eval,
               bool unstructuredContext) override final {
    genFIR(eval, unstructuredContext);
  }

  //===--------------------------------------------------------------------===//
  // Utility methods
  //===--------------------------------------------------------------------===//

  void collectSymbolSet(
      language::Compability::lower::pft::Evaluation &eval,
      toolchain::SetVector<const language::Compability::semantics::Symbol *> &symbolSet,
      language::Compability::semantics::Symbol::Flag flag, bool collectSymbols,
      bool checkHostAssociatedSymbols) override final {
    auto addToList = [&](const language::Compability::semantics::Symbol &sym) {
      std::function<void(const language::Compability::semantics::Symbol &, bool)>
          insertSymbols = [&](const language::Compability::semantics::Symbol &oriSymbol,
                              bool collectSymbol) {
            if (collectSymbol && oriSymbol.test(flag)) {
              symbolSet.insert(&oriSymbol);
            } else if (const auto *commonDetails =
                           oriSymbol.detailsIf<
                               language::Compability::semantics::CommonBlockDetails>()) {
              for (const auto &mem : commonDetails->objects())
                if (collectSymbol && mem->test(flag))
                  symbolSet.insert(&(*mem).GetUltimate());
            } else if (checkHostAssociatedSymbols) {
              if (const auto *details{
                      oriSymbol
                          .detailsIf<language::Compability::semantics::HostAssocDetails>()})
                insertSymbols(details->symbol(), true);
            }
          };
      insertSymbols(sym, collectSymbols);
    };
    language::Compability::lower::pft::visitAllSymbols(eval, addToList);
  }

  mlir::Location getCurrentLocation() override final { return toLocation(); }

  /// Generate a dummy location.
  mlir::Location genUnknownLocation() override final {
    // Note: builder may not be instantiated yet
    return mlir::UnknownLoc::get(&getMLIRContext());
  }

  static mlir::Location genLocation(language::Compability::parser::SourcePosition pos,
                                    mlir::MLIRContext &ctx) {
    toolchain::SmallString<256> path(*pos.path);
    toolchain::sys::fs::make_absolute(path);
    toolchain::sys::path::remove_dots(path);
    return mlir::FileLineColLoc::get(&ctx, path.str(), pos.line, pos.column);
  }

  /// Generate a `Location` from the `CharBlock`.
  mlir::Location
  genLocation(const language::Compability::parser::CharBlock &block) override final {
    mlir::Location mainLocation = genUnknownLocation();
    if (const language::Compability::parser::AllCookedSources *cooked =
            bridge.getCookedSource()) {
      if (std::optional<language::Compability::parser::ProvenanceRange> provenance =
              cooked->GetProvenanceRange(block)) {
        if (std::optional<language::Compability::parser::SourcePosition> filePos =
                cooked->allSources().GetSourcePosition(provenance->start()))
          mainLocation = genLocation(*filePos, getMLIRContext());

        toolchain::SmallVector<mlir::Location> locs;
        locs.push_back(mainLocation);

        toolchain::SmallVector<fir::LocationKindAttr> locAttrs;
        locAttrs.push_back(fir::LocationKindAttr::get(&getMLIRContext(),
                                                      fir::LocationKind::Base));

        // Gather include location information if any.
        language::Compability::parser::ProvenanceRange *prov = &*provenance;
        while (prov) {
          if (std::optional<language::Compability::parser::ProvenanceRange> include =
                  cooked->allSources().GetInclusionInfo(*prov)) {
            if (std::optional<language::Compability::parser::SourcePosition> incPos =
                    cooked->allSources().GetSourcePosition(include->start())) {
              locs.push_back(genLocation(*incPos, getMLIRContext()));
              locAttrs.push_back(fir::LocationKindAttr::get(
                  &getMLIRContext(), fir::LocationKind::Inclusion));
            }
            prov = &*include;
          } else {
            prov = nullptr;
          }
        }
        if (locs.size() > 1) {
          assert(locs.size() == locAttrs.size() &&
                 "expect as many attributes as locations");
          return mlir::FusedLocWith<fir::LocationKindArrayAttr>::get(
              &getMLIRContext(), locs,
              fir::LocationKindArrayAttr::get(&getMLIRContext(), locAttrs));
        }
      }
    }
    return mainLocation;
  }

  const language::Compability::semantics::Scope &getCurrentScope() override final {
    return bridge.getSemanticsContext().FindScope(currentPosition);
  }

  fir::FirOpBuilder &getFirOpBuilder() override final {
    CHECK(builder && "builder is not set before calling getFirOpBuilder");
    return *builder;
  }

  mlir::ModuleOp getModuleOp() override final { return bridge.getModule(); }

  mlir::MLIRContext &getMLIRContext() override final {
    return bridge.getMLIRContext();
  }
  std::string
  mangleName(const language::Compability::semantics::Symbol &symbol) override final {
    return language::Compability::lower::mangle::mangleName(
        symbol, scopeBlockIdMap, /*keepExternalInScope=*/false,
        getLoweringOptions().getUnderscoring());
  }
  std::string mangleName(
      const language::Compability::semantics::DerivedTypeSpec &derivedType) override final {
    return language::Compability::lower::mangle::mangleName(derivedType, scopeBlockIdMap);
  }
  std::string mangleName(std::string &name) override final {
    return language::Compability::lower::mangle::mangleName(name, getCurrentScope(),
                                              scopeBlockIdMap);
  }
  std::string
  mangleName(std::string &name,
             const language::Compability::semantics::Scope &myScope) override final {
    return language::Compability::lower::mangle::mangleName(name, myScope, scopeBlockIdMap);
  }
  std::string getRecordTypeFieldName(
      const language::Compability::semantics::Symbol &component) override final {
    return language::Compability::lower::mangle::getRecordTypeFieldName(component,
                                                          scopeBlockIdMap);
  }
  const fir::KindMapping &getKindMap() override final {
    return bridge.getKindMap();
  }

  /// Return the current function context, which may be a nested BLOCK context
  /// or a full subprogram context.
  language::Compability::lower::StatementContext &getFctCtx() override final {
    if (!activeConstructStack.empty() &&
        activeConstructStack.back().eval.isA<language::Compability::parser::BlockConstruct>())
      return activeConstructStack.back().stmtCtx;
    return bridge.fctCtx();
  }

  mlir::Value hostAssocTupleValue() override final { return hostAssocTuple; }

  /// Record a binding for the ssa-value of the tuple for this function.
  void bindHostAssocTuple(mlir::Value val) override final {
    assert(!hostAssocTuple && val);
    hostAssocTuple = val;
  }

  mlir::Value dummyArgsScopeValue() const override final {
    return dummyArgsScope;
  }

  bool isRegisteredDummySymbol(
      language::Compability::semantics::SymbolRef symRef) const override final {
    auto *sym = &*symRef;
    return registeredDummySymbols.contains(sym);
  }

  const language::Compability::lower::pft::FunctionLikeUnit *
  getCurrentFunctionUnit() const override final {
    return currentFunctionUnit;
  }

  void registerTypeInfo(mlir::Location loc,
                        language::Compability::lower::SymbolRef typeInfoSym,
                        const language::Compability::semantics::DerivedTypeSpec &typeSpec,
                        fir::RecordType type) override final {
    typeInfoConverter.registerTypeInfo(*this, loc, typeInfoSym, typeSpec, type);
  }

  toolchain::StringRef
  getUniqueLitName(mlir::Location loc,
                   std::unique_ptr<language::Compability::lower::SomeExpr> expr,
                   mlir::Type eleTy) override final {
    std::string namePrefix =
        getConstantExprManglePrefix(loc, *expr.get(), eleTy);
    auto [it, inserted] = literalNamesMap.try_emplace(
        expr.get(), namePrefix + std::to_string(uniqueLitId));
    const auto &name = it->second;
    if (inserted) {
      // Keep ownership of the expr key.
      literalExprsStorage.push_back(std::move(expr));

      // If we've just added a new name, we have to make sure
      // there is no global object with the same name in the module.
      fir::GlobalOp global = builder->getNamedGlobal(name);
      if (global)
        fir::emitFatalError(loc, toolchain::Twine("global object with name '") +
                                     toolchain::Twine(name) +
                                     toolchain::Twine("' already exists"));
      ++uniqueLitId;
      return name;
    }

    // The name already exists. Verify that the prefix is the same.
    if (!toolchain::StringRef(name).starts_with(namePrefix))
      fir::emitFatalError(loc, toolchain::Twine("conflicting prefixes: '") +
                                   toolchain::Twine(name) +
                                   toolchain::Twine("' does not start with '") +
                                   toolchain::Twine(namePrefix) + toolchain::Twine("'"));

    return name;
  }

  /// Find the symbol in the inner-most level of the local map or return null.
  language::Compability::lower::SymbolBox
  shallowLookupSymbol(const language::Compability::semantics::Symbol &sym) override {
    if (language::Compability::lower::SymbolBox v = localSymbols.shallowLookupSymbol(sym))
      return v;
    return {};
  }

private:
  FirConverter() = delete;
  FirConverter(const FirConverter &) = delete;
  FirConverter &operator=(const FirConverter &) = delete;

  //===--------------------------------------------------------------------===//
  // Helper member functions
  //===--------------------------------------------------------------------===//

  mlir::Value createFIRExpr(mlir::Location loc,
                            const language::Compability::lower::SomeExpr *expr,
                            language::Compability::lower::StatementContext &stmtCtx) {
    return fir::getBase(genExprValue(*expr, stmtCtx, &loc));
  }

  /// Find the symbol in the local map or return null.
  language::Compability::lower::SymbolBox
  lookupSymbol(const language::Compability::semantics::Symbol &sym,
               language::Compability::lower::SymMap *symMap = nullptr,
               bool forceHlfirBase = false) {
    symMap = symMap ? symMap : &localSymbols;
    if (lowerToHighLevelFIR()) {
      if (std::optional<fir::FortranVariableOpInterface> var =
              symMap->lookupVariableDefinition(sym)) {
        auto exv = hlfir::translateToExtendedValue(toLocation(), *builder, *var,
                                                   forceHlfirBase);
        return exv.match(
            [](mlir::Value x) -> language::Compability::lower::SymbolBox {
              return language::Compability::lower::SymbolBox::Intrinsic{x};
            },
            [](auto x) -> language::Compability::lower::SymbolBox { return x; });
      }

      // Entry character result represented as an argument pair
      // needs to be represented in the symbol table even before
      // we can create DeclareOp for it. The temporary mapping
      // is EmboxCharOp that conveys the address and length information.
      // After mapSymbolAttributes is done, the mapping is replaced
      // with the new DeclareOp, and the following table lookups
      // do not reach here.
      if (sym.IsFuncResult())
        if (const language::Compability::semantics::DeclTypeSpec *declTy = sym.GetType())
          if (declTy->category() ==
              language::Compability::semantics::DeclTypeSpec::Category::Character)
            return symMap->lookupSymbol(sym);

      // Procedure dummies are not mapped with an hlfir.declare because
      // they are not "variable" (cannot be assigned to), and it would
      // make hlfir.declare more complex than it needs to to allow this.
      // Do a regular lookup.
      if (language::Compability::semantics::IsProcedure(sym))
        return symMap->lookupSymbol(sym);

      // Commonblock names are not variables, but in some lowerings (like
      // OpenMP) it is useful to maintain the address of the commonblock in an
      // MLIR value and query it. hlfir.declare need not be created for these.
      if (sym.detailsIf<language::Compability::semantics::CommonBlockDetails>())
        return symMap->lookupSymbol(sym);

      // For symbols to be privatized in OMP, the symbol is mapped to an
      // instance of `SymbolBox::Intrinsic` (i.e. a direct mapping to an MLIR
      // SSA value). This MLIR SSA value is the block argument to the
      // `omp.private`'s `alloc` block. If this is the case, we return this
      // `SymbolBox::Intrinsic` value.
      if (language::Compability::lower::SymbolBox v = symMap->lookupSymbol(sym))
        return v;

      return {};
    }
    if (language::Compability::lower::SymbolBox v = symMap->lookupSymbol(sym))
      return v;
    return {};
  }

  /// Find the symbol in one level up of symbol map such as for host-association
  /// in OpenMP code or return null.
  language::Compability::lower::SymbolBox
  lookupOneLevelUpSymbol(const language::Compability::semantics::Symbol &sym) override {
    if (language::Compability::lower::SymbolBox v = localSymbols.lookupOneLevelUpSymbol(sym))
      return v;
    return {};
  }

  mlir::SymbolTable *getMLIRSymbolTable() override { return &mlirSymbolTable; }

  mlir::StateStack &getStateStack() override { return stateStack; }

  /// Add the symbol to the local map and return `true`. If the symbol is
  /// already in the map and \p forced is `false`, the map is not updated.
  /// Instead the value `false` is returned.
  bool addSymbol(const language::Compability::semantics::SymbolRef sym,
                 fir::ExtendedValue val, bool forced = false) {
    if (!forced && lookupSymbol(sym))
      return false;
    if (lowerToHighLevelFIR()) {
      language::Compability::lower::genDeclareSymbol(*this, localSymbols, sym, val,
                                       fir::FortranVariableFlagsEnum::None,
                                       forced);
    } else {
      localSymbols.addSymbol(sym, val, forced);
    }
    return true;
  }

  void copyVar(const language::Compability::semantics::Symbol &sym,
               const language::Compability::lower::SymbolBox &lhs_sb,
               const language::Compability::lower::SymbolBox &rhs_sb,
               language::Compability::semantics::Symbol::Flags flags) {
    mlir::Location loc = genLocation(sym.name());
    if (lowerToHighLevelFIR())
      copyVarHLFIR(loc, lhs_sb, rhs_sb, flags);
    else
      copyVarFIR(loc, sym, lhs_sb, rhs_sb);
  }

  void copyVarHLFIR(mlir::Location loc, language::Compability::lower::SymbolBox dst,
                    language::Compability::lower::SymbolBox src,
                    language::Compability::semantics::Symbol::Flags flags) {
    assert(lowerToHighLevelFIR());

    bool isBoxAllocatable = dst.match(
        [](const fir::MutableBoxValue &box) { return box.isAllocatable(); },
        [](const fir::FortranVariableOpInterface &box) {
          return fir::FortranVariableOpInterface(box).isAllocatable();
        },
        [](const auto &box) { return false; });

    bool isBoxPointer = dst.match(
        [](const fir::MutableBoxValue &box) { return box.isPointer(); },
        [](const fir::FortranVariableOpInterface &box) {
          return fir::FortranVariableOpInterface(box).isPointer();
        },
        [](const fir::AbstractBox &box) {
          return fir::isBoxProcAddressType(box.getAddr().getType());
        },
        [](const auto &box) { return false; });

    copyVarHLFIR(loc, dst, src, isBoxAllocatable, isBoxPointer, flags);
  }

  void copyVarHLFIR(mlir::Location loc, language::Compability::lower::SymbolBox dst,
                    language::Compability::lower::SymbolBox src, bool isAllocatable,
                    bool isPointer, language::Compability::semantics::Symbol::Flags flags) {
    assert(lowerToHighLevelFIR());
    hlfir::Entity lhs{dst.getAddr()};
    hlfir::Entity rhs{src.getAddr()};

    auto copyData = [&](hlfir::Entity l, hlfir::Entity r) {
      // Dereference RHS and load it if trivial scalar.
      r = hlfir::loadTrivialScalar(loc, *builder, r);
      hlfir::AssignOp::create(*builder, loc, r, l, isAllocatable);
    };

    if (isPointer) {
      // Set LHS target to the target of RHS (do not copy the RHS
      // target data into the LHS target storage).
      auto loadVal = fir::LoadOp::create(*builder, loc, rhs);
      fir::StoreOp::create(*builder, loc, loadVal, lhs);
    } else if (isAllocatable &&
               flags.test(language::Compability::semantics::Symbol::Flag::OmpCopyIn)) {
      // For copyin allocatable variables, RHS must be copied to lhs
      // only when rhs is allocated.
      hlfir::Entity temp =
          hlfir::derefPointersAndAllocatables(loc, *builder, rhs);
      mlir::Value addr = hlfir::genVariableRawAddress(loc, *builder, temp);
      mlir::Value isAllocated = builder->genIsNotNullAddr(loc, addr);
      builder->genIfThenElse(loc, isAllocated)
          .genThen([&]() { copyData(lhs, rhs); })
          .genElse([&]() {
            fir::ExtendedValue hexv = symBoxToExtendedValue(dst);
            hexv.match(
                [&](const fir::MutableBoxValue &new_box) -> void {
                  // if the allocation status of original list item is
                  // unallocated, unallocate the copy if it is allocated, else
                  // do nothing.
                  language::Compability::lower::genDeallocateIfAllocated(*this, new_box, loc);
                },
                [&](const auto &) -> void {});
          })
          .end();
    } else if (isAllocatable &&
               flags.test(language::Compability::semantics::Symbol::Flag::OmpFirstPrivate)) {
      // For firstprivate allocatable variables, RHS must be copied
      // only when LHS is allocated.
      hlfir::Entity temp =
          hlfir::derefPointersAndAllocatables(loc, *builder, lhs);
      mlir::Value addr = hlfir::genVariableRawAddress(loc, *builder, temp);
      mlir::Value isAllocated = builder->genIsNotNullAddr(loc, addr);
      builder->genIfThen(loc, isAllocated)
          .genThen([&]() { copyData(lhs, rhs); })
          .end();
    } else {
      copyData(lhs, rhs);
    }
  }

  void copyVarFIR(mlir::Location loc, const language::Compability::semantics::Symbol &sym,
                  const language::Compability::lower::SymbolBox &lhs_sb,
                  const language::Compability::lower::SymbolBox &rhs_sb) {
    assert(!lowerToHighLevelFIR());
    fir::ExtendedValue lhs = symBoxToExtendedValue(lhs_sb);
    fir::ExtendedValue rhs = symBoxToExtendedValue(rhs_sb);
    mlir::Type symType = genType(sym);
    if (auto seqTy = mlir::dyn_cast<fir::SequenceType>(symType)) {
      language::Compability::lower::StatementContext stmtCtx;
      language::Compability::lower::createSomeArrayAssignment(*this, lhs, rhs, localSymbols,
                                                stmtCtx);
      stmtCtx.finalizeAndReset();
    } else if (lhs.getBoxOf<fir::CharBoxValue>()) {
      fir::factory::CharacterExprHelper{*builder, loc}.createAssign(lhs, rhs);
    } else {
      auto loadVal = fir::LoadOp::create(*builder, loc, fir::getBase(rhs));
      fir::StoreOp::create(*builder, loc, loadVal, fir::getBase(lhs));
    }
  }

  /// Map a block argument to a result or dummy symbol. This is not the
  /// definitive mapping. The specification expression have not been lowered
  /// yet. The final mapping will be done using this pre-mapping in
  /// language::Compability::lower::mapSymbolAttributes.
  bool mapBlockArgToDummyOrResult(const language::Compability::semantics::SymbolRef sym,
                                  mlir::Value val, bool isResult) {
    localSymbols.addSymbol(sym, val);
    if (!isResult)
      registerDummySymbol(sym);

    return true;
  }

  /// Generate the address of loop variable \p sym.
  /// If \p sym is not mapped yet, allocate local storage for it.
  mlir::Value genLoopVariableAddress(mlir::Location loc,
                                     const language::Compability::semantics::Symbol &sym,
                                     bool isUnordered) {
    if (!shallowLookupSymbol(sym) &&
        (isUnordered ||
         GetSymbolDSA(sym).test(language::Compability::semantics::Symbol::Flag::OmpPrivate) ||
         GetSymbolDSA(sym).test(
             language::Compability::semantics::Symbol::Flag::OmpFirstPrivate) ||
         GetSymbolDSA(sym).test(
             language::Compability::semantics::Symbol::Flag::OmpLastPrivate) ||
         GetSymbolDSA(sym).test(language::Compability::semantics::Symbol::Flag::OmpLinear))) {
      // Do concurrent loop variables are not mapped yet since they are
      // local to the Do concurrent scope (same for OpenMP loops).
      mlir::OpBuilder::InsertPoint insPt = builder->saveInsertionPoint();
      builder->setInsertionPointToStart(builder->getAllocaBlock());
      mlir::Type tempTy = genType(sym);
      mlir::Value temp =
          builder->createTemporaryAlloc(loc, tempTy, toStringRef(sym.name()));
      bindIfNewSymbol(sym, temp);
      builder->restoreInsertionPoint(insPt);
    }
    auto entry = lookupSymbol(sym);
    (void)entry;
    assert(entry && "loop control variable must already be in map");
    language::Compability::lower::StatementContext stmtCtx;
    return fir::getBase(
        genExprAddr(language::Compability::evaluate::AsGenericExpr(sym).value(), stmtCtx));
  }

  static bool isNumericScalarCategory(language::Compability::common::TypeCategory cat) {
    return cat == language::Compability::common::TypeCategory::Integer ||
           cat == language::Compability::common::TypeCategory::Real ||
           cat == language::Compability::common::TypeCategory::Complex ||
           cat == language::Compability::common::TypeCategory::Logical;
  }
  static bool isLogicalCategory(language::Compability::common::TypeCategory cat) {
    return cat == language::Compability::common::TypeCategory::Logical;
  }
  static bool isCharacterCategory(language::Compability::common::TypeCategory cat) {
    return cat == language::Compability::common::TypeCategory::Character;
  }
  static bool isDerivedCategory(language::Compability::common::TypeCategory cat) {
    return cat == language::Compability::common::TypeCategory::Derived;
  }

  /// Insert a new block before \p block. Leave the insertion point unchanged.
  mlir::Block *insertBlock(mlir::Block *block) {
    mlir::OpBuilder::InsertPoint insertPt = builder->saveInsertionPoint();
    mlir::Block *newBlock = builder->createBlock(block);
    builder->restoreInsertionPoint(insertPt);
    return newBlock;
  }

  language::Compability::lower::pft::Evaluation &evalOfLabel(language::Compability::parser::Label label) {
    const language::Compability::lower::pft::LabelEvalMap &labelEvaluationMap =
        getEval().getOwningProcedure()->labelEvaluationMap;
    const auto iter = labelEvaluationMap.find(label);
    assert(iter != labelEvaluationMap.end() && "label missing from map");
    return *iter->second;
  }

  void genBranch(mlir::Block *targetBlock) {
    assert(targetBlock && "missing unconditional target block");
    mlir::cf::BranchOp::create(*builder, toLocation(), targetBlock);
  }

  void genConditionalBranch(mlir::Value cond, mlir::Block *trueTarget,
                            mlir::Block *falseTarget) {
    assert(trueTarget && "missing conditional branch true block");
    assert(falseTarget && "missing conditional branch false block");
    mlir::Location loc = toLocation();
    mlir::Value bcc = builder->createConvert(loc, builder->getI1Type(), cond);
    mlir::cf::CondBranchOp::create(*builder, loc, bcc, trueTarget,
                                   mlir::ValueRange{}, falseTarget,
                                   mlir::ValueRange{});
  }
  void genConditionalBranch(mlir::Value cond,
                            language::Compability::lower::pft::Evaluation *trueTarget,
                            language::Compability::lower::pft::Evaluation *falseTarget) {
    genConditionalBranch(cond, trueTarget->block, falseTarget->block);
  }
  void genConditionalBranch(const language::Compability::parser::ScalarLogicalExpr &expr,
                            mlir::Block *trueTarget, mlir::Block *falseTarget) {
    language::Compability::lower::StatementContext stmtCtx;
    mlir::Value cond =
        createFIRExpr(toLocation(), language::Compability::semantics::GetExpr(expr), stmtCtx);
    stmtCtx.finalizeAndReset();
    genConditionalBranch(cond, trueTarget, falseTarget);
  }
  void genConditionalBranch(const language::Compability::parser::ScalarLogicalExpr &expr,
                            language::Compability::lower::pft::Evaluation *trueTarget,
                            language::Compability::lower::pft::Evaluation *falseTarget) {
    language::Compability::lower::StatementContext stmtCtx;
    mlir::Value cond =
        createFIRExpr(toLocation(), language::Compability::semantics::GetExpr(expr), stmtCtx);
    stmtCtx.finalizeAndReset();
    genConditionalBranch(cond, trueTarget->block, falseTarget->block);
  }

  /// Return the nearest active ancestor construct of \p eval, or nullptr.
  language::Compability::lower::pft::Evaluation *
  getActiveAncestor(const language::Compability::lower::pft::Evaluation &eval) {
    language::Compability::lower::pft::Evaluation *ancestor = eval.parentConstruct;
    for (; ancestor; ancestor = ancestor->parentConstruct)
      if (ancestor->activeConstruct)
        break;
    return ancestor;
  }

  /// Return the predicate: "a branch to \p targetEval has exit code".
  bool hasExitCode(const language::Compability::lower::pft::Evaluation &targetEval) {
    language::Compability::lower::pft::Evaluation *activeAncestor =
        getActiveAncestor(targetEval);
    for (auto it = activeConstructStack.rbegin(),
              rend = activeConstructStack.rend();
         it != rend; ++it) {
      if (&it->eval == activeAncestor)
        break;
      if (it->stmtCtx.hasCode())
        return true;
    }
    return false;
  }

  /// Generate a branch to \p targetEval after generating on-exit code for
  /// any enclosing construct scopes that are exited by taking the branch.
  void
  genConstructExitBranch(const language::Compability::lower::pft::Evaluation &targetEval) {
    language::Compability::lower::pft::Evaluation *activeAncestor =
        getActiveAncestor(targetEval);
    for (auto it = activeConstructStack.rbegin(),
              rend = activeConstructStack.rend();
         it != rend; ++it) {
      if (&it->eval == activeAncestor)
        break;
      it->stmtCtx.finalizeAndKeep();
    }
    genBranch(targetEval.block);
  }

  /// A construct contains nested evaluations. Some of these evaluations
  /// may start a new basic block, others will add code to an existing
  /// block.
  /// Collect the list of nested evaluations that are last in their block,
  /// organize them into two sets:
  /// 1. Exiting evaluations: they may need a branch exiting from their
  ///    parent construct,
  /// 2. Fall-through evaluations: they will continue to the following
  ///    evaluation. They may still need a branch, but they do not exit
  ///    the construct. They appear in cases where the following evaluation
  ///    is a target of some branch.
  void collectFinalEvaluations(
      language::Compability::lower::pft::Evaluation &construct,
      toolchain::SmallVector<language::Compability::lower::pft::Evaluation *> &exits,
      toolchain::SmallVector<language::Compability::lower::pft::Evaluation *> &fallThroughs) {
    language::Compability::lower::pft::EvaluationList &nested =
        construct.getNestedEvaluations();
    if (nested.empty())
      return;

    language::Compability::lower::pft::Evaluation *exit = construct.constructExit;
    language::Compability::lower::pft::Evaluation *previous = &nested.front();

    for (auto it = ++nested.begin(), end = nested.end(); it != end;
         previous = &*it++) {
      if (it->block == nullptr)
        continue;
      // "*it" starts a new block, check what to do with "previous"
      if (it->isIntermediateConstructStmt() && previous != exit)
        exits.push_back(previous);
      else if (previous->lexicalSuccessor && previous->lexicalSuccessor->block)
        fallThroughs.push_back(previous);
    }
    if (previous != exit)
      exits.push_back(previous);
  }

  /// Generate a SelectOp or branch sequence that compares \p selector against
  /// values in \p valueList and targets corresponding labels in \p labelList.
  /// If no value matches the selector, branch to \p defaultEval.
  ///
  /// Three cases require special processing.
  ///
  /// An empty \p valueList indicates an ArithmeticIfStmt context that requires
  /// two comparisons against 0 or 0.0. The selector may have either INTEGER
  /// or REAL type.
  ///
  /// A nonpositive \p valuelist value indicates an IO statement context
  /// (0 for ERR, -1 for END, -2 for EOR). An ERR branch must be taken for
  /// any positive (IOSTAT) value. A missing (zero) label requires a branch
  /// to \p defaultEval for that value.
  ///
  /// A non-null \p errorBlock indicates an AssignedGotoStmt context that
  /// must always branch to an explicit target. There is no valid defaultEval
  /// in this case. Generate a branch to \p errorBlock for an AssignedGotoStmt
  /// that violates this program requirement.
  ///
  /// If this is not an ArithmeticIfStmt and no targets have exit code,
  /// generate a SelectOp. Otherwise, for each target, if it has exit code,
  /// branch to a new block, insert exit code, and then branch to the target.
  /// Otherwise, branch directly to the target.
  void genMultiwayBranch(mlir::Value selector,
                         toolchain::SmallVector<int64_t> valueList,
                         toolchain::SmallVector<language::Compability::parser::Label> labelList,
                         const language::Compability::lower::pft::Evaluation &defaultEval,
                         mlir::Block *errorBlock = nullptr) {
    bool inArithmeticIfContext = valueList.empty();
    assert(((inArithmeticIfContext && labelList.size() == 2) ||
            (valueList.size() && labelList.size() == valueList.size())) &&
           "mismatched multiway branch targets");
    mlir::Block *defaultBlock = errorBlock ? errorBlock : defaultEval.block;
    bool defaultHasExitCode = !errorBlock && hasExitCode(defaultEval);
    bool hasAnyExitCode = defaultHasExitCode;
    if (!hasAnyExitCode)
      for (auto label : labelList)
        if (label && hasExitCode(evalOfLabel(label))) {
          hasAnyExitCode = true;
          break;
        }
    mlir::Location loc = toLocation();
    size_t branchCount = labelList.size();
    if (!inArithmeticIfContext && !hasAnyExitCode &&
        !getEval().forceAsUnstructured()) { // from -no-structured-fir option
      // Generate a SelectOp.
      toolchain::SmallVector<mlir::Block *> blockList;
      for (auto label : labelList) {
        mlir::Block *block =
            label ? evalOfLabel(label).block : defaultEval.block;
        assert(block && "missing multiway branch block");
        blockList.push_back(block);
      }
      blockList.push_back(defaultBlock);
      if (valueList[branchCount - 1] == 0) // Swap IO ERR and default blocks.
        std::swap(blockList[branchCount - 1], blockList[branchCount]);
      fir::SelectOp::create(*builder, loc, selector, valueList, blockList);
      return;
    }
    mlir::Type selectorType = selector.getType();
    bool realSelector = mlir::isa<mlir::FloatType>(selectorType);
    assert((inArithmeticIfContext || !realSelector) && "invalid selector type");
    mlir::Value zero;
    if (inArithmeticIfContext)
      zero = realSelector
                 ? mlir::arith::ConstantOp::create(
                       *builder, loc, selectorType,
                       builder->getFloatAttr(selectorType, 0.0))
                 : builder->createIntegerConstant(loc, selectorType, 0);
    for (auto label : toolchain::enumerate(labelList)) {
      mlir::Value cond;
      if (realSelector) // inArithmeticIfContext
        cond = mlir::arith::CmpFOp::create(
            *builder, loc,
            label.index() == 0 ? mlir::arith::CmpFPredicate::OLT
                               : mlir::arith::CmpFPredicate::OGT,
            selector, zero);
      else if (inArithmeticIfContext) // INTEGER selector
        cond = mlir::arith::CmpIOp::create(
            *builder, loc,
            label.index() == 0 ? mlir::arith::CmpIPredicate::slt
                               : mlir::arith::CmpIPredicate::sgt,
            selector, zero);
      else // A value of 0 is an IO ERR branch: invert comparison.
        cond = mlir::arith::CmpIOp::create(
            *builder, loc,
            valueList[label.index()] == 0 ? mlir::arith::CmpIPredicate::ne
                                          : mlir::arith::CmpIPredicate::eq,
            selector,
            builder->createIntegerConstant(loc, selectorType,
                                           valueList[label.index()]));
      // Branch to a new block with exit code and then to the target, or branch
      // directly to the target. defaultBlock is the "else" target.
      bool lastBranch = label.index() == branchCount - 1;
      mlir::Block *nextBlock =
          lastBranch && !defaultHasExitCode
              ? defaultBlock
              : builder->getBlock()->splitBlock(builder->getInsertionPoint());
      const language::Compability::lower::pft::Evaluation &targetEval =
          label.value() ? evalOfLabel(label.value()) : defaultEval;
      if (hasExitCode(targetEval)) {
        mlir::Block *jumpBlock =
            builder->getBlock()->splitBlock(builder->getInsertionPoint());
        genConditionalBranch(cond, jumpBlock, nextBlock);
        startBlock(jumpBlock);
        genConstructExitBranch(targetEval);
      } else {
        genConditionalBranch(cond, targetEval.block, nextBlock);
      }
      if (!lastBranch) {
        startBlock(nextBlock);
      } else if (defaultHasExitCode) {
        startBlock(nextBlock);
        genConstructExitBranch(defaultEval);
      }
    }
  }

  void pushActiveConstruct(language::Compability::lower::pft::Evaluation &eval,
                           language::Compability::lower::StatementContext &stmtCtx) {
    activeConstructStack.push_back(ConstructContext{eval, stmtCtx});
    eval.activeConstruct = true;
  }
  void popActiveConstruct() {
    assert(!activeConstructStack.empty() && "invalid active construct stack");
    activeConstructStack.back().eval.activeConstruct = false;
    if (activeConstructStack.back().pushedScope)
      localSymbols.popScope();
    activeConstructStack.pop_back();
  }

  //===--------------------------------------------------------------------===//
  // Termination of symbolically referenced execution units
  //===--------------------------------------------------------------------===//

  /// Exit of a routine
  ///
  /// Generate the cleanup block before the routine exits
  void genExitRoutine(bool earlyReturn, mlir::ValueRange retval = {}) {
    if (blockIsUnterminated()) {
      bridge.openAccCtx().finalizeAndKeep();
      bridge.fctCtx().finalizeAndKeep();
      mlir::func::ReturnOp::create(*builder, toLocation(), retval);
    }
    if (!earlyReturn) {
      bridge.openAccCtx().pop();
      bridge.fctCtx().pop();
    }
  }

  /// END of procedure-like constructs
  ///
  /// Generate the cleanup block before the procedure exits
  void genReturnSymbol(const language::Compability::semantics::Symbol &functionSymbol) {
    const language::Compability::semantics::Symbol &resultSym =
        functionSymbol.get<language::Compability::semantics::SubprogramDetails>().result();
    language::Compability::lower::SymbolBox resultSymBox = lookupSymbol(resultSym);
    mlir::Location loc = toLocation();
    if (!resultSymBox) {
      // Create a dummy undefined value of the expected return type.
      // This prevents improper cleanup of StatementContext, which would lead
      // to a crash due to a block with no terminator. See issue #126452.
      mlir::FunctionType funcType = builder->getFunction().getFunctionType();
      mlir::Type resultType = funcType.getResult(0);
      mlir::Value undefResult = builder->create<fir::UndefOp>(loc, resultType);
      genExitRoutine(false, undefResult);
      return;
    }
    mlir::Value resultVal = resultSymBox.match(
        [&](const fir::CharBoxValue &x) -> mlir::Value {
          if (language::Compability::semantics::IsBindCProcedure(functionSymbol))
            return fir::LoadOp::create(*builder, loc, x.getBuffer());
          return fir::factory::CharacterExprHelper{*builder, loc}
              .createEmboxChar(x.getBuffer(), x.getLen());
        },
        [&](const fir::MutableBoxValue &x) -> mlir::Value {
          mlir::Value resultRef = resultSymBox.getAddr();
          mlir::Value load = fir::LoadOp::create(*builder, loc, resultRef);
          unsigned rank = x.rank();
          if (x.isAllocatable() && rank > 0) {
            // ALLOCATABLE array result must have default lower bounds.
            // At the call site the result box of a function reference
            // might be considered having default lower bounds, but
            // the runtime box should probably comply with this assumption
            // as well. If the result box has proper lbounds in runtime,
            // this may improve the debugging experience of Fortran apps.
            // We may consider removing this, if the overhead of setting
            // default lower bounds is too big.
            mlir::Value one =
                builder->createIntegerConstant(loc, builder->getIndexType(), 1);
            toolchain::SmallVector<mlir::Value> lbounds{rank, one};
            auto shiftTy = fir::ShiftType::get(builder->getContext(), rank);
            mlir::Value shiftOp =
                fir::ShiftOp::create(*builder, loc, shiftTy, lbounds);
            load = fir::ReboxOp::create(*builder, loc, load.getType(), load,
                                        shiftOp, /*slice=*/mlir::Value{});
          }
          return load;
        },
        [&](const auto &) -> mlir::Value {
          mlir::Value resultRef = resultSymBox.getAddr();
          mlir::Type resultType = genType(resultSym);
          mlir::Type resultRefType = builder->getRefType(resultType);
          // A function with multiple entry points returning different types
          // tags all result variables with one of the largest types to allow
          // them to share the same storage. Convert this to the actual type.
          if (resultRef.getType() != resultRefType)
            resultRef = builder->createConvertWithVolatileCast(
                loc, resultRefType, resultRef);
          return fir::LoadOp::create(*builder, loc, resultRef);
        });
    genExitRoutine(false, resultVal);
  }

  /// Get the return value of a call to \p symbol, which is a subroutine entry
  /// point that has alternative return specifiers.
  const mlir::Value
  getAltReturnResult(const language::Compability::semantics::Symbol &symbol) {
    assert(language::Compability::semantics::HasAlternateReturns(symbol) &&
           "subroutine does not have alternate returns");
    return getSymbolAddress(symbol);
  }

  void genFIRProcedureExit(language::Compability::lower::pft::FunctionLikeUnit &funit,
                           const language::Compability::semantics::Symbol &symbol) {
    if (mlir::Block *finalBlock = funit.finalBlock) {
      // The current block must end with a terminator.
      if (blockIsUnterminated())
        mlir::cf::BranchOp::create(*builder, toLocation(), finalBlock);
      // Set insertion point to final block.
      builder->setInsertionPoint(finalBlock, finalBlock->end());
    }
    if (language::Compability::semantics::IsFunction(symbol)) {
      genReturnSymbol(symbol);
    } else if (language::Compability::semantics::HasAlternateReturns(symbol)) {
      mlir::Value retval = fir::LoadOp::create(*builder, toLocation(),
                                               getAltReturnResult(symbol));
      genExitRoutine(false, retval);
    } else {
      genExitRoutine(false);
    }
  }

  //
  // Statements that have control-flow semantics
  //

  /// Generate an If[Then]Stmt condition or its negation.
  template <typename A>
  mlir::Value genIfCondition(const A *stmt, bool negate = false) {
    mlir::Location loc = toLocation();
    language::Compability::lower::StatementContext stmtCtx;
    mlir::Value condExpr = createFIRExpr(
        loc,
        language::Compability::semantics::GetExpr(
            std::get<language::Compability::parser::ScalarLogicalExpr>(stmt->t)),
        stmtCtx);
    stmtCtx.finalizeAndReset();
    mlir::Value cond =
        builder->createConvert(loc, builder->getI1Type(), condExpr);
    if (negate)
      cond = mlir::arith::XOrIOp::create(
          *builder, loc, cond,
          builder->createIntegerConstant(loc, cond.getType(), 1));
    return cond;
  }

  mlir::func::FuncOp getFunc(toolchain::StringRef name, mlir::FunctionType ty) {
    if (mlir::func::FuncOp func = builder->getNamedFunction(name)) {
      assert(func.getFunctionType() == ty);
      return func;
    }
    return builder->createFunction(toLocation(), name, ty);
  }

  /// Lowering of CALL statement
  void genFIR(const language::Compability::parser::CallStmt &stmt) {
    language::Compability::lower::StatementContext stmtCtx;
    language::Compability::lower::pft::Evaluation &eval = getEval();
    setCurrentPosition(stmt.source);
    assert(stmt.typedCall && "Call was not analyzed");
    mlir::Value res{};
    if (lowerToHighLevelFIR()) {
      std::optional<mlir::Type> resultType;
      if (stmt.typedCall->hasAlternateReturns())
        resultType = builder->getIndexType();
      auto hlfirRes = language::Compability::lower::convertCallToHLFIR(
          toLocation(), *this, *stmt.typedCall, resultType, localSymbols,
          stmtCtx);
      if (hlfirRes)
        res = *hlfirRes;
    } else {
      // Call statement lowering shares code with function call lowering.
      res = language::Compability::lower::createSubroutineCall(
          *this, *stmt.typedCall, explicitIterSpace, implicitIterSpace,
          localSymbols, stmtCtx, /*isUserDefAssignment=*/false);
    }
    stmtCtx.finalizeAndReset();
    if (!res)
      return; // "Normal" subroutine call.
    // Call with alternate return specifiers.
    // The call returns an index that selects an alternate return branch target.
    toolchain::SmallVector<int64_t> indexList;
    toolchain::SmallVector<language::Compability::parser::Label> labelList;
    int64_t index = 0;
    for (const language::Compability::parser::ActualArgSpec &arg :
         std::get<std::list<language::Compability::parser::ActualArgSpec>>(stmt.call.t)) {
      const auto &actual = std::get<language::Compability::parser::ActualArg>(arg.t);
      if (const auto *altReturn =
              std::get_if<language::Compability::parser::AltReturnSpec>(&actual.u)) {
        indexList.push_back(++index);
        labelList.push_back(altReturn->v);
      }
    }
    genMultiwayBranch(res, indexList, labelList, eval.nonNopSuccessor());
  }

  void genFIR(const language::Compability::parser::ComputedGotoStmt &stmt) {
    language::Compability::lower::StatementContext stmtCtx;
    language::Compability::lower::pft::Evaluation &eval = getEval();
    mlir::Value selectExpr =
        createFIRExpr(toLocation(),
                      language::Compability::semantics::GetExpr(
                          std::get<language::Compability::parser::ScalarIntExpr>(stmt.t)),
                      stmtCtx);
    stmtCtx.finalizeAndReset();
    toolchain::SmallVector<int64_t> indexList;
    toolchain::SmallVector<language::Compability::parser::Label> labelList;
    int64_t index = 0;
    for (language::Compability::parser::Label label :
         std::get<std::list<language::Compability::parser::Label>>(stmt.t)) {
      indexList.push_back(++index);
      labelList.push_back(label);
    }
    genMultiwayBranch(selectExpr, indexList, labelList, eval.nonNopSuccessor());
  }

  void genFIR(const language::Compability::parser::ArithmeticIfStmt &stmt) {
    language::Compability::lower::StatementContext stmtCtx;
    mlir::Value expr = createFIRExpr(
        toLocation(),
        language::Compability::semantics::GetExpr(std::get<language::Compability::parser::Expr>(stmt.t)),
        stmtCtx);
    stmtCtx.finalizeAndReset();
    // Raise an exception if REAL expr is a NaN.
    if (mlir::isa<mlir::FloatType>(expr.getType()))
      expr = mlir::arith::AddFOp::create(*builder, toLocation(), expr, expr);
    // An empty valueList indicates to genMultiwayBranch that the branch is
    // an ArithmeticIfStmt that has two branches on value 0 or 0.0.
    toolchain::SmallVector<int64_t> valueList;
    toolchain::SmallVector<language::Compability::parser::Label> labelList;
    labelList.push_back(std::get<1>(stmt.t));
    labelList.push_back(std::get<3>(stmt.t));
    const language::Compability::lower::pft::LabelEvalMap &labelEvaluationMap =
        getEval().getOwningProcedure()->labelEvaluationMap;
    const auto iter = labelEvaluationMap.find(std::get<2>(stmt.t));
    assert(iter != labelEvaluationMap.end() && "label missing from map");
    genMultiwayBranch(expr, valueList, labelList, *iter->second);
  }

  void genFIR(const language::Compability::parser::AssignedGotoStmt &stmt) {
    // See Fortran 90 Clause 8.2.4.
    // Relax the requirement that the GOTO variable must have a value in the
    // label list when a list is present, and allow a branch to any non-format
    // target that has an ASSIGN statement for the variable.
    mlir::Location loc = toLocation();
    language::Compability::lower::pft::Evaluation &eval = getEval();
    language::Compability::lower::pft::FunctionLikeUnit &owningProc =
        *eval.getOwningProcedure();
    const language::Compability::lower::pft::SymbolLabelMap &symbolLabelMap =
        owningProc.assignSymbolLabelMap;
    const language::Compability::lower::pft::LabelEvalMap &labelEvalMap =
        owningProc.labelEvaluationMap;
    const language::Compability::semantics::Symbol &symbol =
        *std::get<language::Compability::parser::Name>(stmt.t).symbol;
    auto labelSetIter = symbolLabelMap.find(symbol);
    toolchain::SmallVector<int64_t> valueList;
    toolchain::SmallVector<language::Compability::parser::Label> labelList;
    if (labelSetIter != symbolLabelMap.end()) {
      for (auto &label : labelSetIter->second) {
        const auto evalIter = labelEvalMap.find(label);
        assert(evalIter != labelEvalMap.end() && "assigned goto label missing");
        if (evalIter->second->block) { // non-format statement
          valueList.push_back(label);  // label as an integer
          labelList.push_back(label);
        }
      }
    }
    if (!labelList.empty()) {
      auto selectExpr =
          fir::LoadOp::create(*builder, loc, getSymbolAddress(symbol));
      // Add a default error target in case the goto is nonconforming.
      mlir::Block *errorBlock =
          builder->getBlock()->splitBlock(builder->getInsertionPoint());
      genMultiwayBranch(selectExpr, valueList, labelList,
                        eval.nonNopSuccessor(), errorBlock);
      startBlock(errorBlock);
    }
    fir::runtime::genReportFatalUserError(
        *builder, loc,
        "Assigned GOTO variable '" + symbol.name().ToString() +
            "' does not have a valid target label value");
    fir::UnreachableOp::create(*builder, loc);
  }

  fir::ReduceOperationEnum
  getReduceOperationEnum(const language::Compability::parser::ReductionOperator &rOpr) {
    switch (rOpr.v) {
    case language::Compability::parser::ReductionOperator::Operator::Plus:
      return fir::ReduceOperationEnum::Add;
    case language::Compability::parser::ReductionOperator::Operator::Multiply:
      return fir::ReduceOperationEnum::Multiply;
    case language::Compability::parser::ReductionOperator::Operator::And:
      return fir::ReduceOperationEnum::AND;
    case language::Compability::parser::ReductionOperator::Operator::Or:
      return fir::ReduceOperationEnum::OR;
    case language::Compability::parser::ReductionOperator::Operator::Eqv:
      return fir::ReduceOperationEnum::EQV;
    case language::Compability::parser::ReductionOperator::Operator::Neqv:
      return fir::ReduceOperationEnum::NEQV;
    case language::Compability::parser::ReductionOperator::Operator::Max:
      return fir::ReduceOperationEnum::MAX;
    case language::Compability::parser::ReductionOperator::Operator::Min:
      return fir::ReduceOperationEnum::MIN;
    case language::Compability::parser::ReductionOperator::Operator::Iand:
      return fir::ReduceOperationEnum::IAND;
    case language::Compability::parser::ReductionOperator::Operator::Ior:
      return fir::ReduceOperationEnum::IOR;
    case language::Compability::parser::ReductionOperator::Operator::Ieor:
      return fir::ReduceOperationEnum::IEOR;
    }
    toolchain_unreachable("illegal reduction operator");
  }

  /// Collect DO CONCURRENT loop control information.
  IncrementLoopNestInfo getConcurrentControl(
      const language::Compability::parser::ConcurrentHeader &header,
      const std::list<language::Compability::parser::LocalitySpec> &localityList = {}) {
    IncrementLoopNestInfo incrementLoopNestInfo;
    for (const language::Compability::parser::ConcurrentControl &control :
         std::get<std::list<language::Compability::parser::ConcurrentControl>>(header.t))
      incrementLoopNestInfo.emplace_back(
          *std::get<0>(control.t).symbol, std::get<1>(control.t),
          std::get<2>(control.t), std::get<3>(control.t), /*isUnordered=*/true);
    IncrementLoopInfo &info = incrementLoopNestInfo.back();
    info.maskExpr = language::Compability::semantics::GetExpr(
        std::get<std::optional<language::Compability::parser::ScalarLogicalExpr>>(header.t));
    for (const language::Compability::parser::LocalitySpec &x : localityList) {
      if (const auto *localList =
              std::get_if<language::Compability::parser::LocalitySpec::Local>(&x.u))
        for (const language::Compability::parser::Name &x : localList->v)
          info.localSymList.push_back(x.symbol);
      if (const auto *localInitList =
              std::get_if<language::Compability::parser::LocalitySpec::LocalInit>(&x.u))
        for (const language::Compability::parser::Name &x : localInitList->v)
          info.localInitSymList.push_back(x.symbol);
      for (IncrementLoopInfo &info : incrementLoopNestInfo) {
        if (const auto *reduceList =
                std::get_if<language::Compability::parser::LocalitySpec::Reduce>(&x.u)) {
          fir::ReduceOperationEnum reduce_operation = getReduceOperationEnum(
              std::get<language::Compability::parser::ReductionOperator>(reduceList->t));
          for (const language::Compability::parser::Name &x :
               std::get<std::list<language::Compability::parser::Name>>(reduceList->t)) {
            info.reduceSymList.push_back(x.symbol);
            info.reduceOperatorList.push_back(reduce_operation);
          }
        }
      }
      if (const auto *sharedList =
              std::get_if<language::Compability::parser::LocalitySpec::Shared>(&x.u))
        for (const language::Compability::parser::Name &x : sharedList->v)
          info.sharedSymList.push_back(x.symbol);
    }
    return incrementLoopNestInfo;
  }

  /// Create DO CONCURRENT construct symbol bindings and generate LOCAL_INIT
  /// assignments.
  void handleLocalitySpecs(const IncrementLoopInfo &info) {
    language::Compability::semantics::SemanticsContext &semanticsContext =
        bridge.getSemanticsContext();
    fir::LocalitySpecifierOperands privateClauseOps;
    auto doConcurrentLoopOp =
        mlir::dyn_cast_if_present<fir::DoConcurrentLoopOp>(info.loopOp);
    // TODO Promote to using `enableDelayedPrivatization` (which is enabled by
    // default unlike the staging flag) once the implementation of this is more
    // complete.
    bool useDelayedPriv =
        enableDelayedPrivatizationStaging && doConcurrentLoopOp;
    toolchain::SetVector<const language::Compability::semantics::Symbol *> allPrivatizedSymbols;
    toolchain::SmallSet<const language::Compability::semantics::Symbol *, 16> mightHaveReadHostSym;

    for (const language::Compability::semantics::Symbol *symToPrivatize : info.localSymList) {
      if (useDelayedPriv) {
        language::Compability::lower::privatizeSymbol<fir::LocalitySpecifierOp>(
            *this, this->getFirOpBuilder(), localSymbols, allPrivatizedSymbols,
            mightHaveReadHostSym, symToPrivatize, &privateClauseOps);
        continue;
      }

      createHostAssociateVarClone(*symToPrivatize, /*skipDefaultInit=*/false);
    }

    for (const language::Compability::semantics::Symbol *symToPrivatize :
         info.localInitSymList) {
      if (useDelayedPriv) {
        language::Compability::lower::privatizeSymbol<fir::LocalitySpecifierOp>(
            *this, this->getFirOpBuilder(), localSymbols, allPrivatizedSymbols,
            mightHaveReadHostSym, symToPrivatize, &privateClauseOps);
        continue;
      }

      createHostAssociateVarClone(*symToPrivatize, /*skipDefaultInit=*/true);
      const auto *hostDetails =
          symToPrivatize->detailsIf<language::Compability::semantics::HostAssocDetails>();
      assert(hostDetails && "missing locality spec host symbol");
      const language::Compability::semantics::Symbol *hostSym = &hostDetails->symbol();
      language::Compability::evaluate::ExpressionAnalyzer ea{semanticsContext};
      language::Compability::evaluate::Assignment assign{
          ea.Designate(language::Compability::evaluate::DataRef{*symToPrivatize}).value(),
          ea.Designate(language::Compability::evaluate::DataRef{*hostSym}).value()};
      if (language::Compability::semantics::IsPointer(*symToPrivatize))
        assign.u = language::Compability::evaluate::Assignment::BoundsSpec{};
      genAssignment(assign);
    }

    for (const language::Compability::semantics::Symbol *sym : info.sharedSymList) {
      const auto *hostDetails =
          sym->detailsIf<language::Compability::semantics::HostAssocDetails>();
      copySymbolBinding(hostDetails->symbol(), *sym);
    }

    if (useDelayedPriv) {
      doConcurrentLoopOp.getLocalVarsMutable().assign(
          privateClauseOps.privateVars);
      doConcurrentLoopOp.setLocalSymsAttr(
          builder->getArrayAttr(privateClauseOps.privateSyms));

      for (auto [sym, privateVar] : toolchain::zip_equal(
               allPrivatizedSymbols, privateClauseOps.privateVars)) {
        auto arg = doConcurrentLoopOp.getRegion().begin()->addArgument(
            privateVar.getType(), doConcurrentLoopOp.getLoc());
        bindSymbol(*sym, hlfir::translateToExtendedValue(
                             privateVar.getLoc(), *builder, hlfir::Entity{arg},
                             /*contiguousHint=*/true)
                             .first);
      }
    }

    if (!doConcurrentLoopOp)
      return;

    toolchain::SmallVector<bool> reduceVarByRef;
    toolchain::SmallVector<mlir::Attribute> reductionDeclSymbols;
    toolchain::SmallVector<mlir::Attribute> nestReduceAttrs;

    for (const auto &reduceOp : info.reduceOperatorList)
      nestReduceAttrs.push_back(
          fir::ReduceAttr::get(builder->getContext(), reduceOp));

    toolchain::SmallVector<mlir::Value> reduceVars;
    language::Compability::lower::omp::ReductionProcessor rp;
    bool result = rp.processReductionArguments<fir::DeclareReductionOp>(
        toLocation(), *this, info.reduceOperatorList, reduceVars,
        reduceVarByRef, reductionDeclSymbols, info.reduceSymList);
    if (!result)
      TODO(toLocation(), "Lowering unrecognised reduction type");

    doConcurrentLoopOp.getReduceVarsMutable().assign(reduceVars);
    doConcurrentLoopOp.setReduceSymsAttr(
        reductionDeclSymbols.empty()
            ? nullptr
            : mlir::ArrayAttr::get(builder->getContext(),
                                   reductionDeclSymbols));
    doConcurrentLoopOp.setReduceAttrsAttr(
        nestReduceAttrs.empty()
            ? nullptr
            : mlir::ArrayAttr::get(builder->getContext(), nestReduceAttrs));
    doConcurrentLoopOp.setReduceByrefAttr(
        reduceVarByRef.empty() ? nullptr
                               : mlir::DenseBoolArrayAttr::get(
                                     builder->getContext(), reduceVarByRef));

    for (auto [sym, reduceVar] :
         toolchain::zip_equal(info.reduceSymList, reduceVars)) {
      auto arg = doConcurrentLoopOp.getRegion().begin()->addArgument(
          reduceVar.getType(), doConcurrentLoopOp.getLoc());
      bindSymbol(*sym, hlfir::translateToExtendedValue(
                           reduceVar.getLoc(), *builder, hlfir::Entity{arg},
                           /*contiguousHint=*/true)
                           .first);
    }

    // Note that allocatable, types with ultimate components, and type
    // requiring finalization are forbidden in LOCAL/LOCAL_INIT (F2023 C1130),
    // so no clean-up needs to be generated for these entities.
  }

  /// Generate FIR for a DO construct. There are six variants:
  ///  - unstructured infinite and while loops
  ///  - structured and unstructured increment loops
  ///  - structured and unstructured concurrent loops
  void genFIR(const language::Compability::parser::DoConstruct &doConstruct) {
    setCurrentPositionAt(doConstruct);
    language::Compability::lower::pft::Evaluation &eval = getEval();
    bool unstructuredContext = eval.lowerAsUnstructured();

    // Loops with induction variables inside OpenACC compute constructs
    // need special handling to ensure that the IVs are privatized.
    if (language::Compability::lower::isInsideOpenACCComputeConstruct(*builder)) {
      mlir::Operation *loopOp = language::Compability::lower::genOpenACCLoopFromDoConstruct(
          *this, bridge.getSemanticsContext(), localSymbols, doConstruct, eval);
      bool success = loopOp != nullptr;
      if (success) {
        // Sanity check that the builder insertion point is inside the newly
        // generated loop.
        assert(
            loopOp->getRegion(0).isAncestor(
                builder->getInsertionPoint()->getBlock()->getParent()) &&
            "builder insertion point is not inside the newly generated loop");

        // Loop body code.
        auto iter = eval.getNestedEvaluations().begin();
        for (auto end = --eval.getNestedEvaluations().end(); iter != end;
             ++iter)
          genFIR(*iter, unstructuredContext);
        return;
      }
      // Fall back to normal loop handling.
    }

    // Collect loop nest information.
    // Generate begin loop code directly for infinite and while loops.
    language::Compability::lower::pft::Evaluation &doStmtEval =
        eval.getFirstNestedEvaluation();
    auto *doStmt = doStmtEval.getIf<language::Compability::parser::NonLabelDoStmt>();
    const auto &loopControl =
        std::get<std::optional<language::Compability::parser::LoopControl>>(doStmt->t);
    mlir::Block *preheaderBlock = doStmtEval.block;
    mlir::Block *beginBlock =
        preheaderBlock ? preheaderBlock : builder->getBlock();
    auto createNextBeginBlock = [&]() {
      // Step beginBlock through unstructured preheader, header, and mask
      // blocks, created in outermost to innermost order.
      return beginBlock = beginBlock->splitBlock(beginBlock->end());
    };
    mlir::Block *headerBlock =
        unstructuredContext ? createNextBeginBlock() : nullptr;
    mlir::Block *bodyBlock = doStmtEval.lexicalSuccessor->block;
    mlir::Block *exitBlock = doStmtEval.parentConstruct->constructExit->block;
    IncrementLoopNestInfo incrementLoopNestInfo;
    const language::Compability::parser::ScalarLogicalExpr *whileCondition = nullptr;
    bool infiniteLoop = !loopControl.has_value();
    if (infiniteLoop) {
      assert(unstructuredContext && "infinite loop must be unstructured");
      startBlock(headerBlock);
    } else if ((whileCondition =
                    std::get_if<language::Compability::parser::ScalarLogicalExpr>(
                        &loopControl->u))) {
      assert(unstructuredContext && "while loop must be unstructured");
      maybeStartBlock(preheaderBlock); // no block or empty block
      startBlock(headerBlock);
      genConditionalBranch(*whileCondition, bodyBlock, exitBlock);
    } else if (const auto *bounds =
                   std::get_if<language::Compability::parser::LoopControl::Bounds>(
                       &loopControl->u)) {
      // Non-concurrent increment loop.
      IncrementLoopInfo &info = incrementLoopNestInfo.emplace_back(
          *bounds->name.thing.symbol, bounds->lower, bounds->upper,
          bounds->step);
      if (unstructuredContext) {
        maybeStartBlock(preheaderBlock);
        info.hasRealControl = info.loopVariableSym->GetType()->IsNumeric(
            language::Compability::common::TypeCategory::Real);
        info.headerBlock = headerBlock;
        info.bodyBlock = bodyBlock;
        info.exitBlock = exitBlock;
      }
    } else {
      const auto *concurrent =
          std::get_if<language::Compability::parser::LoopControl::Concurrent>(
              &loopControl->u);
      assert(concurrent && "invalid DO loop variant");
      incrementLoopNestInfo = getConcurrentControl(
          std::get<language::Compability::parser::ConcurrentHeader>(concurrent->t),
          std::get<std::list<language::Compability::parser::LocalitySpec>>(concurrent->t));
      if (unstructuredContext) {
        maybeStartBlock(preheaderBlock);
        for (IncrementLoopInfo &info : incrementLoopNestInfo) {
          // The original loop body provides the body and latch blocks of the
          // innermost dimension. The (first) body block of a non-innermost
          // dimension is the preheader block of the immediately enclosed
          // dimension. The latch block of a non-innermost dimension is the
          // exit block of the immediately enclosed dimension.
          auto createNextExitBlock = [&]() {
            // Create unstructured loop exit blocks, outermost to innermost.
            return exitBlock = insertBlock(exitBlock);
          };
          bool isInnermost = &info == &incrementLoopNestInfo.back();
          bool isOutermost = &info == &incrementLoopNestInfo.front();
          info.headerBlock = isOutermost ? headerBlock : createNextBeginBlock();
          info.bodyBlock = isInnermost ? bodyBlock : createNextBeginBlock();
          info.exitBlock = isOutermost ? exitBlock : createNextExitBlock();
          if (info.maskExpr)
            info.maskBlock = createNextBeginBlock();
        }
      }
    }

    // Introduce a `do concurrent` scope to bind symbols corresponding to local,
    // local_init, and reduce region arguments.
    if (!incrementLoopNestInfo.empty() &&
        incrementLoopNestInfo.back().isConcurrent)
      localSymbols.pushScope();

    // Increment loop begin code. (Infinite/while code was already generated.)
    if (!infiniteLoop && !whileCondition)
      genFIRIncrementLoopBegin(incrementLoopNestInfo, doStmtEval.dirs);

    // Loop body code.
    auto iter = eval.getNestedEvaluations().begin();
    for (auto end = --eval.getNestedEvaluations().end(); iter != end; ++iter)
      genFIR(*iter, unstructuredContext);

    // An EndDoStmt in unstructured code may start a new block.
    language::Compability::lower::pft::Evaluation &endDoEval = *iter;
    assert(endDoEval.getIf<language::Compability::parser::EndDoStmt>() && "no enddo stmt");
    if (unstructuredContext)
      maybeStartBlock(endDoEval.block);

    // Loop end code.
    if (infiniteLoop || whileCondition)
      genBranch(headerBlock);
    else
      genFIRIncrementLoopEnd(incrementLoopNestInfo);

    // This call may generate a branch in some contexts.
    genFIR(endDoEval, unstructuredContext);

    if (!incrementLoopNestInfo.empty() &&
        incrementLoopNestInfo.back().isConcurrent)
      localSymbols.popScope();
  }

  /// Generate FIR to evaluate loop control values (lower, upper and step).
  mlir::Value genControlValue(const language::Compability::lower::SomeExpr *expr,
                              const IncrementLoopInfo &info,
                              bool *isConst = nullptr) {
    mlir::Location loc = toLocation();
    mlir::Type controlType = info.isStructured() ? builder->getIndexType()
                                                 : info.getLoopVariableType();
    language::Compability::lower::StatementContext stmtCtx;
    if (expr) {
      if (isConst)
        *isConst = language::Compability::evaluate::IsConstantExpr(*expr);
      return builder->createConvert(loc, controlType,
                                    createFIRExpr(loc, expr, stmtCtx));
    }

    if (isConst)
      *isConst = true;
    if (info.hasRealControl)
      return builder->createRealConstant(loc, controlType, 1u);
    return builder->createIntegerConstant(loc, controlType, 1); // step
  }

  // For unroll directives without a value, force full unrolling.
  // For unroll directives with a value, if the value is greater than 1,
  // force unrolling with the given factor. Otherwise, disable unrolling.
  mlir::LLVM::LoopUnrollAttr
  genLoopUnrollAttr(std::optional<std::uint64_t> directiveArg) {
    mlir::BoolAttr falseAttr =
        mlir::BoolAttr::get(builder->getContext(), false);
    mlir::BoolAttr trueAttr = mlir::BoolAttr::get(builder->getContext(), true);
    mlir::IntegerAttr countAttr;
    mlir::BoolAttr fullUnrollAttr;
    bool shouldUnroll = true;
    if (directiveArg.has_value()) {
      auto unrollingFactor = directiveArg.value();
      if (unrollingFactor == 0 || unrollingFactor == 1) {
        shouldUnroll = false;
      } else {
        countAttr =
            builder->getIntegerAttr(builder->getI64Type(), unrollingFactor);
      }
    } else {
      fullUnrollAttr = trueAttr;
    }

    mlir::BoolAttr disableAttr = shouldUnroll ? falseAttr : trueAttr;
    return mlir::LLVM::LoopUnrollAttr::get(
        builder->getContext(), /*disable=*/disableAttr, /*count=*/countAttr, {},
        /*full=*/fullUnrollAttr, {}, {}, {});
  }

  // Enabling unroll and jamming directive without a value.
  // For directives with a value, if the value is greater than 1,
  // force unrolling with the given factor. Otherwise, disable unrolling and
  // jamming.
  mlir::LLVM::LoopUnrollAndJamAttr
  genLoopUnrollAndJamAttr(std::optional<std::uint64_t> count) {
    mlir::BoolAttr falseAttr =
        mlir::BoolAttr::get(builder->getContext(), false);
    mlir::BoolAttr trueAttr = mlir::BoolAttr::get(builder->getContext(), true);
    mlir::IntegerAttr countAttr;
    bool shouldUnroll = true;
    if (count.has_value()) {
      auto unrollingFactor = count.value();
      if (unrollingFactor == 0 || unrollingFactor == 1) {
        shouldUnroll = false;
      } else {
        countAttr =
            builder->getIntegerAttr(builder->getI64Type(), unrollingFactor);
      }
    }

    mlir::BoolAttr disableAttr = shouldUnroll ? falseAttr : trueAttr;
    return mlir::LLVM::LoopUnrollAndJamAttr::get(
        builder->getContext(), /*disable=*/disableAttr, /*count*/ countAttr, {},
        {}, {}, {}, {});
  }

  void addLoopAnnotationAttr(
      IncrementLoopInfo &info,
      toolchain::SmallVectorImpl<const language::Compability::parser::CompilerDirective *> &dirs) {
    mlir::LLVM::LoopVectorizeAttr va;
    mlir::LLVM::LoopUnrollAttr ua;
    mlir::LLVM::LoopUnrollAndJamAttr uja;
    bool has_attrs = false;
    for (const auto *dir : dirs) {
      language::Compability::common::visit(
          language::Compability::common::visitors{
              [&](const language::Compability::parser::CompilerDirective::VectorAlways &) {
                mlir::BoolAttr falseAttr =
                    mlir::BoolAttr::get(builder->getContext(), false);
                va = mlir::LLVM::LoopVectorizeAttr::get(builder->getContext(),
                                                        /*disable=*/falseAttr,
                                                        {}, {}, {}, {}, {}, {});
                has_attrs = true;
              },
              [&](const language::Compability::parser::CompilerDirective::Unroll &u) {
                ua = genLoopUnrollAttr(u.v);
                has_attrs = true;
              },
              [&](const language::Compability::parser::CompilerDirective::UnrollAndJam &u) {
                uja = genLoopUnrollAndJamAttr(u.v);
                has_attrs = true;
              },
              [&](const language::Compability::parser::CompilerDirective::NoVector &u) {
                mlir::BoolAttr trueAttr =
                    mlir::BoolAttr::get(builder->getContext(), true);
                va = mlir::LLVM::LoopVectorizeAttr::get(builder->getContext(),
                                                        /*disable=*/trueAttr,
                                                        {}, {}, {}, {}, {}, {});
                has_attrs = true;
              },
              [&](const language::Compability::parser::CompilerDirective::NoUnroll &u) {
                ua = genLoopUnrollAttr(/*unrollingFactor=*/0);
                has_attrs = true;
              },
              [&](const language::Compability::parser::CompilerDirective::NoUnrollAndJam &u) {
                uja = genLoopUnrollAndJamAttr(/*unrollingFactor=*/0);
                has_attrs = true;
              },

              [&](const auto &) {}},
          dir->u);
    }
    mlir::LLVM::LoopAnnotationAttr la = mlir::LLVM::LoopAnnotationAttr::get(
        builder->getContext(), {}, /*vectorize=*/va, {}, /*unroll*/ ua,
        /*unroll_and_jam*/ uja, {}, {}, {}, {}, {}, {}, {}, {}, {}, {});
    if (has_attrs) {
      if (auto loopOp = mlir::dyn_cast<fir::DoLoopOp>(info.loopOp))
        loopOp.setLoopAnnotationAttr(la);

      if (auto doConcurrentOp =
              mlir::dyn_cast<fir::DoConcurrentLoopOp>(info.loopOp))
        doConcurrentOp.setLoopAnnotationAttr(la);
    }
  }

  /// Generate FIR to begin a structured or unstructured increment loop nest.
  void genFIRIncrementLoopBegin(
      IncrementLoopNestInfo &incrementLoopNestInfo,
      toolchain::SmallVectorImpl<const language::Compability::parser::CompilerDirective *> &dirs) {
    assert(!incrementLoopNestInfo.empty() && "empty loop nest");
    mlir::Location loc = toLocation();
    mlir::arith::IntegerOverflowFlags iofBackup{};

    toolchain::SmallVector<mlir::Value> nestLBs;
    toolchain::SmallVector<mlir::Value> nestUBs;
    toolchain::SmallVector<mlir::Value> nestSts;
    toolchain::SmallVector<mlir::Value> nestReduceOperands;
    toolchain::SmallVector<mlir::Attribute> nestReduceAttrs;
    bool genDoConcurrent = false;

    for (IncrementLoopInfo &info : incrementLoopNestInfo) {
      genDoConcurrent = info.isStructured() && info.isConcurrent;

      if (!genDoConcurrent)
        info.loopVariable = genLoopVariableAddress(loc, *info.loopVariableSym,
                                                   info.isConcurrent);

      if (!getLoweringOptions().getIntegerWrapAround()) {
        iofBackup = builder->getIntegerOverflowFlags();
        builder->setIntegerOverflowFlags(
            mlir::arith::IntegerOverflowFlags::nsw);
      }

      nestLBs.push_back(genControlValue(info.lowerExpr, info));
      nestUBs.push_back(genControlValue(info.upperExpr, info));
      bool isConst = true;
      nestSts.push_back(genControlValue(
          info.stepExpr, info, info.isStructured() ? nullptr : &isConst));

      if (!getLoweringOptions().getIntegerWrapAround())
        builder->setIntegerOverflowFlags(iofBackup);

      // Use a temp variable for unstructured loops with non-const step.
      if (!isConst) {
        mlir::Value stepValue = nestSts.back();
        info.stepVariable = builder->createTemporary(loc, stepValue.getType());
        fir::StoreOp::create(*builder, loc, stepValue, info.stepVariable);
      }
    }

    for (auto [info, lowerValue, upperValue, stepValue] :
         toolchain::zip_equal(incrementLoopNestInfo, nestLBs, nestUBs, nestSts)) {
      // Structured loop - generate fir.do_loop.
      if (info.isStructured()) {
        if (genDoConcurrent)
          continue;

        // The loop variable is a doLoop op argument.
        mlir::Type loopVarType = info.getLoopVariableType();
        auto loopOp = fir::DoLoopOp::create(
            *builder, loc, lowerValue, upperValue, stepValue,
            /*unordered=*/false,
            /*finalCountValue=*/true,
            builder->createConvert(loc, loopVarType, lowerValue));
        info.loopOp = loopOp;
        builder->setInsertionPointToStart(loopOp.getBody());
        mlir::Value loopValue = loopOp.getRegionIterArgs()[0];

        // Update the loop variable value in case it has non-index references.
        fir::StoreOp::create(*builder, loc, loopValue, info.loopVariable);
        addLoopAnnotationAttr(info, dirs);
        continue;
      }

      // Unstructured loop preheader - initialize tripVariable and loopVariable.
      mlir::Value tripCount;
      if (info.hasRealControl) {
        auto diff1 =
            mlir::arith::SubFOp::create(*builder, loc, upperValue, lowerValue);
        auto diff2 =
            mlir::arith::AddFOp::create(*builder, loc, diff1, stepValue);
        tripCount =
            mlir::arith::DivFOp::create(*builder, loc, diff2, stepValue);
        tripCount =
            builder->createConvert(loc, builder->getIndexType(), tripCount);
      } else {
        auto diff1 =
            mlir::arith::SubIOp::create(*builder, loc, upperValue, lowerValue);
        auto diff2 =
            mlir::arith::AddIOp::create(*builder, loc, diff1, stepValue);
        tripCount =
            mlir::arith::DivSIOp::create(*builder, loc, diff2, stepValue);
      }
      if (forceLoopToExecuteOnce) { // minimum tripCount is 1
        mlir::Value one =
            builder->createIntegerConstant(loc, tripCount.getType(), 1);
        auto cond = mlir::arith::CmpIOp::create(
            *builder, loc, mlir::arith::CmpIPredicate::slt, tripCount, one);
        tripCount =
            mlir::arith::SelectOp::create(*builder, loc, cond, one, tripCount);
      }
      info.tripVariable = builder->createTemporary(loc, tripCount.getType());
      fir::StoreOp::create(*builder, loc, tripCount, info.tripVariable);
      fir::StoreOp::create(*builder, loc, lowerValue, info.loopVariable);

      // Unstructured loop header - generate loop condition and mask.
      // Note - Currently there is no way to tag a loop as a concurrent loop.
      startBlock(info.headerBlock);
      tripCount = fir::LoadOp::create(*builder, loc, info.tripVariable);
      mlir::Value zero =
          builder->createIntegerConstant(loc, tripCount.getType(), 0);
      auto cond = mlir::arith::CmpIOp::create(
          *builder, loc, mlir::arith::CmpIPredicate::sgt, tripCount, zero);
      if (info.maskExpr) {
        genConditionalBranch(cond, info.maskBlock, info.exitBlock);
        startBlock(info.maskBlock);
        mlir::Block *latchBlock = getEval().getLastNestedEvaluation().block;
        assert(latchBlock && "missing masked concurrent loop latch block");
        language::Compability::lower::StatementContext stmtCtx;
        mlir::Value maskCond = createFIRExpr(loc, info.maskExpr, stmtCtx);
        stmtCtx.finalizeAndReset();
        genConditionalBranch(maskCond, info.bodyBlock, latchBlock);
      } else {
        genConditionalBranch(cond, info.bodyBlock, info.exitBlock);
        if (&info != &incrementLoopNestInfo.back()) // not innermost
          startBlock(info.bodyBlock); // preheader block of enclosed dimension
      }
      if (info.hasLocalitySpecs()) {
        mlir::OpBuilder::InsertPoint insertPt = builder->saveInsertionPoint();
        builder->setInsertionPointToStart(info.bodyBlock);
        handleLocalitySpecs(info);
        builder->restoreInsertionPoint(insertPt);
      }
    }

    if (genDoConcurrent) {
      auto loopWrapperOp = fir::DoConcurrentOp::create(*builder, loc);
      builder->setInsertionPointToStart(
          builder->createBlock(&loopWrapperOp.getRegion()));

      for (IncrementLoopInfo &info : toolchain::reverse(incrementLoopNestInfo)) {
        info.loopVariable = genLoopVariableAddress(loc, *info.loopVariableSym,
                                                   info.isConcurrent);
      }

      builder->setInsertionPointToEnd(loopWrapperOp.getBody());
      auto loopOp = fir::DoConcurrentLoopOp::create(
          *builder, loc, nestLBs, nestUBs, nestSts, /*loopAnnotation=*/nullptr,
          /*local_vars=*/mlir::ValueRange{},
          /*local_syms=*/nullptr, /*reduce_vars=*/mlir::ValueRange{},
          /*reduce_byref=*/nullptr, /*reduce_syms=*/nullptr,
          /*reduce_attrs=*/nullptr);

      toolchain::SmallVector<mlir::Type> loopBlockArgTypes(
          incrementLoopNestInfo.size(), builder->getIndexType());
      toolchain::SmallVector<mlir::Location> loopBlockArgLocs(
          incrementLoopNestInfo.size(), loc);
      mlir::Region &loopRegion = loopOp.getRegion();
      mlir::Block *loopBlock = builder->createBlock(
          &loopRegion, loopRegion.begin(), loopBlockArgTypes, loopBlockArgLocs);
      builder->setInsertionPointToStart(loopBlock);

      for (auto [info, blockArg] :
           toolchain::zip_equal(incrementLoopNestInfo, loopBlock->getArguments())) {
        info.loopOp = loopOp;
        mlir::Value loopValue =
            builder->createConvert(loc, info.getLoopVariableType(), blockArg);
        fir::StoreOp::create(*builder, loc, loopValue, info.loopVariable);

        if (info.maskExpr) {
          language::Compability::lower::StatementContext stmtCtx;
          mlir::Value maskCond = createFIRExpr(loc, info.maskExpr, stmtCtx);
          stmtCtx.finalizeAndReset();
          mlir::Value maskCondCast =
              builder->createConvert(loc, builder->getI1Type(), maskCond);
          auto ifOp = fir::IfOp::create(*builder, loc, maskCondCast,
                                        /*withElseRegion=*/false);
          builder->setInsertionPointToStart(&ifOp.getThenRegion().front());
        }
      }

      IncrementLoopInfo &innermostInfo = incrementLoopNestInfo.back();

      if (innermostInfo.hasLocalitySpecs())
        handleLocalitySpecs(innermostInfo);

      addLoopAnnotationAttr(innermostInfo, dirs);
    }
  }

  /// Generate FIR to end a structured or unstructured increment loop nest.
  void genFIRIncrementLoopEnd(IncrementLoopNestInfo &incrementLoopNestInfo) {
    assert(!incrementLoopNestInfo.empty() && "empty loop nest");
    mlir::Location loc = toLocation();
    mlir::arith::IntegerOverflowFlags flags{};
    if (!getLoweringOptions().getIntegerWrapAround())
      flags = bitEnumSet(flags, mlir::arith::IntegerOverflowFlags::nsw);
    auto iofAttr = mlir::arith::IntegerOverflowFlagsAttr::get(
        builder->getContext(), flags);
    for (auto it = incrementLoopNestInfo.rbegin(),
              rend = incrementLoopNestInfo.rend();
         it != rend; ++it) {
      IncrementLoopInfo &info = *it;
      if (info.isStructured()) {
        // End fir.do_concurent.loop.
        if (info.isConcurrent) {
          builder->setInsertionPointAfter(info.loopOp->getParentOp());
          continue;
        }

        // End fir.do_loop.
        // Decrement tripVariable.
        auto doLoopOp = mlir::cast<fir::DoLoopOp>(info.loopOp);
        builder->setInsertionPointToEnd(doLoopOp.getBody());
        toolchain::SmallVector<mlir::Value, 2> results;
        results.push_back(mlir::arith::AddIOp::create(
            *builder, loc, doLoopOp.getInductionVar(), doLoopOp.getStep(),
            iofAttr));
        // Step loopVariable to help optimizations such as vectorization.
        // Induction variable elimination will clean up as necessary.
        mlir::Value step = builder->createConvert(
            loc, info.getLoopVariableType(), doLoopOp.getStep());
        mlir::Value loopVar =
            fir::LoadOp::create(*builder, loc, info.loopVariable);
        results.push_back(
            mlir::arith::AddIOp::create(*builder, loc, loopVar, step, iofAttr));
        fir::ResultOp::create(*builder, loc, results);
        builder->setInsertionPointAfter(doLoopOp);
        // The loop control variable may be used after the loop.
        fir::StoreOp::create(*builder, loc, doLoopOp.getResult(1),
                             info.loopVariable);
        continue;
      }

      // Unstructured loop - decrement tripVariable and step loopVariable.
      mlir::Value tripCount =
          fir::LoadOp::create(*builder, loc, info.tripVariable);
      mlir::Value one =
          builder->createIntegerConstant(loc, tripCount.getType(), 1);
      tripCount = mlir::arith::SubIOp::create(*builder, loc, tripCount, one);
      fir::StoreOp::create(*builder, loc, tripCount, info.tripVariable);
      mlir::Value value = fir::LoadOp::create(*builder, loc, info.loopVariable);
      mlir::Value step;
      if (info.stepVariable)
        step = fir::LoadOp::create(*builder, loc, info.stepVariable);
      else
        step = genControlValue(info.stepExpr, info);
      if (info.hasRealControl)
        value = mlir::arith::AddFOp::create(*builder, loc, value, step);
      else
        value =
            mlir::arith::AddIOp::create(*builder, loc, value, step, iofAttr);
      fir::StoreOp::create(*builder, loc, value, info.loopVariable);

      genBranch(info.headerBlock);
      if (&info != &incrementLoopNestInfo.front()) // not outermost
        startBlock(info.exitBlock); // latch block of enclosing dimension
    }
  }

  /// Generate structured or unstructured FIR for an IF construct.
  /// The initial statement may be either an IfStmt or an IfThenStmt.
  void genFIR(const language::Compability::parser::IfConstruct &) {
    language::Compability::lower::pft::Evaluation &eval = getEval();

    // Structured fir.if nest.
    if (eval.lowerAsStructured()) {
      fir::IfOp topIfOp, currentIfOp;
      for (language::Compability::lower::pft::Evaluation &e : eval.getNestedEvaluations()) {
        auto genIfOp = [&](mlir::Value cond) {
          language::Compability::lower::pft::Evaluation &succ = *e.controlSuccessor;
          bool hasElse = succ.isA<language::Compability::parser::ElseIfStmt>() ||
                         succ.isA<language::Compability::parser::ElseStmt>();
          auto ifOp = fir::IfOp::create(*builder, toLocation(), cond,
                                        /*withElseRegion=*/hasElse);
          builder->setInsertionPointToStart(&ifOp.getThenRegion().front());
          return ifOp;
        };
        setCurrentPosition(e.position);
        if (auto *s = e.getIf<language::Compability::parser::IfThenStmt>()) {
          topIfOp = currentIfOp = genIfOp(genIfCondition(s, e.negateCondition));
        } else if (auto *s = e.getIf<language::Compability::parser::IfStmt>()) {
          topIfOp = currentIfOp = genIfOp(genIfCondition(s, e.negateCondition));
        } else if (auto *s = e.getIf<language::Compability::parser::ElseIfStmt>()) {
          builder->setInsertionPointToStart(
              &currentIfOp.getElseRegion().front());
          currentIfOp = genIfOp(genIfCondition(s));
        } else if (e.isA<language::Compability::parser::ElseStmt>()) {
          builder->setInsertionPointToStart(
              &currentIfOp.getElseRegion().front());
        } else if (e.isA<language::Compability::parser::EndIfStmt>()) {
          builder->setInsertionPointAfter(topIfOp);
          genFIR(e, /*unstructuredContext=*/false); // may generate branch
        } else {
          genFIR(e, /*unstructuredContext=*/false);
        }
      }
      return;
    }

    // Unstructured branch sequence.
    toolchain::SmallVector<language::Compability::lower::pft::Evaluation *> exits, fallThroughs;
    collectFinalEvaluations(eval, exits, fallThroughs);

    for (language::Compability::lower::pft::Evaluation &e : eval.getNestedEvaluations()) {
      auto genIfBranch = [&](mlir::Value cond) {
        if (e.lexicalSuccessor == e.controlSuccessor) // empty block -> exit
          genConditionalBranch(cond, e.parentConstruct->constructExit,
                               e.controlSuccessor);
        else // non-empty block
          genConditionalBranch(cond, e.lexicalSuccessor, e.controlSuccessor);
      };
      setCurrentPosition(e.position);
      if (auto *s = e.getIf<language::Compability::parser::IfThenStmt>()) {
        maybeStartBlock(e.block);
        genIfBranch(genIfCondition(s, e.negateCondition));
      } else if (auto *s = e.getIf<language::Compability::parser::IfStmt>()) {
        maybeStartBlock(e.block);
        genIfBranch(genIfCondition(s, e.negateCondition));
      } else if (auto *s = e.getIf<language::Compability::parser::ElseIfStmt>()) {
        startBlock(e.block);
        genIfBranch(genIfCondition(s));
      } else {
        genFIR(e);
        if (blockIsUnterminated()) {
          if (toolchain::is_contained(exits, &e))
            genConstructExitBranch(*eval.constructExit);
          else if (toolchain::is_contained(fallThroughs, &e))
            genBranch(e.lexicalSuccessor->block);
        }
      }
    }
  }

  void genCaseOrRankConstruct() {
    language::Compability::lower::pft::Evaluation &eval = getEval();
    language::Compability::lower::StatementContext stmtCtx;
    pushActiveConstruct(eval, stmtCtx);

    toolchain::SmallVector<language::Compability::lower::pft::Evaluation *> exits, fallThroughs;
    collectFinalEvaluations(eval, exits, fallThroughs);

    for (language::Compability::lower::pft::Evaluation &e : eval.getNestedEvaluations()) {
      if (e.getIf<language::Compability::parser::EndSelectStmt>())
        maybeStartBlock(e.block);
      else
        genFIR(e);
      if (blockIsUnterminated()) {
        if (toolchain::is_contained(exits, &e))
          genConstructExitBranch(*eval.constructExit);
        else if (toolchain::is_contained(fallThroughs, &e))
          genBranch(e.lexicalSuccessor->block);
      }
    }
    popActiveConstruct();
  }
  void genFIR(const language::Compability::parser::CaseConstruct &) {
    genCaseOrRankConstruct();
  }

  template <typename A>
  void genNestedStatement(const language::Compability::parser::Statement<A> &stmt) {
    setCurrentPosition(stmt.source);
    genFIR(stmt.statement);
  }

  /// Force the binding of an explicit symbol. This is used to bind and re-bind
  /// a concurrent control symbol to its value.
  void forceControlVariableBinding(const language::Compability::semantics::Symbol *sym,
                                   mlir::Value inducVar) {
    mlir::Location loc = toLocation();
    assert(sym && "There must be a symbol to bind");
    mlir::Type toTy = genType(*sym);
    // FIXME: this should be a "per iteration" temporary.
    mlir::Value tmp =
        builder->createTemporary(loc, toTy, toStringRef(sym->name()),
                                 toolchain::ArrayRef<mlir::NamedAttribute>{
                                     fir::getAdaptToByRefAttr(*builder)});
    mlir::Value cast = builder->createConvert(loc, toTy, inducVar);
    fir::StoreOp::create(*builder, loc, cast, tmp);
    addSymbol(*sym, tmp, /*force=*/true);
  }

  /// Process a concurrent header for a FORALL. (Concurrent headers for DO
  /// CONCURRENT loops are lowered elsewhere.)
  void genFIR(const language::Compability::parser::ConcurrentHeader &header) {
    toolchain::SmallVector<mlir::Value> lows;
    toolchain::SmallVector<mlir::Value> highs;
    toolchain::SmallVector<mlir::Value> steps;
    if (explicitIterSpace.isOutermostForall()) {
      // For the outermost forall, we evaluate the bounds expressions once.
      // Contrastingly, if this forall is nested, the bounds expressions are
      // assumed to be pure, possibly dependent on outer concurrent control
      // variables, possibly variant with respect to arguments, and will be
      // re-evaluated.
      mlir::Location loc = toLocation();
      mlir::Type idxTy = builder->getIndexType();
      language::Compability::lower::StatementContext &stmtCtx =
          explicitIterSpace.stmtContext();
      auto lowerExpr = [&](auto &e) {
        return fir::getBase(genExprValue(e, stmtCtx));
      };
      for (const language::Compability::parser::ConcurrentControl &ctrl :
           std::get<std::list<language::Compability::parser::ConcurrentControl>>(header.t)) {
        const language::Compability::lower::SomeExpr *lo =
            language::Compability::semantics::GetExpr(std::get<1>(ctrl.t));
        const language::Compability::lower::SomeExpr *hi =
            language::Compability::semantics::GetExpr(std::get<2>(ctrl.t));
        auto &optStep =
            std::get<std::optional<language::Compability::parser::ScalarIntExpr>>(ctrl.t);
        lows.push_back(builder->createConvert(loc, idxTy, lowerExpr(*lo)));
        highs.push_back(builder->createConvert(loc, idxTy, lowerExpr(*hi)));
        steps.push_back(
            optStep.has_value()
                ? builder->createConvert(
                      loc, idxTy,
                      lowerExpr(*language::Compability::semantics::GetExpr(*optStep)))
                : builder->createIntegerConstant(loc, idxTy, 1));
      }
    }
    auto lambda = [&, lows, highs, steps]() {
      // Create our iteration space from the header spec.
      mlir::Location loc = toLocation();
      mlir::Type idxTy = builder->getIndexType();
      toolchain::SmallVector<fir::DoLoopOp> loops;
      language::Compability::lower::StatementContext &stmtCtx =
          explicitIterSpace.stmtContext();
      auto lowerExpr = [&](auto &e) {
        return fir::getBase(genExprValue(e, stmtCtx));
      };
      const bool outermost = !lows.empty();
      std::size_t headerIndex = 0;
      for (const language::Compability::parser::ConcurrentControl &ctrl :
           std::get<std::list<language::Compability::parser::ConcurrentControl>>(header.t)) {
        const language::Compability::semantics::Symbol *ctrlVar =
            std::get<language::Compability::parser::Name>(ctrl.t).symbol;
        mlir::Value lb;
        mlir::Value ub;
        mlir::Value by;
        if (outermost) {
          assert(headerIndex < lows.size());
          if (headerIndex == 0)
            explicitIterSpace.resetInnerArgs();
          lb = lows[headerIndex];
          ub = highs[headerIndex];
          by = steps[headerIndex++];
        } else {
          const language::Compability::lower::SomeExpr *lo =
              language::Compability::semantics::GetExpr(std::get<1>(ctrl.t));
          const language::Compability::lower::SomeExpr *hi =
              language::Compability::semantics::GetExpr(std::get<2>(ctrl.t));
          auto &optStep =
              std::get<std::optional<language::Compability::parser::ScalarIntExpr>>(ctrl.t);
          lb = builder->createConvert(loc, idxTy, lowerExpr(*lo));
          ub = builder->createConvert(loc, idxTy, lowerExpr(*hi));
          by = optStep.has_value()
                   ? builder->createConvert(
                         loc, idxTy,
                         lowerExpr(*language::Compability::semantics::GetExpr(*optStep)))
                   : builder->createIntegerConstant(loc, idxTy, 1);
        }
        auto lp = fir::DoLoopOp::create(
            *builder, loc, lb, ub, by, /*unordered=*/true,
            /*finalCount=*/false, explicitIterSpace.getInnerArgs());
        if ((!loops.empty() || !outermost) && !lp.getRegionIterArgs().empty())
          fir::ResultOp::create(*builder, loc, lp.getResults());
        explicitIterSpace.setInnerArgs(lp.getRegionIterArgs());
        builder->setInsertionPointToStart(lp.getBody());
        forceControlVariableBinding(ctrlVar, lp.getInductionVar());
        loops.push_back(lp);
      }
      if (outermost)
        explicitIterSpace.setOuterLoop(loops[0]);
      explicitIterSpace.appendLoops(loops);
      if (const auto &mask =
              std::get<std::optional<language::Compability::parser::ScalarLogicalExpr>>(
                  header.t);
          mask.has_value()) {
        mlir::Type i1Ty = builder->getI1Type();
        fir::ExtendedValue maskExv =
            genExprValue(*language::Compability::semantics::GetExpr(mask.value()), stmtCtx);
        mlir::Value cond =
            builder->createConvert(loc, i1Ty, fir::getBase(maskExv));
        auto ifOp = fir::IfOp::create(*builder, loc,
                                      explicitIterSpace.innerArgTypes(), cond,
                                      /*withElseRegion=*/true);
        fir::ResultOp::create(*builder, loc, ifOp.getResults());
        builder->setInsertionPointToStart(&ifOp.getElseRegion().front());
        fir::ResultOp::create(*builder, loc, explicitIterSpace.getInnerArgs());
        builder->setInsertionPointToStart(&ifOp.getThenRegion().front());
      }
    };
    // Push the lambda to gen the loop nest context.
    explicitIterSpace.pushLoopNest(lambda);
  }

  void genFIR(const language::Compability::parser::ForallAssignmentStmt &stmt) {
    language::Compability::common::visit([&](const auto &x) { genFIR(x); }, stmt.u);
  }

  void genFIR(const language::Compability::parser::EndForallStmt &) {
    if (!lowerToHighLevelFIR())
      cleanupExplicitSpace();
  }

  template <typename A>
  void prepareExplicitSpace(const A &forall) {
    if (!explicitIterSpace.isActive())
      analyzeExplicitSpace(forall);
    localSymbols.pushScope();
    explicitIterSpace.enter();
  }

  /// Cleanup all the FORALL context information when we exit.
  void cleanupExplicitSpace() {
    explicitIterSpace.leave();
    localSymbols.popScope();
  }

  /// Generate FIR for a FORALL statement.
  void genFIR(const language::Compability::parser::ForallStmt &stmt) {
    const auto &concurrentHeader =
        std::get<
            language::Compability::common::Indirection<language::Compability::parser::ConcurrentHeader>>(
            stmt.t)
            .value();
    if (lowerToHighLevelFIR()) {
      mlir::OpBuilder::InsertionGuard guard(*builder);
      language::Compability::lower::SymMapScope scope(localSymbols);
      genForallNest(concurrentHeader);
      genFIR(std::get<language::Compability::parser::UnlabeledStatement<
                 language::Compability::parser::ForallAssignmentStmt>>(stmt.t)
                 .statement);
      return;
    }
    prepareExplicitSpace(stmt);
    genFIR(concurrentHeader);
    genFIR(std::get<language::Compability::parser::UnlabeledStatement<
               language::Compability::parser::ForallAssignmentStmt>>(stmt.t)
               .statement);
    cleanupExplicitSpace();
  }

  /// Generate FIR for a FORALL construct.
  void genFIR(const language::Compability::parser::ForallConstruct &forall) {
    mlir::OpBuilder::InsertPoint insertPt = builder->saveInsertionPoint();
    if (lowerToHighLevelFIR())
      localSymbols.pushScope();
    else
      prepareExplicitSpace(forall);
    genNestedStatement(
        std::get<
            language::Compability::parser::Statement<language::Compability::parser::ForallConstructStmt>>(
            forall.t));
    for (const language::Compability::parser::ForallBodyConstruct &s :
         std::get<std::list<language::Compability::parser::ForallBodyConstruct>>(forall.t)) {
      language::Compability::common::visit(
          language::Compability::common::visitors{
              [&](const language::Compability::parser::WhereConstruct &b) { genFIR(b); },
              [&](const language::Compability::common::Indirection<
                  language::Compability::parser::ForallConstruct> &b) { genFIR(b.value()); },
              [&](const auto &b) { genNestedStatement(b); }},
          s.u);
    }
    genNestedStatement(
        std::get<language::Compability::parser::Statement<language::Compability::parser::EndForallStmt>>(
            forall.t));
    if (lowerToHighLevelFIR()) {
      localSymbols.popScope();
      builder->restoreInsertionPoint(insertPt);
    }
  }

  /// Lower the concurrent header specification.
  void genFIR(const language::Compability::parser::ForallConstructStmt &stmt) {
    const auto &concurrentHeader =
        std::get<
            language::Compability::common::Indirection<language::Compability::parser::ConcurrentHeader>>(
            stmt.t)
            .value();
    if (lowerToHighLevelFIR())
      genForallNest(concurrentHeader);
    else
      genFIR(concurrentHeader);
  }

  /// Generate hlfir.forall and hlfir.forall_mask nest given a Forall
  /// concurrent header
  void genForallNest(const language::Compability::parser::ConcurrentHeader &header) {
    mlir::Location loc = getCurrentLocation();
    const bool isOutterForall = !isInsideHlfirForallOrWhere();
    hlfir::ForallOp outerForall;
    auto evaluateControl = [&](const auto &parserExpr, mlir::Region &region,
                               bool isMask = false) {
      if (region.empty())
        builder->createBlock(&region);
      language::Compability::lower::StatementContext localStmtCtx;
      const language::Compability::semantics::SomeExpr *anlalyzedExpr =
          language::Compability::semantics::GetExpr(parserExpr);
      assert(anlalyzedExpr && "expression semantics failed");
      // Generate the controls of outer forall outside of the hlfir.forall
      // region. They do not depend on any previous forall indices (C1123) and
      // no assignment has been made yet that could modify their value. This
      // will simplify hlfir.forall analysis because the SSA integer value
      // yielded will obviously not depend on any variable modified by the
      // forall when produced outside of it.
      // This is not done for the mask because it may (and in usual code, does)
      // depend on the forall indices that have just been defined as
      // hlfir.forall block arguments.
      mlir::OpBuilder::InsertPoint innerInsertionPoint;
      if (outerForall && !isMask) {
        innerInsertionPoint = builder->saveInsertionPoint();
        builder->setInsertionPoint(outerForall);
      }
      mlir::Value exprVal =
          fir::getBase(genExprValue(*anlalyzedExpr, localStmtCtx, &loc));
      localStmtCtx.finalizeAndPop();
      if (isMask)
        exprVal = builder->createConvert(loc, builder->getI1Type(), exprVal);
      if (innerInsertionPoint.isSet())
        builder->restoreInsertionPoint(innerInsertionPoint);
      hlfir::YieldOp::create(*builder, loc, exprVal);
    };
    for (const language::Compability::parser::ConcurrentControl &control :
         std::get<std::list<language::Compability::parser::ConcurrentControl>>(header.t)) {
      auto forallOp = hlfir::ForallOp::create(*builder, loc);
      if (isOutterForall && !outerForall)
        outerForall = forallOp;
      evaluateControl(std::get<1>(control.t), forallOp.getLbRegion());
      evaluateControl(std::get<2>(control.t), forallOp.getUbRegion());
      if (const auto &optionalStep =
              std::get<std::optional<language::Compability::parser::ScalarIntExpr>>(
                  control.t))
        evaluateControl(*optionalStep, forallOp.getStepRegion());
      // Create block argument and map it to a symbol via an hlfir.forall_index
      // op (symbols must be mapped to in memory values).
      const language::Compability::semantics::Symbol *controlVar =
          std::get<language::Compability::parser::Name>(control.t).symbol;
      assert(controlVar && "symbol analysis failed");
      mlir::Type controlVarType = genType(*controlVar);
      mlir::Block *forallBody = builder->createBlock(&forallOp.getBody(), {},
                                                     {controlVarType}, {loc});
      auto forallIndex = hlfir::ForallIndexOp::create(
          *builder, loc, fir::ReferenceType::get(controlVarType),
          forallBody->getArguments()[0],
          builder->getStringAttr(controlVar->name().ToString()));
      localSymbols.addVariableDefinition(*controlVar, forallIndex,
                                         /*force=*/true);
      auto end = fir::FirEndOp::create(*builder, loc);
      builder->setInsertionPoint(end);
    }

    if (const auto &maskExpr =
            std::get<std::optional<language::Compability::parser::ScalarLogicalExpr>>(
                header.t)) {
      // Create hlfir.forall_mask and set insertion point in its body.
      auto forallMaskOp = hlfir::ForallMaskOp::create(*builder, loc);
      evaluateControl(*maskExpr, forallMaskOp.getMaskRegion(), /*isMask=*/true);
      builder->createBlock(&forallMaskOp.getBody());
      auto end = fir::FirEndOp::create(*builder, loc);
      builder->setInsertionPoint(end);
    }
  }

  void attachDirectiveToLoop(const language::Compability::parser::CompilerDirective &dir,
                             language::Compability::lower::pft::Evaluation *e) {
    while (e->isDirective())
      e = e->lexicalSuccessor;

    if (e->isA<language::Compability::parser::NonLabelDoStmt>())
      e->dirs.push_back(&dir);
  }

  void genFIR(const language::Compability::parser::CompilerDirective &dir) {
    language::Compability::lower::pft::Evaluation &eval = getEval();

    language::Compability::common::visit(
        language::Compability::common::visitors{
            [&](const language::Compability::parser::CompilerDirective::VectorAlways &) {
              attachDirectiveToLoop(dir, &eval);
            },
            [&](const language::Compability::parser::CompilerDirective::Unroll &) {
              attachDirectiveToLoop(dir, &eval);
            },
            [&](const language::Compability::parser::CompilerDirective::UnrollAndJam &) {
              attachDirectiveToLoop(dir, &eval);
            },
            [&](const language::Compability::parser::CompilerDirective::NoVector &) {
              attachDirectiveToLoop(dir, &eval);
            },
            [&](const language::Compability::parser::CompilerDirective::NoUnroll &) {
              attachDirectiveToLoop(dir, &eval);
            },
            [&](const language::Compability::parser::CompilerDirective::NoUnrollAndJam &) {
              attachDirectiveToLoop(dir, &eval);
            },
            [&](const auto &) {}},
        dir.u);
  }

  void genFIR(const language::Compability::parser::OpenACCConstruct &acc) {
    mlir::OpBuilder::InsertPoint insertPt = builder->saveInsertionPoint();
    localSymbols.pushScope();
    mlir::Value exitCond = genOpenACCConstruct(
        *this, bridge.getSemanticsContext(), getEval(), acc);

    const language::Compability::parser::OpenACCLoopConstruct *accLoop =
        std::get_if<language::Compability::parser::OpenACCLoopConstruct>(&acc.u);
    const language::Compability::parser::OpenACCCombinedConstruct *accCombined =
        std::get_if<language::Compability::parser::OpenACCCombinedConstruct>(&acc.u);

    language::Compability::lower::pft::Evaluation *curEval = &getEval();

    if (accLoop || accCombined) {
      uint64_t loopCount;
      if (accLoop) {
        const language::Compability::parser::AccBeginLoopDirective &beginLoopDir =
            std::get<language::Compability::parser::AccBeginLoopDirective>(accLoop->t);
        const language::Compability::parser::AccClauseList &clauseList =
            std::get<language::Compability::parser::AccClauseList>(beginLoopDir.t);
        loopCount = language::Compability::lower::getLoopCountForCollapseAndTile(clauseList);
      } else if (accCombined) {
        const language::Compability::parser::AccBeginCombinedDirective &beginCombinedDir =
            std::get<language::Compability::parser::AccBeginCombinedDirective>(
                accCombined->t);
        const language::Compability::parser::AccClauseList &clauseList =
            std::get<language::Compability::parser::AccClauseList>(beginCombinedDir.t);
        loopCount = language::Compability::lower::getLoopCountForCollapseAndTile(clauseList);
      }

      if (curEval->lowerAsStructured()) {
        curEval = &curEval->getFirstNestedEvaluation();
        for (uint64_t i = 1; i < loopCount; i++)
          curEval = &*std::next(curEval->getNestedEvaluations().begin());
      }
    }

    for (language::Compability::lower::pft::Evaluation &e : curEval->getNestedEvaluations())
      genFIR(e);
    localSymbols.popScope();
    builder->restoreInsertionPoint(insertPt);

    if (accLoop && exitCond) {
      language::Compability::lower::pft::FunctionLikeUnit *funit =
          getEval().getOwningProcedure();
      assert(funit && "not inside main program, function or subroutine");
      mlir::Block *continueBlock =
          builder->getBlock()->splitBlock(builder->getBlock()->end());
      mlir::cf::CondBranchOp::create(*builder, toLocation(), exitCond,
                                     funit->finalBlock, continueBlock);
      builder->setInsertionPointToEnd(continueBlock);
    }
  }

  void genFIR(const language::Compability::parser::OpenACCDeclarativeConstruct &accDecl) {
    genOpenACCDeclarativeConstruct(*this, bridge.getSemanticsContext(),
                                   bridge.openAccCtx(), accDecl);
    for (language::Compability::lower::pft::Evaluation &e : getEval().getNestedEvaluations())
      genFIR(e);
  }

  void genFIR(const language::Compability::parser::OpenACCRoutineConstruct &acc) {
    // Handled by genFIR(const language::Compability::parser::OpenACCDeclarativeConstruct &)
  }

  void genFIR(const language::Compability::parser::CUFKernelDoConstruct &kernel) {
    language::Compability::lower::SymMapScope scope(localSymbols);
    const language::Compability::parser::CUFKernelDoConstruct::Directive &dir =
        std::get<language::Compability::parser::CUFKernelDoConstruct::Directive>(kernel.t);

    mlir::Location loc = genLocation(dir.source);

    language::Compability::lower::StatementContext stmtCtx;

    unsigned nestedLoops = 1;

    const auto &nLoops =
        std::get<std::optional<language::Compability::parser::ScalarIntConstantExpr>>(dir.t);
    if (nLoops)
      nestedLoops = *language::Compability::semantics::GetIntValue(*nLoops);

    mlir::IntegerAttr n;
    if (nestedLoops > 1)
      n = builder->getIntegerAttr(builder->getI64Type(), nestedLoops);

    const auto &launchConfig = std::get<std::optional<
        language::Compability::parser::CUFKernelDoConstruct::LaunchConfiguration>>(dir.t);

    const std::list<language::Compability::parser::CUFReduction> &cufreds =
        std::get<2>(dir.t);

    toolchain::SmallVector<mlir::Value> reduceOperands;
    toolchain::SmallVector<mlir::Attribute> reduceAttrs;

    for (const language::Compability::parser::CUFReduction &cufred : cufreds) {
      fir::ReduceOperationEnum redOpEnum = getReduceOperationEnum(
          std::get<language::Compability::parser::ReductionOperator>(cufred.t));
      const std::list<language::Compability::parser::Scalar<language::Compability::parser::Variable>>
          &scalarvars = std::get<1>(cufred.t);
      for (const language::Compability::parser::Scalar<language::Compability::parser::Variable> &scalarvar :
           scalarvars) {
        auto reduce_attr =
            fir::ReduceAttr::get(builder->getContext(), redOpEnum);
        reduceAttrs.push_back(reduce_attr);
        const language::Compability::parser::Variable &var = scalarvar.thing;
        if (const auto *iDesignator = std::get_if<
                language::Compability::common::Indirection<language::Compability::parser::Designator>>(
                &var.u)) {
          const language::Compability::parser::Designator &designator = iDesignator->value();
          if (const auto *name =
                  language::Compability::semantics::getDesignatorNameIfDataRef(designator)) {
            auto val = getSymbolAddress(*name->symbol);
            reduceOperands.push_back(val);
          }
        }
      }
    }

    auto isOnlyStars =
        [&](const std::list<language::Compability::parser::CUFKernelDoConstruct::StarOrExpr>
                &list) -> bool {
      for (const language::Compability::parser::CUFKernelDoConstruct::StarOrExpr &expr :
           list) {
        if (expr.v)
          return false;
      }
      return true;
    };

    mlir::Value zero =
        builder->createIntegerConstant(loc, builder->getI32Type(), 0);

    toolchain::SmallVector<mlir::Value> gridValues;
    toolchain::SmallVector<mlir::Value> blockValues;
    mlir::Value streamAddr;

    if (launchConfig) {
      const std::list<language::Compability::parser::CUFKernelDoConstruct::StarOrExpr> &grid =
          std::get<0>(launchConfig->t);
      const std::list<language::Compability::parser::CUFKernelDoConstruct::StarOrExpr>
          &block = std::get<1>(launchConfig->t);
      const std::optional<language::Compability::parser::ScalarIntExpr> &stream =
          std::get<2>(launchConfig->t);
      if (!isOnlyStars(grid)) {
        for (const language::Compability::parser::CUFKernelDoConstruct::StarOrExpr &expr :
             grid) {
          if (expr.v) {
            gridValues.push_back(fir::getBase(
                genExprValue(*language::Compability::semantics::GetExpr(*expr.v), stmtCtx)));
          } else {
            gridValues.push_back(zero);
          }
        }
      }
      if (!isOnlyStars(block)) {
        for (const language::Compability::parser::CUFKernelDoConstruct::StarOrExpr &expr :
             block) {
          if (expr.v) {
            blockValues.push_back(fir::getBase(
                genExprValue(*language::Compability::semantics::GetExpr(*expr.v), stmtCtx)));
          } else {
            blockValues.push_back(zero);
          }
        }
      }

      if (stream)
        streamAddr = fir::getBase(
            genExprAddr(*language::Compability::semantics::GetExpr(*stream), stmtCtx));
    }

    const auto &outerDoConstruct =
        std::get<std::optional<language::Compability::parser::DoConstruct>>(kernel.t);

    toolchain::SmallVector<mlir::Location> locs;
    locs.push_back(loc);
    toolchain::SmallVector<mlir::Value> lbs, ubs, steps;

    mlir::Type idxTy = builder->getIndexType();

    toolchain::SmallVector<mlir::Type> ivTypes;
    toolchain::SmallVector<mlir::Location> ivLocs;
    toolchain::SmallVector<mlir::Value> ivValues;
    language::Compability::lower::pft::Evaluation *loopEval =
        &getEval().getFirstNestedEvaluation();
    if (outerDoConstruct->IsDoConcurrent()) {
      // Handle DO CONCURRENT
      locs.push_back(
          genLocation(language::Compability::parser::FindSourceLocation(outerDoConstruct)));
      const language::Compability::parser::LoopControl *loopControl =
          &*outerDoConstruct->GetLoopControl();
      const auto &concurrent =
          std::get<language::Compability::parser::LoopControl::Concurrent>(loopControl->u);

      if (!std::get<std::list<language::Compability::parser::LocalitySpec>>(concurrent.t)
               .empty())
        TODO(loc, "DO CONCURRENT with locality spec");

      const auto &concurrentHeader =
          std::get<language::Compability::parser::ConcurrentHeader>(concurrent.t);
      const auto &controls =
          std::get<std::list<language::Compability::parser::ConcurrentControl>>(
              concurrentHeader.t);

      for (const auto &control : controls) {
        mlir::Value lb = fir::getBase(genExprValue(
            *language::Compability::semantics::GetExpr(std::get<1>(control.t)), stmtCtx));
        mlir::Value ub = fir::getBase(genExprValue(
            *language::Compability::semantics::GetExpr(std::get<2>(control.t)), stmtCtx));
        mlir::Value step;

        if (const auto &expr =
                std::get<std::optional<language::Compability::parser::ScalarIntExpr>>(
                    control.t))
          step = fir::getBase(
              genExprValue(*language::Compability::semantics::GetExpr(*expr), stmtCtx));
        else
          step = mlir::arith::ConstantIndexOp::create(
              *builder, loc, 1); // Use index type directly

        // Ensure lb, ub, and step are of index type using fir.convert
        lb = fir::ConvertOp::create(*builder, loc, idxTy, lb);
        ub = fir::ConvertOp::create(*builder, loc, idxTy, ub);
        step = fir::ConvertOp::create(*builder, loc, idxTy, step);

        lbs.push_back(lb);
        ubs.push_back(ub);
        steps.push_back(step);

        const auto &name = std::get<language::Compability::parser::Name>(control.t);

        // Handle induction variable
        mlir::Value ivValue = getSymbolAddress(*name.symbol);

        if (!ivValue) {
          // DO CONCURRENT induction variables are not mapped yet since they are
          // local to the DO CONCURRENT scope.
          mlir::OpBuilder::InsertPoint insPt = builder->saveInsertionPoint();
          builder->setInsertionPointToStart(builder->getAllocaBlock());
          ivValue = builder->createTemporaryAlloc(
              loc, idxTy, toStringRef(name.symbol->name()));
          builder->restoreInsertionPoint(insPt);
        }

        // Bind the symbol to the declared variable
        bindSymbol(*name.symbol, ivValue);
        language::Compability::lower::SymbolBox hsb = localSymbols.lookupSymbol(*name.symbol);
        fir::ExtendedValue extIvValue = symBoxToExtendedValue(hsb);
        ivValue = fir::getBase(extIvValue);
        ivValues.push_back(ivValue);
        ivTypes.push_back(idxTy);
        ivLocs.push_back(loc);
      }
    } else {
      for (unsigned i = 0; i < nestedLoops; ++i) {
        const language::Compability::parser::LoopControl *loopControl;
        mlir::Location crtLoc = loc;
        if (i == 0) {
          loopControl = &*outerDoConstruct->GetLoopControl();
          crtLoc = genLocation(
              language::Compability::parser::FindSourceLocation(outerDoConstruct));
        } else {
          auto *doCons = loopEval->getIf<language::Compability::parser::DoConstruct>();
          assert(doCons && "expect do construct");
          loopControl = &*doCons->GetLoopControl();
          crtLoc = genLocation(language::Compability::parser::FindSourceLocation(*doCons));
        }

        locs.push_back(crtLoc);

        const language::Compability::parser::LoopControl::Bounds *bounds =
            std::get_if<language::Compability::parser::LoopControl::Bounds>(&loopControl->u);
        assert(bounds && "Expected bounds on the loop construct");

        language::Compability::semantics::Symbol &ivSym =
            bounds->name.thing.symbol->GetUltimate();
        ivValues.push_back(getSymbolAddress(ivSym));

        lbs.push_back(builder->createConvert(
            crtLoc, idxTy,
            fir::getBase(genExprValue(
                *language::Compability::semantics::GetExpr(bounds->lower), stmtCtx))));
        ubs.push_back(builder->createConvert(
            crtLoc, idxTy,
            fir::getBase(genExprValue(
                *language::Compability::semantics::GetExpr(bounds->upper), stmtCtx))));
        if (bounds->step)
          steps.push_back(builder->createConvert(
              crtLoc, idxTy,
              fir::getBase(genExprValue(
                  *language::Compability::semantics::GetExpr(bounds->step), stmtCtx))));
        else // If `step` is not present, assume it is `1`.
          steps.push_back(builder->createIntegerConstant(loc, idxTy, 1));

        ivTypes.push_back(idxTy);
        ivLocs.push_back(crtLoc);
        if (i < nestedLoops - 1)
          loopEval = &*std::next(loopEval->getNestedEvaluations().begin());
      }
    }

    auto op = cuf::KernelOp::create(
        *builder, loc, gridValues, blockValues, streamAddr, lbs, ubs, steps, n,
        mlir::ValueRange(reduceOperands), builder->getArrayAttr(reduceAttrs));
    builder->createBlock(&op.getRegion(), op.getRegion().end(), ivTypes,
                         ivLocs);
    mlir::Block &b = op.getRegion().back();
    builder->setInsertionPointToStart(&b);

    language::Compability::lower::pft::Evaluation *crtEval = &getEval();
    if (crtEval->lowerAsUnstructured())
      language::Compability::lower::createEmptyRegionBlocks<fir::FirEndOp>(
          *builder, crtEval->getNestedEvaluations());
    builder->setInsertionPointToStart(&b);

    for (auto [arg, value] : toolchain::zip(
             op.getLoopRegions().front()->front().getArguments(), ivValues)) {
      mlir::Value convArg =
          builder->createConvert(loc, fir::unwrapRefType(value.getType()), arg);
      fir::StoreOp::create(*builder, loc, convArg, value);
    }

    if (crtEval->lowerAsStructured()) {
      crtEval = &crtEval->getFirstNestedEvaluation();
      for (int64_t i = 1; i < nestedLoops; i++)
        crtEval = &*std::next(crtEval->getNestedEvaluations().begin());
    }

    // Generate loop body
    for (language::Compability::lower::pft::Evaluation &e : crtEval->getNestedEvaluations())
      genFIR(e);

    fir::FirEndOp::create(*builder, loc);
    builder->setInsertionPointAfter(op);
  }

  void genFIR(const language::Compability::parser::OpenMPConstruct &omp) {
    mlir::OpBuilder::InsertPoint insertPt = builder->saveInsertionPoint();
    genOpenMPConstruct(*this, localSymbols, bridge.getSemanticsContext(),
                       getEval(), omp);
    builder->restoreInsertionPoint(insertPt);

    // Register if a target region was found
    ompDeviceCodeFound =
        ompDeviceCodeFound || language::Compability::lower::isOpenMPTargetConstruct(omp);
  }

  void genFIR(const language::Compability::parser::OpenMPDeclarativeConstruct &ompDecl) {
    mlir::OpBuilder::InsertPoint insertPt = builder->saveInsertionPoint();
    // Register if a declare target construct intended for a target device was
    // found
    ompDeviceCodeFound =
        ompDeviceCodeFound ||
        language::Compability::lower::isOpenMPDeviceDeclareTarget(
            *this, bridge.getSemanticsContext(), getEval(), ompDecl);
    language::Compability::lower::gatherOpenMPDeferredDeclareTargets(
        *this, bridge.getSemanticsContext(), getEval(), ompDecl,
        ompDeferredDeclareTarget);
    genOpenMPDeclarativeConstruct(
        *this, localSymbols, bridge.getSemanticsContext(), getEval(), ompDecl);
    builder->restoreInsertionPoint(insertPt);
  }

  /// Generate FIR for a SELECT CASE statement.
  /// The selector may have CHARACTER, INTEGER, UNSIGNED, or LOGICAL type.
  void genFIR(const language::Compability::parser::SelectCaseStmt &stmt) {
    language::Compability::lower::pft::Evaluation &eval = getEval();
    language::Compability::lower::pft::Evaluation *parentConstruct = eval.parentConstruct;
    assert(!activeConstructStack.empty() &&
           &activeConstructStack.back().eval == parentConstruct &&
           "select case construct is not active");
    language::Compability::lower::StatementContext &stmtCtx =
        activeConstructStack.back().stmtCtx;
    const language::Compability::lower::SomeExpr *expr = language::Compability::semantics::GetExpr(
        std::get<language::Compability::parser::Scalar<language::Compability::parser::Expr>>(stmt.t));
    bool isCharSelector = isCharacterCategory(expr->GetType()->category());
    bool isLogicalSelector = isLogicalCategory(expr->GetType()->category());
    mlir::MLIRContext *context = builder->getContext();
    mlir::Location loc = toLocation();
    auto charValue = [&](const language::Compability::lower::SomeExpr *expr) {
      fir::ExtendedValue exv = genExprAddr(*expr, stmtCtx, &loc);
      return exv.match(
          [&](const fir::CharBoxValue &cbv) {
            return fir::factory::CharacterExprHelper{*builder, loc}
                .createEmboxChar(cbv.getAddr(), cbv.getLen());
          },
          [&](auto) {
            fir::emitFatalError(loc, "not a character");
            return mlir::Value{};
          });
    };
    mlir::Value selector;
    if (isCharSelector) {
      selector = charValue(expr);
    } else {
      selector = createFIRExpr(loc, expr, stmtCtx);
      if (isLogicalSelector)
        selector = builder->createConvert(loc, builder->getI1Type(), selector);
    }
    mlir::Type selectType = selector.getType();
    if (selectType.isUnsignedInteger())
      selectType = mlir::IntegerType::get(
          builder->getContext(), selectType.getIntOrFloatBitWidth(),
          mlir::IntegerType::SignednessSemantics::Signless);
    toolchain::SmallVector<mlir::Attribute> attrList;
    toolchain::SmallVector<mlir::Value> valueList;
    toolchain::SmallVector<mlir::Block *> blockList;
    mlir::Block *defaultBlock = parentConstruct->constructExit->block;
    using CaseValue = language::Compability::parser::Scalar<language::Compability::parser::ConstantExpr>;
    auto addValue = [&](const CaseValue &caseValue) {
      const language::Compability::lower::SomeExpr *expr =
          language::Compability::semantics::GetExpr(caseValue.thing);
      if (isCharSelector)
        valueList.push_back(charValue(expr));
      else if (isLogicalSelector)
        valueList.push_back(builder->createConvert(
            loc, selectType, createFIRExpr(toLocation(), expr, stmtCtx)));
      else {
        valueList.push_back(builder->createIntegerConstant(
            loc, selectType, *language::Compability::evaluate::ToInt64(*expr)));
      }
    };
    for (language::Compability::lower::pft::Evaluation *e = eval.controlSuccessor; e;
         e = e->controlSuccessor) {
      const auto &caseStmt = e->getIf<language::Compability::parser::CaseStmt>();
      assert(e->block && "missing CaseStmt block");
      const auto &caseSelector =
          std::get<language::Compability::parser::CaseSelector>(caseStmt->t);
      const auto *caseValueRangeList =
          std::get_if<std::list<language::Compability::parser::CaseValueRange>>(
              &caseSelector.u);
      if (!caseValueRangeList) {
        defaultBlock = e->block;
        continue;
      }
      for (const language::Compability::parser::CaseValueRange &caseValueRange :
           *caseValueRangeList) {
        blockList.push_back(e->block);
        if (const auto *caseValue = std::get_if<CaseValue>(&caseValueRange.u)) {
          attrList.push_back(fir::PointIntervalAttr::get(context));
          addValue(*caseValue);
          continue;
        }
        const auto &caseRange =
            std::get<language::Compability::parser::CaseValueRange::Range>(caseValueRange.u);
        if (caseRange.lower && caseRange.upper) {
          attrList.push_back(fir::ClosedIntervalAttr::get(context));
          addValue(*caseRange.lower);
          addValue(*caseRange.upper);
        } else if (caseRange.lower) {
          attrList.push_back(fir::LowerBoundAttr::get(context));
          addValue(*caseRange.lower);
        } else {
          attrList.push_back(fir::UpperBoundAttr::get(context));
          addValue(*caseRange.upper);
        }
      }
    }
    // Skip a logical default block that can never be referenced.
    if (isLogicalSelector && attrList.size() == 2)
      defaultBlock = parentConstruct->constructExit->block;
    attrList.push_back(mlir::UnitAttr::get(context));
    blockList.push_back(defaultBlock);

    // Generate a fir::SelectCaseOp. Explicit branch code is better for the
    // LOGICAL type. The CHARACTER type does not have downstream SelectOp
    // support. The -no-structured-fir option can be used to force generation
    // of INTEGER type branch code.
    if (!isLogicalSelector && !isCharSelector &&
        !getEval().forceAsUnstructured()) {
      // The selector is in an ssa register. Any temps that may have been
      // generated while evaluating it can be cleaned up now.
      stmtCtx.finalizeAndReset();
      fir::SelectCaseOp::create(*builder, loc, selector, attrList, valueList,
                                blockList);
      return;
    }

    // Generate a sequence of case value comparisons and branches.
    auto caseValue = valueList.begin();
    auto caseBlock = blockList.begin();
    for (mlir::Attribute attr : attrList) {
      if (mlir::isa<mlir::UnitAttr>(attr)) {
        genBranch(*caseBlock++);
        break;
      }
      auto genCond = [&](mlir::Value rhs,
                         mlir::arith::CmpIPredicate pred) -> mlir::Value {
        if (!isCharSelector)
          return mlir::arith::CmpIOp::create(*builder, loc, pred, selector,
                                             rhs);
        fir::factory::CharacterExprHelper charHelper{*builder, loc};
        std::pair<mlir::Value, mlir::Value> lhsVal =
            charHelper.createUnboxChar(selector);
        std::pair<mlir::Value, mlir::Value> rhsVal =
            charHelper.createUnboxChar(rhs);
        return fir::runtime::genCharCompare(*builder, loc, pred, lhsVal.first,
                                            lhsVal.second, rhsVal.first,
                                            rhsVal.second);
      };
      mlir::Block *newBlock = insertBlock(*caseBlock);
      if (mlir::isa<fir::ClosedIntervalAttr>(attr)) {
        mlir::Block *newBlock2 = insertBlock(*caseBlock);
        mlir::Value cond =
            genCond(*caseValue++, mlir::arith::CmpIPredicate::sge);
        genConditionalBranch(cond, newBlock, newBlock2);
        builder->setInsertionPointToEnd(newBlock);
        mlir::Value cond2 =
            genCond(*caseValue++, mlir::arith::CmpIPredicate::sle);
        genConditionalBranch(cond2, *caseBlock++, newBlock2);
        builder->setInsertionPointToEnd(newBlock2);
        continue;
      }
      mlir::arith::CmpIPredicate pred;
      if (mlir::isa<fir::PointIntervalAttr>(attr)) {
        pred = mlir::arith::CmpIPredicate::eq;
      } else if (mlir::isa<fir::LowerBoundAttr>(attr)) {
        pred = mlir::arith::CmpIPredicate::sge;
      } else {
        assert(mlir::isa<fir::UpperBoundAttr>(attr) && "unexpected predicate");
        pred = mlir::arith::CmpIPredicate::sle;
      }
      mlir::Value cond = genCond(*caseValue++, pred);
      genConditionalBranch(cond, *caseBlock++, newBlock);
      builder->setInsertionPointToEnd(newBlock);
    }
    assert(caseValue == valueList.end() && caseBlock == blockList.end() &&
           "select case list mismatch");
  }

  fir::ExtendedValue
  genAssociateSelector(const language::Compability::lower::SomeExpr &selector,
                       language::Compability::lower::StatementContext &stmtCtx) {
    if (lowerToHighLevelFIR())
      return genExprAddr(selector, stmtCtx);
    return language::Compability::lower::isArraySectionWithoutVectorSubscript(selector)
               ? language::Compability::lower::createSomeArrayBox(*this, selector,
                                                    localSymbols, stmtCtx)
               : genExprAddr(selector, stmtCtx);
  }

  void genFIR(const language::Compability::parser::AssociateConstruct &) {
    language::Compability::lower::pft::Evaluation &eval = getEval();
    language::Compability::lower::StatementContext stmtCtx;
    pushActiveConstruct(eval, stmtCtx);
    for (language::Compability::lower::pft::Evaluation &e : eval.getNestedEvaluations()) {
      setCurrentPosition(e.position);
      if (auto *stmt = e.getIf<language::Compability::parser::AssociateStmt>()) {
        if (eval.lowerAsUnstructured())
          maybeStartBlock(e.block);
        localSymbols.pushScope();
        for (const language::Compability::parser::Association &assoc :
             std::get<std::list<language::Compability::parser::Association>>(stmt->t)) {
          language::Compability::semantics::Symbol &sym =
              *std::get<language::Compability::parser::Name>(assoc.t).symbol;
          const language::Compability::lower::SomeExpr &selector =
              *sym.get<language::Compability::semantics::AssocEntityDetails>().expr();
          addSymbol(sym, genAssociateSelector(selector, stmtCtx));
        }
      } else if (e.getIf<language::Compability::parser::EndAssociateStmt>()) {
        if (eval.lowerAsUnstructured())
          maybeStartBlock(e.block);
        localSymbols.popScope();
      } else {
        genFIR(e);
      }
    }
    popActiveConstruct();
  }

  void genFIR(const language::Compability::parser::BlockConstruct &blockConstruct) {
    language::Compability::lower::pft::Evaluation &eval = getEval();
    language::Compability::lower::StatementContext stmtCtx;
    pushActiveConstruct(eval, stmtCtx);
    for (language::Compability::lower::pft::Evaluation &e : eval.getNestedEvaluations()) {
      setCurrentPosition(e.position);
      if (e.getIf<language::Compability::parser::BlockStmt>()) {
        if (eval.lowerAsUnstructured())
          maybeStartBlock(e.block);
        const language::Compability::parser::CharBlock &endPosition =
            eval.getLastNestedEvaluation().position;
        localSymbols.pushScope();
        mlir::Value stackPtr = builder->genStackSave(toLocation());
        mlir::Location endLoc = genLocation(endPosition);
        stmtCtx.attachCleanup(
            [=]() { builder->genStackRestore(endLoc, stackPtr); });
        language::Compability::semantics::Scope &scope =
            bridge.getSemanticsContext().FindScope(endPosition);
        scopeBlockIdMap.try_emplace(&scope, ++blockId);
        language::Compability::lower::AggregateStoreMap storeMap;
        for (const language::Compability::lower::pft::Variable &var :
             language::Compability::lower::pft::getScopeVariableList(scope)) {
          // Do no instantiate again variables from the block host
          // that appears in specification of block variables.
          if (!var.hasSymbol() || !lookupSymbol(var.getSymbol()))
            instantiateVar(var, storeMap);
        }
      } else if (e.getIf<language::Compability::parser::EndBlockStmt>()) {
        if (eval.lowerAsUnstructured())
          maybeStartBlock(e.block);
        localSymbols.popScope();
      } else {
        genFIR(e);
      }
    }
    popActiveConstruct();
  }

  void genFIR(const language::Compability::parser::ChangeTeamConstruct &construct) {
    TODO(toLocation(), "coarray: ChangeTeamConstruct");
  }
  void genFIR(const language::Compability::parser::ChangeTeamStmt &stmt) {
    TODO(toLocation(), "coarray: ChangeTeamStmt");
  }
  void genFIR(const language::Compability::parser::EndChangeTeamStmt &stmt) {
    TODO(toLocation(), "coarray: EndChangeTeamStmt");
  }

  void genFIR(const language::Compability::parser::CriticalConstruct &criticalConstruct) {
    setCurrentPositionAt(criticalConstruct);
    TODO(toLocation(), "coarray: CriticalConstruct");
  }
  void genFIR(const language::Compability::parser::CriticalStmt &) {
    TODO(toLocation(), "coarray: CriticalStmt");
  }
  void genFIR(const language::Compability::parser::EndCriticalStmt &) {
    TODO(toLocation(), "coarray: EndCriticalStmt");
  }

  void genFIR(const language::Compability::parser::SelectRankConstruct &selectRankConstruct) {
    setCurrentPositionAt(selectRankConstruct);
    genCaseOrRankConstruct();
  }

  void genFIR(const language::Compability::parser::SelectRankStmt &selectRankStmt) {
    // Generate a fir.select_case with the selector rank. The RANK(*) case,
    // if any, is handles with a conditional branch before the fir.select_case.
    mlir::Type rankType = builder->getIntegerType(8);
    mlir::MLIRContext *context = builder->getContext();
    mlir::Location loc = toLocation();
    // Build block list for fir.select_case, and identify RANK(*) block, if any.
    // Default block must be placed last in the fir.select_case block list.
    mlir::Block *rankStarBlock = nullptr;
    language::Compability::lower::pft::Evaluation &eval = getEval();
    mlir::Block *defaultBlock = eval.parentConstruct->constructExit->block;
    toolchain::SmallVector<mlir::Attribute> attrList;
    toolchain::SmallVector<mlir::Value> valueList;
    toolchain::SmallVector<mlir::Block *> blockList;
    for (language::Compability::lower::pft::Evaluation *e = eval.controlSuccessor; e;
         e = e->controlSuccessor) {
      if (const auto *rankCaseStmt =
              e->getIf<language::Compability::parser::SelectRankCaseStmt>()) {
        const auto &rank = std::get<language::Compability::parser::SelectRankCaseStmt::Rank>(
            rankCaseStmt->t);
        assert(e->block && "missing SelectRankCaseStmt block");
        language::Compability::common::visit(
            language::Compability::common::visitors{
                [&](const language::Compability::parser::ScalarIntConstantExpr &rankExpr) {
                  blockList.emplace_back(e->block);
                  attrList.emplace_back(fir::PointIntervalAttr::get(context));
                  std::optional<std::int64_t> rankCst =
                      language::Compability::evaluate::ToInt64(
                          language::Compability::semantics::GetExpr(rankExpr));
                  assert(rankCst.has_value() &&
                         "rank expr must be constant integer");
                  valueList.emplace_back(
                      builder->createIntegerConstant(loc, rankType, *rankCst));
                },
                [&](const language::Compability::parser::Star &) {
                  rankStarBlock = e->block;
                },
                [&](const language::Compability::parser::Default &) {
                  defaultBlock = e->block;
                }},
            rank.u);
      }
    }
    attrList.push_back(mlir::UnitAttr::get(context));
    blockList.push_back(defaultBlock);

    // Lower selector.
    assert(!activeConstructStack.empty() && "must be inside construct");
    assert(!activeConstructStack.back().selector &&
           "selector should not yet be set");
    language::Compability::lower::StatementContext &stmtCtx =
        activeConstructStack.back().stmtCtx;
    const language::Compability::lower::SomeExpr *selectorExpr = language::Compability::common::visit(
        [](const auto &x) { return language::Compability::semantics::GetExpr(x); },
        std::get<language::Compability::parser::Selector>(selectRankStmt.t).u);
    assert(selectorExpr && "failed to retrieve selector expr");
    hlfir::Entity selector = language::Compability::lower::convertExprToHLFIR(
        loc, *this, *selectorExpr, localSymbols, stmtCtx);
    activeConstructStack.back().selector = selector;

    // Deal with assumed-size first. They must fall into RANK(*) if present, or
    // the default case (F'2023 11.1.10.2.). The selector cannot be an
    // assumed-size if it is allocatable or pointer, so the check is skipped.
    if (!language::Compability::evaluate::IsAllocatableOrPointerObject(*selectorExpr)) {
      mlir::Value isAssumedSize = fir::IsAssumedSizeOp::create(
          *builder, loc, builder->getI1Type(), selector);
      // Create new block to hold the fir.select_case for the non assumed-size
      // cases.
      mlir::Block *selectCaseBlock = insertBlock(blockList[0]);
      mlir::Block *assumedSizeBlock =
          rankStarBlock ? rankStarBlock : defaultBlock;
      mlir::cf::CondBranchOp::create(*builder, loc, isAssumedSize,
                                     assumedSizeBlock, mlir::ValueRange{},
                                     selectCaseBlock, mlir::ValueRange{});
      startBlock(selectCaseBlock);
    }
    // Create fir.select_case for the other rank cases.
    mlir::Value rank =
        fir::BoxRankOp::create(*builder, loc, rankType, selector);
    stmtCtx.finalizeAndReset();
    fir::SelectCaseOp::create(*builder, loc, rank, attrList, valueList,
                              blockList);
  }

  // Get associating entity symbol inside case statement scope.
  static const language::Compability::semantics::Symbol &
  getAssociatingEntitySymbol(const language::Compability::semantics::Scope &scope) {
    const language::Compability::semantics::Symbol *assocSym = nullptr;
    for (const auto &sym : scope.GetSymbols()) {
      if (sym->has<language::Compability::semantics::AssocEntityDetails>()) {
        assert(!assocSym &&
               "expect only one associating entity symbol in this scope");
        assocSym = &*sym;
      }
    }
    assert(assocSym && "should contain associating entity symbol");
    return *assocSym;
  }

  void genFIR(const language::Compability::parser::SelectRankCaseStmt &stmt) {
    assert(!activeConstructStack.empty() &&
           "must be inside select rank construct");
    // Pop previous associating entity mapping, if any, and push scope for new
    // mapping.
    if (activeConstructStack.back().pushedScope)
      localSymbols.popScope();
    localSymbols.pushScope();
    activeConstructStack.back().pushedScope = true;
    const language::Compability::semantics::Symbol &assocEntitySymbol =
        getAssociatingEntitySymbol(
            bridge.getSemanticsContext().FindScope(getEval().position));
    const auto &details =
        assocEntitySymbol.get<language::Compability::semantics::AssocEntityDetails>();
    assert(!activeConstructStack.empty() &&
           activeConstructStack.back().selector.has_value() &&
           "selector must have been created");
    // Get lowered value for the selector.
    hlfir::Entity selector = *activeConstructStack.back().selector;
    assert(selector.isVariable() && "assumed-rank selector are variables");
    // Cook selector mlir::Value according to rank case and map it to
    // associating entity symbol.
    language::Compability::lower::StatementContext stmtCtx;
    mlir::Location loc = toLocation();
    if (details.IsAssumedRank()) {
      fir::ExtendedValue selectorExv = language::Compability::lower::translateToExtendedValue(
          loc, *builder, selector, stmtCtx);
      addSymbol(assocEntitySymbol, selectorExv);
    } else if (details.IsAssumedSize()) {
      // Create rank-1 assumed-size from descriptor. Assumed-size are contiguous
      // so a new entity can be built from scratch using the base address, type
      // parameters and dynamic type. The selector cannot be a
      // POINTER/ALLOCATBLE as per F'2023 C1160.
      fir::ExtendedValue newExv;
      toolchain::SmallVector assumeSizeExtents{
          builder->createMinusOneInteger(loc, builder->getIndexType())};
      mlir::Value baseAddr =
          hlfir::genVariableRawAddress(loc, *builder, selector);
      const bool isVolatile = fir::isa_volatile_type(selector.getType());
      mlir::Type eleType =
          fir::unwrapSequenceType(fir::unwrapRefType(baseAddr.getType()));
      mlir::Type rank1Type = fir::ReferenceType::get(
          builder->getVarLenSeqTy(eleType, 1), isVolatile);
      baseAddr = builder->createConvert(loc, rank1Type, baseAddr);
      if (selector.isCharacter()) {
        mlir::Value len = hlfir::genCharLength(loc, *builder, selector);
        newExv = fir::CharArrayBoxValue{baseAddr, len, assumeSizeExtents};
      } else if (selector.isDerivedWithLengthParameters()) {
        TODO(loc, "RANK(*) with parameterized derived type selector");
      } else if (selector.isPolymorphic()) {
        TODO(loc, "RANK(*) with polymorphic selector");
      } else {
        // Simple intrinsic or derived type.
        newExv = fir::ArrayBoxValue{baseAddr, assumeSizeExtents};
      }
      addSymbol(assocEntitySymbol, newExv);
    } else {
      int rank = details.rank().value();
      auto boxTy =
          mlir::cast<fir::BaseBoxType>(fir::unwrapRefType(selector.getType()));
      mlir::Type newBoxType = boxTy.getBoxTypeWithNewShape(rank);
      if (fir::isa_ref_type(selector.getType()))
        newBoxType = fir::ReferenceType::get(
            newBoxType, fir::isa_volatile_type(selector.getType()));
      // Give rank info to value via cast, and get rid of the box if not needed
      // (simple scalars, contiguous arrays... This is done by
      // translateVariableToExtendedValue).
      hlfir::Entity rankedBox{
          builder->createConvert(loc, newBoxType, selector)};
      bool isSimplyContiguous = language::Compability::evaluate::IsSimplyContiguous(
          assocEntitySymbol, getFoldingContext());
      fir::ExtendedValue newExv = language::Compability::lower::translateToExtendedValue(
          loc, *builder, rankedBox, stmtCtx, isSimplyContiguous);

      // Non deferred length parameters of character allocatable/pointer
      // MutableBoxValue should be properly set before binding it to a symbol in
      // order to get correct assignment semantics.
      if (const fir::MutableBoxValue *mutableBox =
              newExv.getBoxOf<fir::MutableBoxValue>()) {
        if (selector.isCharacter()) {
          auto dynamicType =
              language::Compability::evaluate::DynamicType::From(assocEntitySymbol);
          if (!dynamicType.value().HasDeferredTypeParameter()) {
            toolchain::SmallVector<mlir::Value> lengthParams;
            hlfir::genLengthParameters(loc, *builder, selector, lengthParams);
            newExv = fir::MutableBoxValue{rankedBox, lengthParams,
                                          mutableBox->getMutableProperties()};
          }
        }
      }
      addSymbol(assocEntitySymbol, newExv);
    }
    // Statements inside rank case are lowered by SelectRankConstruct visit.
  }

  void genFIR(const language::Compability::parser::SelectTypeConstruct &selectTypeConstruct) {
    mlir::MLIRContext *context = builder->getContext();
    language::Compability::lower::StatementContext stmtCtx;
    fir::ExtendedValue selector;
    toolchain::SmallVector<mlir::Attribute> attrList;
    toolchain::SmallVector<mlir::Block *> blockList;
    unsigned typeGuardIdx = 0;
    std::size_t defaultAttrPos = std::numeric_limits<size_t>::max();
    bool hasLocalScope = false;
    toolchain::SmallVector<const language::Compability::semantics::Scope *> typeCaseScopes;

    const auto selectorIsVolatile = [&selector]() {
      return fir::isa_volatile_type(fir::getBase(selector).getType());
    };

    const auto &typeCaseList =
        std::get<std::list<language::Compability::parser::SelectTypeConstruct::TypeCase>>(
            selectTypeConstruct.t);
    for (const auto &typeCase : typeCaseList) {
      const auto &stmt =
          std::get<language::Compability::parser::Statement<language::Compability::parser::TypeGuardStmt>>(
              typeCase.t);
      const language::Compability::semantics::Scope &scope =
          bridge.getSemanticsContext().FindScope(stmt.source);
      typeCaseScopes.push_back(&scope);
    }

    pushActiveConstruct(getEval(), stmtCtx);
    toolchain::SmallVector<language::Compability::lower::pft::Evaluation *> exits, fallThroughs;
    collectFinalEvaluations(getEval(), exits, fallThroughs);
    language::Compability::lower::pft::Evaluation &constructExit = *getEval().constructExit;

    for (language::Compability::lower::pft::Evaluation &eval :
         getEval().getNestedEvaluations()) {
      setCurrentPosition(eval.position);
      mlir::Location loc = toLocation();
      if (auto *selectTypeStmt =
              eval.getIf<language::Compability::parser::SelectTypeStmt>()) {
        // A genFIR(SelectTypeStmt) call would have unwanted side effects.
        maybeStartBlock(eval.block);
        // Retrieve the selector
        const auto &s = std::get<language::Compability::parser::Selector>(selectTypeStmt->t);
        if (const auto *v = std::get_if<language::Compability::parser::Variable>(&s.u))
          selector = genExprBox(loc, *language::Compability::semantics::GetExpr(*v), stmtCtx);
        else if (const auto *e = std::get_if<language::Compability::parser::Expr>(&s.u))
          selector = genExprBox(loc, *language::Compability::semantics::GetExpr(*e), stmtCtx);

        // Going through the controlSuccessor first to create the
        // fir.select_type operation.
        mlir::Block *defaultBlock = eval.parentConstruct->constructExit->block;
        for (language::Compability::lower::pft::Evaluation *e = eval.controlSuccessor; e;
             e = e->controlSuccessor) {
          const auto &typeGuardStmt =
              e->getIf<language::Compability::parser::TypeGuardStmt>();
          const auto &guard =
              std::get<language::Compability::parser::TypeGuardStmt::Guard>(typeGuardStmt->t);
          assert(e->block && "missing TypeGuardStmt block");
          // CLASS DEFAULT
          if (std::holds_alternative<language::Compability::parser::Default>(guard.u)) {
            defaultBlock = e->block;
            // Keep track of the actual position of the CLASS DEFAULT type guard
            // in the SELECT TYPE construct.
            defaultAttrPos = attrList.size();
            continue;
          }

          blockList.push_back(e->block);
          if (const auto *typeSpec =
                  std::get_if<language::Compability::parser::TypeSpec>(&guard.u)) {
            // TYPE IS
            mlir::Type ty;
            if (std::holds_alternative<language::Compability::parser::IntrinsicTypeSpec>(
                    typeSpec->u)) {
              const language::Compability::semantics::IntrinsicTypeSpec *intrinsic =
                  typeSpec->declTypeSpec->AsIntrinsic();
              int kind =
                  language::Compability::evaluate::ToInt64(intrinsic->kind()).value_or(kind);
              toolchain::SmallVector<language::Compability::lower::LenParameterTy> params;
              ty = genType(intrinsic->category(), kind, params);
            } else {
              const language::Compability::semantics::DerivedTypeSpec *derived =
                  typeSpec->declTypeSpec->AsDerived();
              ty = genType(*derived);
            }
            attrList.push_back(fir::ExactTypeAttr::get(ty));
          } else if (const auto *derived =
                         std::get_if<language::Compability::parser::DerivedTypeSpec>(
                             &guard.u)) {
            // CLASS IS
            assert(derived->derivedTypeSpec && "derived type spec is null");
            mlir::Type ty = genType(*(derived->derivedTypeSpec));
            attrList.push_back(fir::SubclassAttr::get(ty));
          }
        }
        attrList.push_back(mlir::UnitAttr::get(context));
        blockList.push_back(defaultBlock);
        fir::SelectTypeOp::create(*builder, loc, fir::getBase(selector),
                                  attrList, blockList);

        // If the actual position of CLASS DEFAULT type guard is not the last
        // one, it needs to be put back at its correct position for the rest of
        // the processing. TypeGuardStmt are processed in the same order they
        // appear in the Fortran code.
        if (defaultAttrPos < attrList.size() - 1) {
          auto attrIt = attrList.begin();
          attrIt = attrIt + defaultAttrPos;
          auto blockIt = blockList.begin();
          blockIt = blockIt + defaultAttrPos;
          attrList.insert(attrIt, mlir::UnitAttr::get(context));
          blockList.insert(blockIt, defaultBlock);
          attrList.pop_back();
          blockList.pop_back();
        }
      } else if (auto *typeGuardStmt =
                     eval.getIf<language::Compability::parser::TypeGuardStmt>()) {
        // Map the type guard local symbol for the selector to a more precise
        // typed entity in the TypeGuardStmt when necessary.
        genFIR(eval);
        const auto &guard =
            std::get<language::Compability::parser::TypeGuardStmt::Guard>(typeGuardStmt->t);
        if (hasLocalScope)
          localSymbols.popScope();
        localSymbols.pushScope();
        hasLocalScope = true;
        assert(attrList.size() >= typeGuardIdx &&
               "TypeGuard attribute missing");
        mlir::Attribute typeGuardAttr = attrList[typeGuardIdx];
        mlir::Block *typeGuardBlock = blockList[typeGuardIdx];
        mlir::OpBuilder::InsertPoint crtInsPt = builder->saveInsertionPoint();
        builder->setInsertionPointToStart(typeGuardBlock);

        auto addAssocEntitySymbol = [&](fir::ExtendedValue exv) {
          for (auto &symbol : typeCaseScopes[typeGuardIdx]->GetSymbols()) {
            if (symbol->GetUltimate()
                    .detailsIf<language::Compability::semantics::AssocEntityDetails>()) {
              addSymbol(symbol, exv);
              break;
            }
          }
        };

        mlir::Type baseTy = fir::getBase(selector).getType();
        bool isPointer = fir::isPointerType(baseTy);
        bool isAllocatable = fir::isAllocatableType(baseTy);
        bool isArray =
            mlir::isa<fir::SequenceType>(fir::dyn_cast_ptrOrBoxEleTy(baseTy));
        const fir::BoxValue *selectorBox = selector.getBoxOf<fir::BoxValue>();
        if (std::holds_alternative<language::Compability::parser::Default>(guard.u)) {
          // CLASS DEFAULT
          addAssocEntitySymbol(selector);
        } else if (const auto *typeSpec =
                       std::get_if<language::Compability::parser::TypeSpec>(&guard.u)) {
          // TYPE IS
          fir::ExactTypeAttr attr =
              mlir::dyn_cast<fir::ExactTypeAttr>(typeGuardAttr);
          mlir::Value exactValue;
          mlir::Type addrTy = attr.getType();
          if (isArray) {
            auto seqTy = mlir::dyn_cast<fir::SequenceType>(
                fir::dyn_cast_ptrOrBoxEleTy(baseTy));
            addrTy = fir::SequenceType::get(seqTy.getShape(), attr.getType());
          }
          if (isPointer)
            addrTy = fir::PointerType::get(addrTy);
          if (isAllocatable)
            addrTy = fir::HeapType::get(addrTy);
          if (std::holds_alternative<language::Compability::parser::IntrinsicTypeSpec>(
                  typeSpec->u)) {
            mlir::Type refTy =
                fir::ReferenceType::get(addrTy, selectorIsVolatile());
            if (isPointer || isAllocatable)
              refTy = addrTy;
            exactValue = fir::BoxAddrOp::create(*builder, loc, refTy,
                                                fir::getBase(selector));
            const language::Compability::semantics::IntrinsicTypeSpec *intrinsic =
                typeSpec->declTypeSpec->AsIntrinsic();
            if (isArray) {
              mlir::Value exact = fir::ConvertOp::create(
                  *builder, loc,
                  fir::BoxType::get(addrTy, selectorIsVolatile()),
                  fir::getBase(selector));
              addAssocEntitySymbol(selectorBox->clone(exact));
            } else if (intrinsic->category() ==
                       language::Compability::common::TypeCategory::Character) {
              auto charTy = mlir::dyn_cast<fir::CharacterType>(attr.getType());
              mlir::Value charLen =
                  fir::factory::CharacterExprHelper(*builder, loc)
                      .readLengthFromBox(fir::getBase(selector), charTy);
              addAssocEntitySymbol(fir::CharBoxValue(exactValue, charLen));
            } else {
              addAssocEntitySymbol(exactValue);
            }
          } else if (std::holds_alternative<language::Compability::parser::DerivedTypeSpec>(
                         typeSpec->u)) {
            exactValue = fir::ConvertOp::create(
                *builder, loc, fir::BoxType::get(addrTy, selectorIsVolatile()),
                fir::getBase(selector));
            addAssocEntitySymbol(selectorBox->clone(exactValue));
          }
        } else if (std::holds_alternative<language::Compability::parser::DerivedTypeSpec>(
                       guard.u)) {
          // CLASS IS
          fir::SubclassAttr attr =
              mlir::dyn_cast<fir::SubclassAttr>(typeGuardAttr);
          mlir::Type addrTy = attr.getType();
          if (isArray) {
            auto seqTy = mlir::dyn_cast<fir::SequenceType>(
                fir::dyn_cast_ptrOrBoxEleTy(baseTy));
            addrTy = fir::SequenceType::get(seqTy.getShape(), attr.getType());
          }
          if (isPointer)
            addrTy = fir::PointerType::get(addrTy);
          if (isAllocatable)
            addrTy = fir::HeapType::get(addrTy);
          mlir::Type classTy =
              fir::ClassType::get(addrTy, selectorIsVolatile());
          if (classTy == baseTy) {
            addAssocEntitySymbol(selector);
          } else {
            mlir::Value derived = fir::ConvertOp::create(
                *builder, loc, classTy, fir::getBase(selector));
            addAssocEntitySymbol(selectorBox->clone(derived));
          }
        }
        builder->restoreInsertionPoint(crtInsPt);
        ++typeGuardIdx;
      } else if (eval.getIf<language::Compability::parser::EndSelectStmt>()) {
        maybeStartBlock(eval.block);
        if (hasLocalScope)
          localSymbols.popScope();
      } else {
        genFIR(eval);
      }
      if (blockIsUnterminated()) {
        if (toolchain::is_contained(exits, &eval))
          genConstructExitBranch(constructExit);
        else if (toolchain::is_contained(fallThroughs, &eval))
          genBranch(eval.lexicalSuccessor->block);
      }
    }
    popActiveConstruct();
  }

  //===--------------------------------------------------------------------===//
  // IO statements (see io.h)
  //===--------------------------------------------------------------------===//

  void genFIR(const language::Compability::parser::BackspaceStmt &stmt) {
    mlir::Value iostat = genBackspaceStatement(*this, stmt);
    genIoConditionBranches(getEval(), stmt.v, iostat);
  }
  void genFIR(const language::Compability::parser::CloseStmt &stmt) {
    mlir::Value iostat = genCloseStatement(*this, stmt);
    genIoConditionBranches(getEval(), stmt.v, iostat);
  }
  void genFIR(const language::Compability::parser::EndfileStmt &stmt) {
    mlir::Value iostat = genEndfileStatement(*this, stmt);
    genIoConditionBranches(getEval(), stmt.v, iostat);
  }
  void genFIR(const language::Compability::parser::FlushStmt &stmt) {
    mlir::Value iostat = genFlushStatement(*this, stmt);
    genIoConditionBranches(getEval(), stmt.v, iostat);
  }
  void genFIR(const language::Compability::parser::InquireStmt &stmt) {
    mlir::Value iostat = genInquireStatement(*this, stmt);
    if (const auto *specs =
            std::get_if<std::list<language::Compability::parser::InquireSpec>>(&stmt.u))
      genIoConditionBranches(getEval(), *specs, iostat);
  }
  void genFIR(const language::Compability::parser::OpenStmt &stmt) {
    mlir::Value iostat = genOpenStatement(*this, stmt);
    genIoConditionBranches(getEval(), stmt.v, iostat);
  }
  void genFIR(const language::Compability::parser::PrintStmt &stmt) {
    genPrintStatement(*this, stmt);
  }
  void genFIR(const language::Compability::parser::ReadStmt &stmt) {
    mlir::Value iostat = genReadStatement(*this, stmt);
    genIoConditionBranches(getEval(), stmt.controls, iostat);
  }
  void genFIR(const language::Compability::parser::RewindStmt &stmt) {
    mlir::Value iostat = genRewindStatement(*this, stmt);
    genIoConditionBranches(getEval(), stmt.v, iostat);
  }
  void genFIR(const language::Compability::parser::WaitStmt &stmt) {
    mlir::Value iostat = genWaitStatement(*this, stmt);
    genIoConditionBranches(getEval(), stmt.v, iostat);
  }
  void genFIR(const language::Compability::parser::WriteStmt &stmt) {
    mlir::Value iostat = genWriteStatement(*this, stmt);
    genIoConditionBranches(getEval(), stmt.controls, iostat);
  }

  template <typename A>
  void genIoConditionBranches(language::Compability::lower::pft::Evaluation &eval,
                              const A &specList, mlir::Value iostat) {
    if (!iostat)
      return;

    language::Compability::parser::Label endLabel{};
    language::Compability::parser::Label eorLabel{};
    language::Compability::parser::Label errLabel{};
    bool hasIostat{};
    for (const auto &spec : specList) {
      language::Compability::common::visit(
          language::Compability::common::visitors{
              [&](const language::Compability::parser::EndLabel &label) {
                endLabel = label.v;
              },
              [&](const language::Compability::parser::EorLabel &label) {
                eorLabel = label.v;
              },
              [&](const language::Compability::parser::ErrLabel &label) {
                errLabel = label.v;
              },
              [&](const language::Compability::parser::StatVariable &) { hasIostat = true; },
              [](const auto &) {}},
          spec.u);
    }
    if (!endLabel && !eorLabel && !errLabel)
      return;

    // An ERR specifier branch is taken on any positive error value rather than
    // some single specific value. If ERR and IOSTAT specifiers are given and
    // END and EOR specifiers are allowed, the latter two specifiers must have
    // explicit branch targets to allow the ERR branch to be implemented as a
    // default/else target. A label=0 target for an absent END or EOR specifier
    // indicates that these specifiers have a fallthrough target. END and EOR
    // specifiers may appear on READ and WAIT statements.
    bool allSpecifiersRequired = errLabel && hasIostat &&
                                 (eval.isA<language::Compability::parser::ReadStmt>() ||
                                  eval.isA<language::Compability::parser::WaitStmt>());
    mlir::Value selector =
        builder->createConvert(toLocation(), builder->getIndexType(), iostat);
    toolchain::SmallVector<int64_t> valueList;
    toolchain::SmallVector<language::Compability::parser::Label> labelList;
    if (eorLabel || allSpecifiersRequired) {
      valueList.push_back(language::Compability::runtime::io::IostatEor);
      labelList.push_back(eorLabel ? eorLabel : 0);
    }
    if (endLabel || allSpecifiersRequired) {
      valueList.push_back(language::Compability::runtime::io::IostatEnd);
      labelList.push_back(endLabel ? endLabel : 0);
    }
    if (errLabel) {
      // Must be last. Value 0 is interpreted as any positive value, or
      // equivalently as any value other than 0, IostatEor, or IostatEnd.
      valueList.push_back(0);
      labelList.push_back(errLabel);
    }
    genMultiwayBranch(selector, valueList, labelList, eval.nonNopSuccessor());
  }

  //===--------------------------------------------------------------------===//
  // Memory allocation and deallocation
  //===--------------------------------------------------------------------===//

  void genFIR(const language::Compability::parser::AllocateStmt &stmt) {
    language::Compability::lower::genAllocateStmt(*this, stmt, toLocation());
  }

  void genFIR(const language::Compability::parser::DeallocateStmt &stmt) {
    language::Compability::lower::genDeallocateStmt(*this, stmt, toLocation());
  }

  /// Nullify pointer object list
  ///
  /// For each pointer object, reset the pointer to a disassociated status.
  /// We do this by setting each pointer to null.
  void genFIR(const language::Compability::parser::NullifyStmt &stmt) {
    mlir::Location loc = toLocation();
    for (auto &pointerObject : stmt.v) {
      const language::Compability::lower::SomeExpr *expr =
          language::Compability::semantics::GetExpr(pointerObject);
      assert(expr);
      if (language::Compability::evaluate::IsProcedurePointer(*expr)) {
        language::Compability::lower::StatementContext stmtCtx;
        hlfir::Entity pptr = language::Compability::lower::convertExprToHLFIR(
            loc, *this, *expr, localSymbols, stmtCtx);
        auto boxTy{
            language::Compability::lower::getUntypedBoxProcType(builder->getContext())};
        hlfir::Entity nullBoxProc(
            fir::factory::createNullBoxProc(*builder, loc, boxTy));
        builder->createStoreWithConvert(loc, nullBoxProc, pptr);
      } else {
        fir::MutableBoxValue box = genExprMutableBox(loc, *expr);
        fir::factory::disassociateMutableBox(*builder, loc, box);
        cuf::genPointerSync(box.getAddr(), *builder);
      }
    }
  }

  //===--------------------------------------------------------------------===//

  void genFIR(const language::Compability::parser::NotifyWaitStmt &stmt) {
    genNotifyWaitStatement(*this, stmt);
  }

  void genFIR(const language::Compability::parser::EventPostStmt &stmt) {
    genEventPostStatement(*this, stmt);
  }

  void genFIR(const language::Compability::parser::EventWaitStmt &stmt) {
    genEventWaitStatement(*this, stmt);
  }

  void genFIR(const language::Compability::parser::FormTeamStmt &stmt) {
    genFormTeamStatement(*this, getEval(), stmt);
  }

  void genFIR(const language::Compability::parser::LockStmt &stmt) {
    genLockStatement(*this, stmt);
  }

  fir::ExtendedValue
  genInitializerExprValue(const language::Compability::lower::SomeExpr &expr,
                          language::Compability::lower::StatementContext &stmtCtx) {
    return language::Compability::lower::createSomeInitializerExpression(
        toLocation(), *this, expr, localSymbols, stmtCtx);
  }

  /// Return true if the current context is a conditionalized and implied
  /// iteration space.
  bool implicitIterationSpace() { return !implicitIterSpace.empty(); }

  /// Return true if context is currently an explicit iteration space. A scalar
  /// assignment expression may be contextually within a user-defined iteration
  /// space, transforming it into an array expression.
  bool explicitIterationSpace() { return explicitIterSpace.isActive(); }

  /// Generate an array assignment.
  /// This is an assignment expression with rank > 0. The assignment may or may
  /// not be in a WHERE and/or FORALL context.
  /// In a FORALL context, the assignment may be a pointer assignment and the \p
  /// lbounds and \p ubounds parameters should only be used in such a pointer
  /// assignment case. (If both are None then the array assignment cannot be a
  /// pointer assignment.)
  void genArrayAssignment(
      const language::Compability::evaluate::Assignment &assign,
      language::Compability::lower::StatementContext &localStmtCtx,
      std::optional<toolchain::SmallVector<mlir::Value>> lbounds = std::nullopt,
      std::optional<toolchain::SmallVector<mlir::Value>> ubounds = std::nullopt) {

    language::Compability::lower::StatementContext &stmtCtx =
        explicitIterationSpace()
            ? explicitIterSpace.stmtContext()
            : (implicitIterationSpace() ? implicitIterSpace.stmtContext()
                                        : localStmtCtx);
    if (language::Compability::lower::isWholeAllocatable(assign.lhs)) {
      // Assignment to allocatables may require the lhs to be
      // deallocated/reallocated. See Fortran 2018 10.2.1.3 p3
      language::Compability::lower::createAllocatableArrayAssignment(
          *this, assign.lhs, assign.rhs, explicitIterSpace, implicitIterSpace,
          localSymbols, stmtCtx);
      return;
    }

    if (lbounds) {
      // Array of POINTER entities, with elemental assignment.
      if (!language::Compability::lower::isWholePointer(assign.lhs))
        fir::emitFatalError(toLocation(), "pointer assignment to non-pointer");

      language::Compability::lower::createArrayOfPointerAssignment(
          *this, assign.lhs, assign.rhs, explicitIterSpace, implicitIterSpace,
          *lbounds, ubounds, localSymbols, stmtCtx);
      return;
    }

    if (!implicitIterationSpace() && !explicitIterationSpace()) {
      // No masks and the iteration space is implied by the array, so create a
      // simple array assignment.
      language::Compability::lower::createSomeArrayAssignment(*this, assign.lhs, assign.rhs,
                                                localSymbols, stmtCtx);
      return;
    }

    // If there is an explicit iteration space, generate an array assignment
    // with a user-specified iteration space and possibly with masks. These
    // assignments may *appear* to be scalar expressions, but the scalar
    // expression is evaluated at all points in the user-defined space much like
    // an ordinary array assignment. More specifically, the semantics inside the
    // FORALL much more closely resembles that of WHERE than a scalar
    // assignment.
    // Otherwise, generate a masked array assignment. The iteration space is
    // implied by the lhs array expression.
    language::Compability::lower::createAnyMaskedArrayAssignment(
        *this, assign.lhs, assign.rhs, explicitIterSpace, implicitIterSpace,
        localSymbols, stmtCtx);
  }

#if !defined(NDEBUG)
  static bool isFuncResultDesignator(const language::Compability::lower::SomeExpr &expr) {
    const language::Compability::semantics::Symbol *sym =
        language::Compability::evaluate::GetFirstSymbol(expr);
    return sym && sym->IsFuncResult();
  }
#endif

  inline fir::MutableBoxValue
  genExprMutableBox(mlir::Location loc,
                    const language::Compability::lower::SomeExpr &expr) override final {
    if (lowerToHighLevelFIR())
      return language::Compability::lower::convertExprToMutableBox(loc, *this, expr,
                                                     localSymbols);
    return language::Compability::lower::createMutableBox(loc, *this, expr, localSymbols);
  }

  // Create the [newRank] array with the lower bounds to be passed to the
  // runtime as a descriptor.
  mlir::Value createLboundArray(toolchain::ArrayRef<mlir::Value> lbounds,
                                mlir::Location loc) {
    mlir::Type indexTy = builder->getIndexType();
    mlir::Type boundArrayTy = fir::SequenceType::get(
        {static_cast<int64_t>(lbounds.size())}, builder->getI64Type());
    mlir::Value boundArray = fir::AllocaOp::create(*builder, loc, boundArrayTy);
    mlir::Value array = fir::UndefOp::create(*builder, loc, boundArrayTy);
    for (unsigned i = 0; i < lbounds.size(); ++i) {
      array = fir::InsertValueOp::create(
          *builder, loc, boundArrayTy, array, lbounds[i],
          builder->getArrayAttr({builder->getIntegerAttr(
              builder->getIndexType(), static_cast<int>(i))}));
    }
    fir::StoreOp::create(*builder, loc, array, boundArray);
    mlir::Type boxTy = fir::BoxType::get(boundArrayTy);
    mlir::Value ext =
        builder->createIntegerConstant(loc, indexTy, lbounds.size());
    toolchain::SmallVector<mlir::Value> shapes = {ext};
    mlir::Value shapeOp = builder->genShape(loc, shapes);
    return fir::EmboxOp::create(*builder, loc, boxTy, boundArray, shapeOp);
  }

  // Generate pointer assignment with possibly empty bounds-spec. R1035: a
  // bounds-spec is a lower bound value.
  void genPointerAssignment(
      mlir::Location loc, const language::Compability::evaluate::Assignment &assign,
      const language::Compability::evaluate::Assignment::BoundsSpec &lbExprs) {
    language::Compability::lower::StatementContext stmtCtx;

    if (!lowerToHighLevelFIR() &&
        language::Compability::evaluate::IsProcedureDesignator(assign.rhs))
      TODO(loc, "procedure pointer assignment");
    if (language::Compability::evaluate::IsProcedurePointer(assign.lhs)) {
      hlfir::Entity lhs = language::Compability::lower::convertExprToHLFIR(
          loc, *this, assign.lhs, localSymbols, stmtCtx);
      if (language::Compability::evaluate::UnwrapExpr<language::Compability::evaluate::NullPointer>(
              assign.rhs)) {
        // rhs is null(). rhs being null(pptr) is handled in genNull.
        auto boxTy{
            language::Compability::lower::getUntypedBoxProcType(builder->getContext())};
        hlfir::Entity rhs(
            fir::factory::createNullBoxProc(*builder, loc, boxTy));
        builder->createStoreWithConvert(loc, rhs, lhs);
        return;
      }
      hlfir::Entity rhs(getBase(language::Compability::lower::convertExprToAddress(
          loc, *this, assign.rhs, localSymbols, stmtCtx)));
      builder->createStoreWithConvert(loc, rhs, lhs);
      return;
    }

    std::optional<language::Compability::evaluate::DynamicType> lhsType =
        assign.lhs.GetType();
    // Delegate pointer association to unlimited polymorphic pointer
    // to the runtime. element size, type code, attribute and of
    // course base_addr might need to be updated.
    if (lhsType && lhsType->IsPolymorphic()) {
      if (!lowerToHighLevelFIR() && explicitIterationSpace())
        TODO(loc, "polymorphic pointer assignment in FORALL");
      toolchain::SmallVector<mlir::Value> lbounds;
      for (const language::Compability::evaluate::ExtentExpr &lbExpr : lbExprs)
        lbounds.push_back(
            fir::getBase(genExprValue(toEvExpr(lbExpr), stmtCtx)));
      fir::MutableBoxValue lhsMutableBox = genExprMutableBox(loc, assign.lhs);
      if (language::Compability::evaluate::UnwrapExpr<language::Compability::evaluate::NullPointer>(
              assign.rhs)) {
        fir::factory::disassociateMutableBox(*builder, loc, lhsMutableBox);
        return;
      }
      mlir::Value lhs = lhsMutableBox.getAddr();
      mlir::Value rhs = fir::getBase(genExprBox(loc, assign.rhs, stmtCtx));
      if (!lbounds.empty()) {
        mlir::Value boundsDesc = createLboundArray(lbounds, loc);
        language::Compability::lower::genPointerAssociateLowerBounds(*builder, loc, lhs, rhs,
                                                       boundsDesc);
        return;
      }
      language::Compability::lower::genPointerAssociate(*builder, loc, lhs, rhs);
      return;
    }

    toolchain::SmallVector<mlir::Value> lbounds;
    for (const language::Compability::evaluate::ExtentExpr &lbExpr : lbExprs)
      lbounds.push_back(fir::getBase(genExprValue(toEvExpr(lbExpr), stmtCtx)));
    if (!lowerToHighLevelFIR() && explicitIterationSpace()) {
      // Pointer assignment in FORALL context. Copy the rhs box value
      // into the lhs box variable.
      genArrayAssignment(assign, stmtCtx, lbounds);
      return;
    }
    fir::MutableBoxValue lhs = genExprMutableBox(loc, assign.lhs);
    language::Compability::lower::associateMutableBox(*this, loc, lhs, assign.rhs, lbounds,
                                        stmtCtx);
  }

  void genForallPointerAssignment(mlir::Location loc,
                                  const language::Compability::evaluate::Assignment &assign) {
    // Lower pointer assignment inside forall with hlfir.region_assign with
    // descriptor address/value and later implemented with a store.
    // The RHS is fully prepared in lowering, so that all that is left
    // in hlfir.region_assign code generation is the store.
    auto regionAssignOp = hlfir::RegionAssignOp::create(*builder, loc);

    // Lower LHS in its own region.
    builder->createBlock(&regionAssignOp.getLhsRegion());
    language::Compability::lower::StatementContext lhsContext;
    hlfir::Entity lhs = language::Compability::lower::convertExprToHLFIR(
        loc, *this, assign.lhs, localSymbols, lhsContext);
    auto lhsYieldOp = hlfir::YieldOp::create(*builder, loc, lhs);
    language::Compability::lower::genCleanUpInRegionIfAny(
        loc, *builder, lhsYieldOp.getCleanup(), lhsContext);

    // Lower RHS in its own region.
    builder->createBlock(&regionAssignOp.getRhsRegion());
    language::Compability::lower::StatementContext rhsContext;
    mlir::Value rhs =
        genForallPointerAssignmentRhs(loc, lhs, assign, rhsContext);
    auto rhsYieldOp = hlfir::YieldOp::create(*builder, loc, rhs);
    language::Compability::lower::genCleanUpInRegionIfAny(
        loc, *builder, rhsYieldOp.getCleanup(), rhsContext);

    builder->setInsertionPointAfter(regionAssignOp);
  }

  mlir::Value lowerToIndexValue(mlir::Location loc,
                                const language::Compability::evaluate::ExtentExpr &expr,
                                language::Compability::lower::StatementContext &stmtCtx) {
    mlir::Value val = fir::getBase(genExprValue(toEvExpr(expr), stmtCtx));
    return builder->createConvert(loc, builder->getIndexType(), val);
  }

  mlir::Value
  genForallPointerAssignmentRhs(mlir::Location loc, mlir::Value lhs,
                                const language::Compability::evaluate::Assignment &assign,
                                language::Compability::lower::StatementContext &rhsContext) {
    if (language::Compability::evaluate::IsProcedureDesignator(assign.lhs)) {
      if (language::Compability::evaluate::UnwrapExpr<language::Compability::evaluate::NullPointer>(
              assign.rhs))
        return fir::factory::createNullBoxProc(
            *builder, loc, fir::unwrapRefType(lhs.getType()));
      return fir::getBase(language::Compability::lower::convertExprToAddress(
          loc, *this, assign.rhs, localSymbols, rhsContext));
    }
    // Data target.
    auto lhsBoxType =
        toolchain::cast<fir::BaseBoxType>(fir::unwrapRefType(lhs.getType()));
    // For NULL, create disassociated descriptor whose dynamic type is
    // the static type of the LHS.
    if (language::Compability::evaluate::UnwrapExpr<language::Compability::evaluate::NullPointer>(
            assign.rhs))
      return fir::factory::createUnallocatedBox(*builder, loc, lhsBoxType, {});
    hlfir::Entity rhs = language::Compability::lower::convertExprToHLFIR(
        loc, *this, assign.rhs, localSymbols, rhsContext);
    // Create pointer descriptor value from the RHS.
    if (rhs.isMutableBox())
      rhs = hlfir::Entity{fir::LoadOp::create(*builder, loc, rhs)};
    mlir::Value rhsBox = hlfir::genVariableBox(
        loc, *builder, rhs, lhsBoxType.getBoxTypeWithNewShape(rhs.getRank()));
    // Apply lower bounds or reshaping if any.
    if (const auto *lbExprs =
            std::get_if<language::Compability::evaluate::Assignment::BoundsSpec>(&assign.u);
        lbExprs && !lbExprs->empty()) {
      // Override target lower bounds with the LHS bounds spec.
      toolchain::SmallVector<mlir::Value> lbounds;
      for (const language::Compability::evaluate::ExtentExpr &lbExpr : *lbExprs)
        lbounds.push_back(lowerToIndexValue(loc, lbExpr, rhsContext));
      mlir::Value shift = builder->genShift(loc, lbounds);
      rhsBox = fir::ReboxOp::create(*builder, loc, lhsBoxType, rhsBox, shift,
                                    /*slice=*/mlir::Value{});
    } else if (const auto *boundExprs =
                   std::get_if<language::Compability::evaluate::Assignment::BoundsRemapping>(
                       &assign.u);
               boundExprs && !boundExprs->empty()) {
      // Reshape the target according to the LHS bounds remapping.
      toolchain::SmallVector<mlir::Value> lbounds;
      toolchain::SmallVector<mlir::Value> extents;
      mlir::Type indexTy = builder->getIndexType();
      mlir::Value zero = builder->createIntegerConstant(loc, indexTy, 0);
      mlir::Value one = builder->createIntegerConstant(loc, indexTy, 1);
      for (const auto &[lbExpr, ubExpr] : *boundExprs) {
        lbounds.push_back(lowerToIndexValue(loc, lbExpr, rhsContext));
        mlir::Value ub = lowerToIndexValue(loc, ubExpr, rhsContext);
        extents.push_back(fir::factory::computeExtent(
            *builder, loc, lbounds.back(), ub, zero, one));
      }
      mlir::Value shape = builder->genShape(loc, lbounds, extents);
      rhsBox = fir::ReboxOp::create(*builder, loc, lhsBoxType, rhsBox, shape,
                                    /*slice=*/mlir::Value{});
    }
    return rhsBox;
  }

  // Create the 2 x newRank array with the bounds to be passed to the runtime as
  // a descriptor.
  mlir::Value createBoundArray(toolchain::ArrayRef<mlir::Value> lbounds,
                               toolchain::ArrayRef<mlir::Value> ubounds,
                               mlir::Location loc) {
    assert(lbounds.size() && ubounds.size());
    mlir::Type indexTy = builder->getIndexType();
    mlir::Type boundArrayTy = fir::SequenceType::get(
        {2, static_cast<int64_t>(lbounds.size())}, builder->getI64Type());
    mlir::Value boundArray = fir::AllocaOp::create(*builder, loc, boundArrayTy);
    mlir::Value array = fir::UndefOp::create(*builder, loc, boundArrayTy);
    for (unsigned i = 0; i < lbounds.size(); ++i) {
      array = fir::InsertValueOp::create(
          *builder, loc, boundArrayTy, array, lbounds[i],
          builder->getArrayAttr(
              {builder->getIntegerAttr(builder->getIndexType(), 0),
               builder->getIntegerAttr(builder->getIndexType(),
                                       static_cast<int>(i))}));
      array = fir::InsertValueOp::create(
          *builder, loc, boundArrayTy, array, ubounds[i],
          builder->getArrayAttr(
              {builder->getIntegerAttr(builder->getIndexType(), 1),
               builder->getIntegerAttr(builder->getIndexType(),
                                       static_cast<int>(i))}));
    }
    fir::StoreOp::create(*builder, loc, array, boundArray);
    mlir::Type boxTy = fir::BoxType::get(boundArrayTy);
    mlir::Value ext =
        builder->createIntegerConstant(loc, indexTy, lbounds.size());
    mlir::Value c2 = builder->createIntegerConstant(loc, indexTy, 2);
    toolchain::SmallVector<mlir::Value> shapes = {c2, ext};
    mlir::Value shapeOp = builder->genShape(loc, shapes);
    return fir::EmboxOp::create(*builder, loc, boxTy, boundArray, shapeOp);
  }

  // Pointer assignment with bounds-remapping. R1036: a bounds-remapping is a
  // pair, lower bound and upper bound.
  void genPointerAssignment(
      mlir::Location loc, const language::Compability::evaluate::Assignment &assign,
      const language::Compability::evaluate::Assignment::BoundsRemapping &boundExprs) {
    language::Compability::lower::StatementContext stmtCtx;
    toolchain::SmallVector<mlir::Value> lbounds;
    toolchain::SmallVector<mlir::Value> ubounds;
    for (const std::pair<language::Compability::evaluate::ExtentExpr,
                         language::Compability::evaluate::ExtentExpr> &pair : boundExprs) {
      const language::Compability::evaluate::ExtentExpr &lbExpr = pair.first;
      const language::Compability::evaluate::ExtentExpr &ubExpr = pair.second;
      lbounds.push_back(fir::getBase(genExprValue(toEvExpr(lbExpr), stmtCtx)));
      ubounds.push_back(fir::getBase(genExprValue(toEvExpr(ubExpr), stmtCtx)));
    }

    std::optional<language::Compability::evaluate::DynamicType> lhsType =
        assign.lhs.GetType();
    std::optional<language::Compability::evaluate::DynamicType> rhsType =
        assign.rhs.GetType();
    // Polymorphic lhs/rhs need more care. See F2018 10.2.2.3.
    if ((lhsType && lhsType->IsPolymorphic()) ||
        (rhsType && rhsType->IsPolymorphic())) {
      if (!lowerToHighLevelFIR() && explicitIterationSpace())
        TODO(loc, "polymorphic pointer assignment in FORALL");

      fir::MutableBoxValue lhsMutableBox = genExprMutableBox(loc, assign.lhs);
      if (language::Compability::evaluate::UnwrapExpr<language::Compability::evaluate::NullPointer>(
              assign.rhs)) {
        fir::factory::disassociateMutableBox(*builder, loc, lhsMutableBox);
        return;
      }
      mlir::Value lhs = lhsMutableBox.getAddr();
      mlir::Value rhs = fir::getBase(genExprBox(loc, assign.rhs, stmtCtx));
      mlir::Value boundsDesc = createBoundArray(lbounds, ubounds, loc);
      language::Compability::lower::genPointerAssociateRemapping(
          *builder, loc, lhs, rhs, boundsDesc,
          lhsType && rhsType && !lhsType->IsPolymorphic() &&
              rhsType->IsPolymorphic());
      return;
    }
    if (!lowerToHighLevelFIR() && explicitIterationSpace()) {
      // Pointer assignment in FORALL context. Copy the rhs box value
      // into the lhs box variable.
      genArrayAssignment(assign, stmtCtx, lbounds, ubounds);
      return;
    }
    fir::MutableBoxValue lhs = genExprMutableBox(loc, assign.lhs);
    if (language::Compability::evaluate::UnwrapExpr<language::Compability::evaluate::NullPointer>(
            assign.rhs)) {
      fir::factory::disassociateMutableBox(*builder, loc, lhs);
      return;
    }
    if (lowerToHighLevelFIR()) {
      fir::ExtendedValue rhs = genExprAddr(assign.rhs, stmtCtx);
      fir::factory::associateMutableBoxWithRemap(*builder, loc, lhs, rhs,
                                                 lbounds, ubounds);
      return;
    }
    // Legacy lowering below.
    // Do not generate a temp in case rhs is an array section.
    fir::ExtendedValue rhs =
        language::Compability::lower::isArraySectionWithoutVectorSubscript(assign.rhs)
            ? language::Compability::lower::createSomeArrayBox(*this, assign.rhs,
                                                 localSymbols, stmtCtx)
            : genExprAddr(assign.rhs, stmtCtx);
    fir::factory::associateMutableBoxWithRemap(*builder, loc, lhs, rhs, lbounds,
                                               ubounds);
    if (explicitIterationSpace()) {
      mlir::ValueRange inners = explicitIterSpace.getInnerArgs();
      if (!inners.empty())
        fir::ResultOp::create(*builder, loc, inners);
    }
  }

  /// Given converted LHS and RHS of the assignment, materialize any
  /// implicit conversion of the RHS to the LHS type. The front-end
  /// usually already makes those explicit, except for non-standard
  /// LOGICAL <-> INTEGER, or if the LHS is a whole allocatable
  /// (making the conversion explicit in the front-end would prevent
  /// propagation of the LHS lower bound in the reallocation).
  /// If array temporaries or values are created, the cleanups are
  /// added in the statement context.
  hlfir::Entity genImplicitConvert(const language::Compability::evaluate::Assignment &assign,
                                   hlfir::Entity rhs, bool preserveLowerBounds,
                                   language::Compability::lower::StatementContext &stmtCtx) {
    mlir::Location loc = toLocation();
    auto &builder = getFirOpBuilder();
    mlir::Type toType = genType(assign.lhs);
    auto valueAndPair = hlfir::genTypeAndKindConvert(loc, builder, rhs, toType,
                                                     preserveLowerBounds);
    if (valueAndPair.second)
      stmtCtx.attachCleanup(*valueAndPair.second);
    return hlfir::Entity{valueAndPair.first};
  }

  bool firstDummyIsPointerOrAllocatable(
      const language::Compability::evaluate::ProcedureRef &userDefinedAssignment) {
    using DummyAttr = language::Compability::evaluate::characteristics::DummyDataObject::Attr;
    if (auto procedure =
            language::Compability::evaluate::characteristics::Procedure::Characterize(
                userDefinedAssignment.proc(), getFoldingContext(),
                /*emitError=*/false))
      if (!procedure->dummyArguments.empty())
        if (const auto *dataArg = std::get_if<
                language::Compability::evaluate::characteristics::DummyDataObject>(
                &procedure->dummyArguments[0].u))
          return dataArg->attrs.test(DummyAttr::Pointer) ||
                 dataArg->attrs.test(DummyAttr::Allocatable);
    return false;
  }

  void genCUDADataTransfer(fir::FirOpBuilder &builder, mlir::Location loc,
                           const language::Compability::evaluate::Assignment &assign,
                           hlfir::Entity &lhs, hlfir::Entity &rhs,
                           bool isWholeAllocatableAssignment,
                           bool keepLhsLengthInAllocatableAssignment) {
    bool lhsIsDevice = language::Compability::evaluate::HasCUDADeviceAttrs(assign.lhs);
    bool rhsIsDevice = language::Compability::evaluate::HasCUDADeviceAttrs(assign.rhs);

    auto getRefFromValue = [](mlir::Value val) -> mlir::Value {
      if (auto loadOp =
              mlir::dyn_cast_or_null<fir::LoadOp>(val.getDefiningOp()))
        return loadOp.getMemref();
      if (!mlir::isa<fir::BaseBoxType>(val.getType()))
        return val;
      if (auto declOp =
              mlir::dyn_cast_or_null<hlfir::DeclareOp>(val.getDefiningOp())) {
        if (!declOp.getShape())
          return val;
        if (mlir::isa<fir::ReferenceType>(declOp.getMemref().getType()))
          return declOp.getResults()[1];
      }
      return val;
    };

    auto getShapeFromDecl = [](mlir::Value val) -> mlir::Value {
      if (!mlir::isa<fir::BaseBoxType>(val.getType()))
        return {};
      if (auto declOp =
              mlir::dyn_cast_or_null<hlfir::DeclareOp>(val.getDefiningOp()))
        return declOp.getShape();
      return {};
    };

    mlir::Value rhsVal = getRefFromValue(rhs.getBase());
    mlir::Value lhsVal = getRefFromValue(lhs.getBase());
    // Get shape from the rhs if available otherwise get it from lhs.
    mlir::Value shape = getShapeFromDecl(rhs.getBase());
    if (!shape)
      shape = getShapeFromDecl(lhs.getBase());

    // device = host
    if (lhsIsDevice && !rhsIsDevice) {
      auto transferKindAttr = cuf::DataTransferKindAttr::get(
          builder.getContext(), cuf::DataTransferKind::HostDevice);
      if (!rhs.isVariable()) {
        mlir::Value base = rhs;
        if (auto convertOp =
                mlir::dyn_cast<fir::ConvertOp>(rhs.getDefiningOp()))
          base = convertOp.getValue();
        // Special case if the rhs is a constant.
        if (matchPattern(base.getDefiningOp(), mlir::m_Constant())) {
          cuf::DataTransferOp::create(builder, loc, base, lhsVal, shape,
                                      transferKindAttr);
        } else {
          auto associate = hlfir::genAssociateExpr(
              loc, builder, rhs, rhs.getType(), ".cuf_host_tmp");
          cuf::DataTransferOp::create(builder, loc, associate.getBase(), lhsVal,
                                      shape, transferKindAttr);
          hlfir::EndAssociateOp::create(builder, loc, associate);
        }
      } else {
        cuf::DataTransferOp::create(builder, loc, rhsVal, lhsVal, shape,
                                    transferKindAttr);
      }
      return;
    }

    // host = device
    if (!lhsIsDevice && rhsIsDevice) {
      if (language::Compability::lower::isTransferWithConversion(rhs)) {
        mlir::OpBuilder::InsertionGuard insertionGuard(builder);
        auto elementalOp =
            mlir::dyn_cast<hlfir::ElementalOp>(rhs.getDefiningOp());
        assert(elementalOp && "expect elemental op");
        auto designateOp =
            *elementalOp.getBody()->getOps<hlfir::DesignateOp>().begin();
        builder.setInsertionPoint(elementalOp);
        // Create a temp to transfer the rhs before applying the conversion.
        hlfir::Entity entity{designateOp.getMemref()};
        auto [temp, cleanup] = hlfir::createTempFromMold(loc, builder, entity);
        auto transferKindAttr = cuf::DataTransferKindAttr::get(
            builder.getContext(), cuf::DataTransferKind::DeviceHost);
        cuf::DataTransferOp::create(builder, loc, designateOp.getMemref(), temp,
                                    /*shape=*/mlir::Value{}, transferKindAttr);
        designateOp.getMemrefMutable().assign(temp);
        builder.setInsertionPointAfter(elementalOp);
        hlfir::AssignOp::create(builder, loc, elementalOp, lhs,
                                isWholeAllocatableAssignment,
                                keepLhsLengthInAllocatableAssignment);
        return;
      }
      auto transferKindAttr = cuf::DataTransferKindAttr::get(
          builder.getContext(), cuf::DataTransferKind::DeviceHost);
      cuf::DataTransferOp::create(builder, loc, rhsVal, lhsVal, shape,
                                  transferKindAttr);
      return;
    }

    // device = device
    if (lhsIsDevice && rhsIsDevice) {
      auto transferKindAttr = cuf::DataTransferKindAttr::get(
          builder.getContext(), cuf::DataTransferKind::DeviceDevice);
      cuf::DataTransferOp::create(builder, loc, rhsVal, lhsVal, shape,
                                  transferKindAttr);
      return;
    }
    toolchain_unreachable("Unhandled CUDA data transfer");
  }

  toolchain::SmallVector<mlir::Value>
  genCUDAImplicitDataTransfer(fir::FirOpBuilder &builder, mlir::Location loc,
                              const language::Compability::evaluate::Assignment &assign) {
    toolchain::SmallVector<mlir::Value> temps;
    localSymbols.pushScope();
    auto transferKindAttr = cuf::DataTransferKindAttr::get(
        builder.getContext(), cuf::DataTransferKind::DeviceHost);
    [[maybe_unused]] unsigned nbDeviceResidentObject = 0;
    for (const language::Compability::semantics::Symbol &sym :
         language::Compability::evaluate::CollectSymbols(assign.rhs)) {
      if (const auto *details =
              sym.GetUltimate()
                  .detailsIf<language::Compability::semantics::ObjectEntityDetails>()) {
        if (details->cudaDataAttr() &&
            *details->cudaDataAttr() != language::Compability::common::CUDADataAttr::Pinned) {
          assert(
              nbDeviceResidentObject <= 1 &&
              "Only one reference to the device resident object is supported");
          auto addr = getSymbolAddress(sym);
          mlir::Value baseValue;
          if (auto declareOp =
                  toolchain::dyn_cast<hlfir::DeclareOp>(addr.getDefiningOp()))
            baseValue = declareOp.getBase();
          else
            baseValue = addr;

          hlfir::Entity entity{baseValue};
          auto [temp, cleanup] =
              hlfir::createTempFromMold(loc, builder, entity);
          auto needCleanup = fir::getIntIfConstant(cleanup);
          if (needCleanup && *needCleanup) {
            if (auto declareOp =
                    mlir::dyn_cast<hlfir::DeclareOp>(temp.getDefiningOp()))
              temps.push_back(declareOp.getMemref());
            else
              temps.push_back(temp);
          }
          addSymbol(sym,
                    hlfir::translateToExtendedValue(loc, builder, temp).first,
                    /*forced=*/true);
          cuf::DataTransferOp::create(builder, loc, addr, temp,
                                      /*shape=*/mlir::Value{},
                                      transferKindAttr);
          ++nbDeviceResidentObject;
        }
      }
    }
    return temps;
  }

  void genDataAssignment(
      const language::Compability::evaluate::Assignment &assign,
      const language::Compability::evaluate::ProcedureRef *userDefinedAssignment) {
    mlir::Location loc = getCurrentLocation();
    fir::FirOpBuilder &builder = getFirOpBuilder();

    bool isInDeviceContext = cuf::isCUDADeviceContext(
        builder.getRegion(),
        getFoldingContext().languageFeatures().IsEnabled(
            language::Compability::common::LanguageFeature::DoConcurrentOffload));

    bool isCUDATransfer =
        IsCUDADataTransfer(assign.lhs, assign.rhs) && !isInDeviceContext;
    bool hasCUDAImplicitTransfer =
        isCUDATransfer &&
        language::Compability::evaluate::HasCUDAImplicitTransfer(assign.rhs);
    toolchain::SmallVector<mlir::Value> implicitTemps;

    if (hasCUDAImplicitTransfer && !isInDeviceContext)
      implicitTemps = genCUDAImplicitDataTransfer(builder, loc, assign);

    // Gather some information about the assignment that will impact how it is
    // lowered.
    const bool isWholeAllocatableAssignment =
        !userDefinedAssignment && !isInsideHlfirWhere() &&
        language::Compability::lower::isWholeAllocatable(assign.lhs) &&
        bridge.getLoweringOptions().getReallocateLHS();
    const bool isUserDefAssignToPointerOrAllocatable =
        userDefinedAssignment &&
        firstDummyIsPointerOrAllocatable(*userDefinedAssignment);
    std::optional<language::Compability::evaluate::DynamicType> lhsType =
        assign.lhs.GetType();
    const bool keepLhsLengthInAllocatableAssignment =
        isWholeAllocatableAssignment && lhsType.has_value() &&
        lhsType->category() == language::Compability::common::TypeCategory::Character &&
        !lhsType->HasDeferredTypeParameter();
    const bool lhsHasVectorSubscripts =
        language::Compability::evaluate::HasVectorSubscript(assign.lhs);

    // Helper to generate the code evaluating the right-hand side.
    auto evaluateRhs = [&](language::Compability::lower::StatementContext &stmtCtx) {
      hlfir::Entity rhs = language::Compability::lower::convertExprToHLFIR(
          loc, *this, assign.rhs, localSymbols, stmtCtx);
      // Load trivial scalar RHS to allow the loads to be hoisted outside of
      // loops early if possible. This also dereferences pointer and
      // allocatable RHS: the target is being assigned from.
      rhs = hlfir::loadTrivialScalar(loc, builder, rhs);
      // In intrinsic assignments, the LHS type may not match the RHS type, in
      // which case an implicit conversion of the LHS must be done. The
      // front-end usually makes it explicit, unless it cannot (whole
      // allocatable LHS or Logical<->Integer assignment extension). Recognize
      // any type mismatches here and insert explicit scalar convert or
      // ElementalOp for array assignment. Preserve the RHS lower bounds on the
      // converted entity in case of assignment to whole allocatables so to
      // propagate the lower bounds to the LHS in case of reallocation.
      if (!userDefinedAssignment)
        rhs = genImplicitConvert(assign, rhs, isWholeAllocatableAssignment,
                                 stmtCtx);
      return rhs;
    };

    // Helper to generate the code evaluating the left-hand side.
    auto evaluateLhs = [&](language::Compability::lower::StatementContext &stmtCtx) {
      hlfir::Entity lhs = language::Compability::lower::convertExprToHLFIR(
          loc, *this, assign.lhs, localSymbols, stmtCtx);
      // Dereference pointer LHS: the target is being assigned to.
      // Same for allocatables outside of whole allocatable assignments.
      if (!isWholeAllocatableAssignment &&
          !isUserDefAssignToPointerOrAllocatable)
        lhs = hlfir::derefPointersAndAllocatables(loc, builder, lhs);
      return lhs;
    };

    if (!isInsideHlfirForallOrWhere() && !lhsHasVectorSubscripts &&
        !userDefinedAssignment) {
      language::Compability::lower::StatementContext localStmtCtx;
      hlfir::Entity rhs = evaluateRhs(localStmtCtx);
      hlfir::Entity lhs = evaluateLhs(localStmtCtx);
      if (isCUDATransfer && !hasCUDAImplicitTransfer)
        genCUDADataTransfer(builder, loc, assign, lhs, rhs,
                            isWholeAllocatableAssignment,
                            keepLhsLengthInAllocatableAssignment);
      else
        hlfir::AssignOp::create(builder, loc, rhs, lhs,
                                isWholeAllocatableAssignment,
                                keepLhsLengthInAllocatableAssignment);
      if (hasCUDAImplicitTransfer && !isInDeviceContext) {
        localSymbols.popScope();
        for (mlir::Value temp : implicitTemps)
          fir::FreeMemOp::create(builder, loc, temp);
      }
      return;
    }
    // Assignments inside Forall, Where, or assignments to a vector subscripted
    // left-hand side requires using an hlfir.region_assign in HLFIR. The
    // right-hand side and left-hand side must be evaluated inside the
    // hlfir.region_assign regions.
    auto regionAssignOp = hlfir::RegionAssignOp::create(builder, loc);

    // Lower RHS in its own region.
    builder.createBlock(&regionAssignOp.getRhsRegion());
    language::Compability::lower::StatementContext rhsContext;
    hlfir::Entity rhs = evaluateRhs(rhsContext);
    auto rhsYieldOp = hlfir::YieldOp::create(builder, loc, rhs);
    language::Compability::lower::genCleanUpInRegionIfAny(
        loc, builder, rhsYieldOp.getCleanup(), rhsContext);
    // Lower LHS in its own region.
    builder.createBlock(&regionAssignOp.getLhsRegion());
    language::Compability::lower::StatementContext lhsContext;
    mlir::Value lhsYield = nullptr;
    if (!lhsHasVectorSubscripts) {
      hlfir::Entity lhs = evaluateLhs(lhsContext);
      auto lhsYieldOp = hlfir::YieldOp::create(builder, loc, lhs);
      language::Compability::lower::genCleanUpInRegionIfAny(
          loc, builder, lhsYieldOp.getCleanup(), lhsContext);
      lhsYield = lhs;
    } else {
      hlfir::ElementalAddrOp elementalAddr =
          language::Compability::lower::convertVectorSubscriptedExprToElementalAddr(
              loc, *this, assign.lhs, localSymbols, lhsContext);
      language::Compability::lower::genCleanUpInRegionIfAny(
          loc, builder, elementalAddr.getCleanup(), lhsContext);
      lhsYield = elementalAddr.getYieldOp().getEntity();
    }
    assert(lhsYield && "must have been set");

    // Add "realloc" flag to hlfir.region_assign.
    if (isWholeAllocatableAssignment)
      TODO(loc, "assignment to a whole allocatable inside FORALL");

    // Generate the hlfir.region_assign userDefinedAssignment region.
    if (userDefinedAssignment) {
      mlir::Type rhsType = rhs.getType();
      mlir::Type lhsType = lhsYield.getType();
      if (userDefinedAssignment->IsElemental()) {
        rhsType = hlfir::getEntityElementType(rhs);
        lhsType = hlfir::getEntityElementType(hlfir::Entity{lhsYield});
      }
      builder.createBlock(&regionAssignOp.getUserDefinedAssignment(),
                          mlir::Region::iterator{}, {rhsType, lhsType},
                          {loc, loc});
      auto end = fir::FirEndOp::create(builder, loc);
      builder.setInsertionPoint(end);
      hlfir::Entity lhsBlockArg{regionAssignOp.getUserAssignmentLhs()};
      hlfir::Entity rhsBlockArg{regionAssignOp.getUserAssignmentRhs()};
      language::Compability::lower::convertUserDefinedAssignmentToHLFIR(
          loc, *this, *userDefinedAssignment, lhsBlockArg, rhsBlockArg,
          localSymbols);
    }
    builder.setInsertionPointAfter(regionAssignOp);
  }

  /// Shared for both assignments and pointer assignments.
  void genAssignment(const language::Compability::evaluate::Assignment &assign) {
    mlir::Location loc = toLocation();
    if (lowerToHighLevelFIR()) {
      language::Compability::common::visit(
          language::Compability::common::visitors{
              [&](const language::Compability::evaluate::Assignment::Intrinsic &) {
                genDataAssignment(assign, /*userDefinedAssignment=*/nullptr);
              },
              [&](const language::Compability::evaluate::ProcedureRef &procRef) {
                genDataAssignment(assign, /*userDefinedAssignment=*/&procRef);
              },
              [&](const language::Compability::evaluate::Assignment::BoundsSpec &lbExprs) {
                if (isInsideHlfirForallOrWhere())
                  genForallPointerAssignment(loc, assign);
                else
                  genPointerAssignment(loc, assign, lbExprs);
              },
              [&](const language::Compability::evaluate::Assignment::BoundsRemapping
                      &boundExprs) {
                if (isInsideHlfirForallOrWhere())
                  genForallPointerAssignment(loc, assign);
                else
                  genPointerAssignment(loc, assign, boundExprs);
              },
          },
          assign.u);
      return;
    }
    if (explicitIterationSpace()) {
      language::Compability::lower::createArrayLoads(*this, explicitIterSpace, localSymbols);
      explicitIterSpace.genLoopNest();
    }
    language::Compability::lower::StatementContext stmtCtx;
    language::Compability::common::visit(
        language::Compability::common::visitors{
            // [1] Plain old assignment.
            [&](const language::Compability::evaluate::Assignment::Intrinsic &) {
              const language::Compability::semantics::Symbol *sym =
                  language::Compability::evaluate::GetLastSymbol(assign.lhs);

              if (!sym)
                TODO(loc, "assignment to pointer result of function reference");

              std::optional<language::Compability::evaluate::DynamicType> lhsType =
                  assign.lhs.GetType();
              assert(lhsType && "lhs cannot be typeless");
              std::optional<language::Compability::evaluate::DynamicType> rhsType =
                  assign.rhs.GetType();

              // Assignment to/from polymorphic entities are done with the
              // runtime.
              if (lhsType->IsPolymorphic() ||
                  lhsType->IsUnlimitedPolymorphic() ||
                  (rhsType && (rhsType->IsPolymorphic() ||
                               rhsType->IsUnlimitedPolymorphic()))) {
                mlir::Value lhs;
                if (language::Compability::lower::isWholeAllocatable(assign.lhs))
                  lhs = genExprMutableBox(loc, assign.lhs).getAddr();
                else
                  lhs = fir::getBase(genExprBox(loc, assign.lhs, stmtCtx));
                mlir::Value rhs =
                    fir::getBase(genExprBox(loc, assign.rhs, stmtCtx));
                if ((lhsType->IsPolymorphic() ||
                     lhsType->IsUnlimitedPolymorphic()) &&
                    language::Compability::lower::isWholeAllocatable(assign.lhs))
                  fir::runtime::genAssignPolymorphic(*builder, loc, lhs, rhs);
                else
                  fir::runtime::genAssign(*builder, loc, lhs, rhs);
                return;
              }

              // Note: No ad-hoc handling for pointers is required here. The
              // target will be assigned as per 2018 10.2.1.3 p2. genExprAddr
              // on a pointer returns the target address and not the address of
              // the pointer variable.

              if (assign.lhs.Rank() > 0 || explicitIterationSpace()) {
                if (isDerivedCategory(lhsType->category()) &&
                    language::Compability::semantics::IsFinalizable(
                        lhsType->GetDerivedTypeSpec()))
                  TODO(loc, "derived-type finalization with array assignment");
                // Array assignment
                // See Fortran 2018 10.2.1.3 p5, p6, and p7
                genArrayAssignment(assign, stmtCtx);
                return;
              }

              // Scalar assignment
              const bool isNumericScalar =
                  isNumericScalarCategory(lhsType->category());
              const bool isVector =
                  isDerivedCategory(lhsType->category()) &&
                  lhsType->GetDerivedTypeSpec().IsVectorType();
              fir::ExtendedValue rhs = (isNumericScalar || isVector)
                                           ? genExprValue(assign.rhs, stmtCtx)
                                           : genExprAddr(assign.rhs, stmtCtx);
              const bool lhsIsWholeAllocatable =
                  language::Compability::lower::isWholeAllocatable(assign.lhs);
              std::optional<fir::factory::MutableBoxReallocation> lhsRealloc;
              std::optional<fir::MutableBoxValue> lhsMutableBox;

              // Set flag to know if the LHS needs finalization. Polymorphic,
              // unlimited polymorphic assignment will be done with genAssign.
              // Assign runtime function performs the finalization.
              bool needFinalization = !lhsType->IsPolymorphic() &&
                                      !lhsType->IsUnlimitedPolymorphic() &&
                                      (isDerivedCategory(lhsType->category()) &&
                                       language::Compability::semantics::IsFinalizable(
                                           lhsType->GetDerivedTypeSpec()));

              auto lhs = [&]() -> fir::ExtendedValue {
                if (lhsIsWholeAllocatable) {
                  lhsMutableBox = genExprMutableBox(loc, assign.lhs);
                  // Finalize if needed.
                  if (needFinalization) {
                    mlir::Value isAllocated =
                        fir::factory::genIsAllocatedOrAssociatedTest(
                            *builder, loc, *lhsMutableBox);
                    builder->genIfThen(loc, isAllocated)
                        .genThen([&]() {
                          fir::runtime::genDerivedTypeDestroy(
                              *builder, loc, fir::getBase(*lhsMutableBox));
                        })
                        .end();
                    needFinalization = false;
                  }

                  toolchain::SmallVector<mlir::Value> lengthParams;
                  if (const fir::CharBoxValue *charBox = rhs.getCharBox())
                    lengthParams.push_back(charBox->getLen());
                  else if (fir::isDerivedWithLenParameters(rhs))
                    TODO(loc, "assignment to derived type allocatable with "
                              "LEN parameters");
                  lhsRealloc = fir::factory::genReallocIfNeeded(
                      *builder, loc, *lhsMutableBox,
                      /*shape=*/{}, lengthParams);
                  return lhsRealloc->newValue;
                }
                return genExprAddr(assign.lhs, stmtCtx);
              }();

              if (isNumericScalar || isVector) {
                // Fortran 2018 10.2.1.3 p8 and p9
                // Conversions should have been inserted by semantic analysis,
                // but they can be incorrect between the rhs and lhs. Correct
                // that here.
                mlir::Value addr = fir::getBase(lhs);
                mlir::Value val = fir::getBase(rhs);
                // A function with multiple entry points returning different
                // types tags all result variables with one of the largest
                // types to allow them to share the same storage. Assignment
                // to a result variable of one of the other types requires
                // conversion to the actual type.
                mlir::Type toTy = genType(assign.lhs);

                // If Cray pointee, need to handle the address
                // Array is handled in genCoordinateOp.
                if (sym->test(language::Compability::semantics::Symbol::Flag::CrayPointee) &&
                    sym->Rank() == 0) {
                  // get the corresponding Cray pointer

                  const language::Compability::semantics::Symbol &ptrSym =
                      language::Compability::semantics::GetCrayPointer(*sym);
                  fir::ExtendedValue ptr =
                      getSymbolExtendedValue(ptrSym, nullptr);
                  mlir::Value ptrVal = fir::getBase(ptr);
                  mlir::Type ptrTy = genType(ptrSym);

                  fir::ExtendedValue pte =
                      getSymbolExtendedValue(*sym, nullptr);
                  mlir::Value pteVal = fir::getBase(pte);
                  mlir::Value cnvrt = language::Compability::lower::addCrayPointerInst(
                      loc, *builder, ptrVal, ptrTy, pteVal.getType());
                  addr = fir::LoadOp::create(*builder, loc, cnvrt);
                }
                mlir::Value cast =
                    isVector ? val
                             : builder->convertWithSemantics(loc, toTy, val);
                if (fir::dyn_cast_ptrEleTy(addr.getType()) != toTy) {
                  assert(isFuncResultDesignator(assign.lhs) && "type mismatch");
                  addr = builder->createConvert(
                      toLocation(), builder->getRefType(toTy), addr);
                }
                fir::StoreOp::create(*builder, loc, cast, addr);
              } else if (isCharacterCategory(lhsType->category())) {
                // Fortran 2018 10.2.1.3 p10 and p11
                fir::factory::CharacterExprHelper{*builder, loc}.createAssign(
                    lhs, rhs);
              } else if (isDerivedCategory(lhsType->category())) {
                // Handle parent component.
                if (language::Compability::lower::isParentComponent(assign.lhs)) {
                  if (!mlir::isa<fir::BaseBoxType>(fir::getBase(lhs).getType()))
                    lhs = fir::getBase(builder->createBox(loc, lhs));
                  lhs = language::Compability::lower::updateBoxForParentComponent(*this, lhs,
                                                                    assign.lhs);
                }

                // Fortran 2018 10.2.1.3 p13 and p14
                // Recursively gen an assignment on each element pair.
                fir::factory::genRecordAssignment(*builder, loc, lhs, rhs,
                                                  needFinalization);
              } else {
                toolchain_unreachable("unknown category");
              }
              if (lhsIsWholeAllocatable) {
                assert(lhsRealloc.has_value());
                fir::factory::finalizeRealloc(*builder, loc, *lhsMutableBox,
                                              /*lbounds=*/{},
                                              /*takeLboundsIfRealloc=*/false,
                                              *lhsRealloc);
              }
            },

            // [2] User defined assignment. If the context is a scalar
            // expression then call the procedure.
            [&](const language::Compability::evaluate::ProcedureRef &procRef) {
              language::Compability::lower::StatementContext &ctx =
                  explicitIterationSpace() ? explicitIterSpace.stmtContext()
                                           : stmtCtx;
              language::Compability::lower::createSubroutineCall(
                  *this, procRef, explicitIterSpace, implicitIterSpace,
                  localSymbols, ctx, /*isUserDefAssignment=*/true);
            },

            [&](const language::Compability::evaluate::Assignment::BoundsSpec &lbExprs) {
              return genPointerAssignment(loc, assign, lbExprs);
            },
            [&](const language::Compability::evaluate::Assignment::BoundsRemapping
                    &boundExprs) {
              return genPointerAssignment(loc, assign, boundExprs);
            },
        },
        assign.u);
    if (explicitIterationSpace())
      language::Compability::lower::createArrayMergeStores(*this, explicitIterSpace);
  }

  // Is the insertion point of the builder directly or indirectly set
  // inside any operation of type "Op"?
  template <typename... Op>
  bool isInsideOp() const {
    mlir::Block *block = builder->getInsertionBlock();
    mlir::Operation *op = block ? block->getParentOp() : nullptr;
    while (op) {
      if (mlir::isa<Op...>(op))
        return true;
      op = op->getParentOp();
    }
    return false;
  }
  bool isInsideHlfirForallOrWhere() const {
    return isInsideOp<hlfir::ForallOp, hlfir::WhereOp>();
  }
  bool isInsideHlfirWhere() const { return isInsideOp<hlfir::WhereOp>(); }

  void genFIR(const language::Compability::parser::WhereConstruct &c) {
    mlir::Location loc = getCurrentLocation();
    hlfir::WhereOp whereOp;

    if (!lowerToHighLevelFIR()) {
      implicitIterSpace.growStack();
    } else {
      whereOp = hlfir::WhereOp::create(*builder, loc);
      builder->createBlock(&whereOp.getMaskRegion());
    }

    // Lower the where mask. For HLFIR, this is done in the hlfir.where mask
    // region.
    genNestedStatement(
        std::get<
            language::Compability::parser::Statement<language::Compability::parser::WhereConstructStmt>>(
            c.t));

    // Lower WHERE body. For HLFIR, this is done in the hlfir.where body
    // region.
    if (whereOp)
      builder->createBlock(&whereOp.getBody());

    for (const auto &body :
         std::get<std::list<language::Compability::parser::WhereBodyConstruct>>(c.t))
      genFIR(body);
    for (const auto &e :
         std::get<std::list<language::Compability::parser::WhereConstruct::MaskedElsewhere>>(
             c.t))
      genFIR(e);
    if (const auto &e =
            std::get<std::optional<language::Compability::parser::WhereConstruct::Elsewhere>>(
                c.t);
        e.has_value())
      genFIR(*e);
    genNestedStatement(
        std::get<language::Compability::parser::Statement<language::Compability::parser::EndWhereStmt>>(
            c.t));

    if (whereOp) {
      // For HLFIR, create fir.end terminator in the last hlfir.elsewhere, or
      // in the hlfir.where if it had no elsewhere.
      fir::FirEndOp::create(*builder, loc);
      builder->setInsertionPointAfter(whereOp);
    }
  }
  void genFIR(const language::Compability::parser::WhereBodyConstruct &body) {
    language::Compability::common::visit(
        language::Compability::common::visitors{
            [&](const language::Compability::parser::Statement<
                language::Compability::parser::AssignmentStmt> &stmt) {
              genNestedStatement(stmt);
            },
            [&](const language::Compability::parser::Statement<language::Compability::parser::WhereStmt>
                    &stmt) { genNestedStatement(stmt); },
            [&](const language::Compability::common::Indirection<
                language::Compability::parser::WhereConstruct> &c) { genFIR(c.value()); },
        },
        body.u);
  }

  /// Lower a Where or Elsewhere mask into an hlfir mask region.
  void lowerWhereMaskToHlfir(mlir::Location loc,
                             const language::Compability::semantics::SomeExpr *maskExpr) {
    assert(maskExpr && "mask semantic analysis failed");
    language::Compability::lower::StatementContext maskContext;
    hlfir::Entity mask = language::Compability::lower::convertExprToHLFIR(
        loc, *this, *maskExpr, localSymbols, maskContext);
    mask = hlfir::loadTrivialScalar(loc, *builder, mask);
    auto yieldOp = hlfir::YieldOp::create(*builder, loc, mask);
    language::Compability::lower::genCleanUpInRegionIfAny(loc, *builder, yieldOp.getCleanup(),
                                            maskContext);
  }
  void genFIR(const language::Compability::parser::WhereConstructStmt &stmt) {
    const language::Compability::semantics::SomeExpr *maskExpr = language::Compability::semantics::GetExpr(
        std::get<language::Compability::parser::LogicalExpr>(stmt.t));
    if (lowerToHighLevelFIR())
      lowerWhereMaskToHlfir(getCurrentLocation(), maskExpr);
    else
      implicitIterSpace.append(maskExpr);
  }
  void genFIR(const language::Compability::parser::WhereConstruct::MaskedElsewhere &ew) {
    mlir::Location loc = getCurrentLocation();
    hlfir::ElseWhereOp elsewhereOp;
    if (lowerToHighLevelFIR()) {
      elsewhereOp = hlfir::ElseWhereOp::create(*builder, loc);
      // Lower mask in the mask region.
      builder->createBlock(&elsewhereOp.getMaskRegion());
    }
    genNestedStatement(
        std::get<
            language::Compability::parser::Statement<language::Compability::parser::MaskedElsewhereStmt>>(
            ew.t));

    // For HLFIR, lower the body in the hlfir.elsewhere body region.
    if (elsewhereOp)
      builder->createBlock(&elsewhereOp.getBody());

    for (const auto &body :
         std::get<std::list<language::Compability::parser::WhereBodyConstruct>>(ew.t))
      genFIR(body);
  }
  void genFIR(const language::Compability::parser::MaskedElsewhereStmt &stmt) {
    const auto *maskExpr = language::Compability::semantics::GetExpr(
        std::get<language::Compability::parser::LogicalExpr>(stmt.t));
    if (lowerToHighLevelFIR())
      lowerWhereMaskToHlfir(getCurrentLocation(), maskExpr);
    else
      implicitIterSpace.append(maskExpr);
  }
  void genFIR(const language::Compability::parser::WhereConstruct::Elsewhere &ew) {
    if (lowerToHighLevelFIR()) {
      auto elsewhereOp =
          hlfir::ElseWhereOp::create(*builder, getCurrentLocation());
      builder->createBlock(&elsewhereOp.getBody());
    }
    genNestedStatement(
        std::get<language::Compability::parser::Statement<language::Compability::parser::ElsewhereStmt>>(
            ew.t));
    for (const auto &body :
         std::get<std::list<language::Compability::parser::WhereBodyConstruct>>(ew.t))
      genFIR(body);
  }
  void genFIR(const language::Compability::parser::ElsewhereStmt &stmt) {
    if (!lowerToHighLevelFIR())
      implicitIterSpace.append(nullptr);
  }
  void genFIR(const language::Compability::parser::EndWhereStmt &) {
    if (!lowerToHighLevelFIR())
      implicitIterSpace.shrinkStack();
  }

  void genFIR(const language::Compability::parser::WhereStmt &stmt) {
    language::Compability::lower::StatementContext stmtCtx;
    const auto &assign = std::get<language::Compability::parser::AssignmentStmt>(stmt.t);
    const auto *mask = language::Compability::semantics::GetExpr(
        std::get<language::Compability::parser::LogicalExpr>(stmt.t));
    if (lowerToHighLevelFIR()) {
      mlir::Location loc = getCurrentLocation();
      auto whereOp = hlfir::WhereOp::create(*builder, loc);
      builder->createBlock(&whereOp.getMaskRegion());
      lowerWhereMaskToHlfir(loc, mask);
      builder->createBlock(&whereOp.getBody());
      genAssignment(*assign.typedAssignment->v);
      fir::FirEndOp::create(*builder, loc);
      builder->setInsertionPointAfter(whereOp);
      return;
    }
    implicitIterSpace.growStack();
    implicitIterSpace.append(mask);
    genAssignment(*assign.typedAssignment->v);
    implicitIterSpace.shrinkStack();
  }

  void genFIR(const language::Compability::parser::PointerAssignmentStmt &stmt) {
    genAssignment(*stmt.typedAssignment->v);
  }

  void genFIR(const language::Compability::parser::AssignmentStmt &stmt) {
    genAssignment(*stmt.typedAssignment->v);
  }

  void genFIR(const language::Compability::parser::SyncAllStmt &stmt) {
    genSyncAllStatement(*this, stmt);
  }

  void genFIR(const language::Compability::parser::SyncImagesStmt &stmt) {
    genSyncImagesStatement(*this, stmt);
  }

  void genFIR(const language::Compability::parser::SyncMemoryStmt &stmt) {
    genSyncMemoryStatement(*this, stmt);
  }

  void genFIR(const language::Compability::parser::SyncTeamStmt &stmt) {
    genSyncTeamStatement(*this, stmt);
  }

  void genFIR(const language::Compability::parser::UnlockStmt &stmt) {
    genUnlockStatement(*this, stmt);
  }

  void genFIR(const language::Compability::parser::AssignStmt &stmt) {
    const language::Compability::semantics::Symbol &symbol =
        *std::get<language::Compability::parser::Name>(stmt.t).symbol;

    mlir::Location loc = toLocation();
    mlir::Type symbolType = genType(symbol);
    mlir::Value addr = getSymbolAddress(symbol);

    // Handle the case where the assigned variable is declared as a pointer
    if (auto eleTy = fir::dyn_cast_ptrOrBoxEleTy(symbolType)) {
      if (auto ptrType = mlir::dyn_cast<fir::PointerType>(eleTy)) {
        symbolType = ptrType.getEleTy();
      } else {
        symbolType = eleTy;
      }
    } else if (auto ptrType = mlir::dyn_cast<fir::PointerType>(symbolType)) {
      symbolType = ptrType.getEleTy();
    }

    mlir::Value labelValue = builder->createIntegerConstant(
        loc, symbolType, std::get<language::Compability::parser::Label>(stmt.t));

    // If the address points to a boxed pointer, we need to dereference it
    if (auto refType = mlir::dyn_cast<fir::ReferenceType>(addr.getType())) {
      if (auto boxType = mlir::dyn_cast<fir::BoxType>(refType.getEleTy())) {
        mlir::Value boxValue = fir::LoadOp::create(*builder, loc, addr);
        addr = fir::BoxAddrOp::create(*builder, loc, boxValue);
      }
    }

    fir::StoreOp::create(*builder, loc, labelValue, addr);
  }

  void genFIR(const language::Compability::parser::FormatStmt &) {
    // do nothing.

    // FORMAT statements have no semantics. They may be lowered if used by a
    // data transfer statement.
  }

  void genFIR(const language::Compability::parser::PauseStmt &stmt) {
    genPauseStatement(*this, stmt);
  }

  // call FAIL IMAGE in runtime
  void genFIR(const language::Compability::parser::FailImageStmt &stmt) {
    genFailImageStatement(*this);
  }

  // call STOP, ERROR STOP in runtime
  void genFIR(const language::Compability::parser::StopStmt &stmt) {
    genStopStatement(*this, stmt);
  }

  void genFIR(const language::Compability::parser::ReturnStmt &stmt) {
    language::Compability::lower::pft::FunctionLikeUnit *funit =
        getEval().getOwningProcedure();
    assert(funit && "not inside main program, function or subroutine");
    for (auto it = activeConstructStack.rbegin(),
              rend = activeConstructStack.rend();
         it != rend; ++it) {
      it->stmtCtx.finalizeAndKeep();
    }
    if (funit->isMainProgram()) {
      genExitRoutine(true);
      return;
    }
    mlir::Location loc = toLocation();
    if (stmt.v) {
      // Alternate return statement - If this is a subroutine where some
      // alternate entries have alternate returns, but the active entry point
      // does not, ignore the alternate return value. Otherwise, assign it
      // to the compiler-generated result variable.
      const language::Compability::semantics::Symbol &symbol = funit->getSubprogramSymbol();
      if (language::Compability::semantics::HasAlternateReturns(symbol)) {
        language::Compability::lower::StatementContext stmtCtx;
        const language::Compability::lower::SomeExpr *expr =
            language::Compability::semantics::GetExpr(*stmt.v);
        assert(expr && "missing alternate return expression");
        mlir::Value altReturnIndex = builder->createConvert(
            loc, builder->getIndexType(), createFIRExpr(loc, expr, stmtCtx));
        fir::StoreOp::create(*builder, loc, altReturnIndex,
                             getAltReturnResult(symbol));
      }
    }
    // Branch to the last block of the SUBROUTINE, which has the actual return.
    if (!funit->finalBlock) {
      mlir::OpBuilder::InsertPoint insPt = builder->saveInsertionPoint();
      language::Compability::lower::setInsertionPointAfterOpenACCLoopIfInside(*builder);
      funit->finalBlock = builder->createBlock(&builder->getRegion());
      builder->restoreInsertionPoint(insPt);
    }

    if (language::Compability::lower::isInOpenACCLoop(*builder))
      language::Compability::lower::genEarlyReturnInOpenACCLoop(*builder, loc);
    else
      mlir::cf::BranchOp::create(*builder, loc, funit->finalBlock);
  }

  void genFIR(const language::Compability::parser::CycleStmt &) {
    genConstructExitBranch(*getEval().controlSuccessor);
  }
  void genFIR(const language::Compability::parser::ExitStmt &) {
    genConstructExitBranch(*getEval().controlSuccessor);
  }
  void genFIR(const language::Compability::parser::GotoStmt &) {
    genConstructExitBranch(*getEval().controlSuccessor);
  }

  // Nop statements - No code, or code is generated at the construct level.
  // But note that the genFIR call immediately below that wraps one of these
  // calls does block management, possibly starting a new block, and possibly
  // generating a branch to end a block. So these calls may still be required
  // for that functionality.
  void genFIR(const language::Compability::parser::AssociateStmt &) {}       // nop
  void genFIR(const language::Compability::parser::BlockStmt &) {}           // nop
  void genFIR(const language::Compability::parser::CaseStmt &) {}            // nop
  void genFIR(const language::Compability::parser::ContinueStmt &) {}        // nop
  void genFIR(const language::Compability::parser::ElseIfStmt &) {}          // nop
  void genFIR(const language::Compability::parser::ElseStmt &) {}            // nop
  void genFIR(const language::Compability::parser::EndAssociateStmt &) {}    // nop
  void genFIR(const language::Compability::parser::EndBlockStmt &) {}        // nop
  void genFIR(const language::Compability::parser::EndDoStmt &) {}           // nop
  void genFIR(const language::Compability::parser::EndFunctionStmt &) {}     // nop
  void genFIR(const language::Compability::parser::EndIfStmt &) {}           // nop
  void genFIR(const language::Compability::parser::EndMpSubprogramStmt &) {} // nop
  void genFIR(const language::Compability::parser::EndProgramStmt &) {}      // nop
  void genFIR(const language::Compability::parser::EndSelectStmt &) {}       // nop
  void genFIR(const language::Compability::parser::EndSubroutineStmt &) {}   // nop
  void genFIR(const language::Compability::parser::EntryStmt &) {}           // nop
  void genFIR(const language::Compability::parser::IfStmt &) {}              // nop
  void genFIR(const language::Compability::parser::IfThenStmt &) {}          // nop
  void genFIR(const language::Compability::parser::NonLabelDoStmt &) {}      // nop
  void genFIR(const language::Compability::parser::OmpEndLoopDirective &) {} // nop
  void genFIR(const language::Compability::parser::SelectTypeStmt &) {}      // nop
  void genFIR(const language::Compability::parser::TypeGuardStmt &) {}       // nop

  /// Generate FIR for Evaluation \p eval.
  void genFIR(language::Compability::lower::pft::Evaluation &eval,
              bool unstructuredContext = true) {
    // Start a new unstructured block when applicable. When transitioning
    // from unstructured to structured code, unstructuredContext is true,
    // which accounts for the possibility that the structured code could be
    // a target that starts a new block.
    if (unstructuredContext)
      maybeStartBlock(eval.isConstruct() && eval.lowerAsStructured()
                          ? eval.getFirstNestedEvaluation().block
                          : eval.block);

    // Generate evaluation specific code. Even nop calls should usually reach
    // here in case they start a new block or require generation of a generic
    // end-of-block branch. An alternative is to add special case code
    // elsewhere, such as in the genFIR code for a parent construct.
    setCurrentEval(eval);
    setCurrentPosition(eval.position);
    eval.visit([&](const auto &stmt) { genFIR(stmt); });
  }

  /// Map mlir function block arguments to the corresponding Fortran dummy
  /// variables. When the result is passed as a hidden argument, the Fortran
  /// result is also mapped. The symbol map is used to hold this mapping.
  void mapDummiesAndResults(language::Compability::lower::pft::FunctionLikeUnit &funit,
                            const language::Compability::lower::CalleeInterface &callee) {
    assert(builder && "require a builder object at this point");
    using PassBy = language::Compability::lower::CalleeInterface::PassEntityBy;
    auto mapPassedEntity = [&](const auto arg, bool isResult = false) {
      if (arg.passBy == PassBy::AddressAndLength) {
        if (callee.characterize().IsBindC())
          return;
        // TODO: now that fir call has some attributes regarding character
        // return, PassBy::AddressAndLength should be retired.
        mlir::Location loc = toLocation();
        fir::factory::CharacterExprHelper charHelp{*builder, loc};
        mlir::Value casted =
            builder->createVolatileCast(loc, false, arg.firArgument);
        mlir::Value box = charHelp.createEmboxChar(casted, arg.firLength);
        mapBlockArgToDummyOrResult(arg.entity->get(), box, isResult);
      } else {
        if (arg.entity.has_value()) {
          mapBlockArgToDummyOrResult(arg.entity->get(), arg.firArgument,
                                     isResult);
        } else {
          assert(funit.parentHasTupleHostAssoc() && "expect tuple argument");
        }
      }
    };
    for (const language::Compability::lower::CalleeInterface::PassedEntity &arg :
         callee.getPassedArguments())
      mapPassedEntity(arg);

    // Always generate fir.dummy_scope even if there are no arguments.
    // It is currently used to create proper TBAA forest.
    if (lowerToHighLevelFIR()) {
      mlir::Value scopeOp = fir::DummyScopeOp::create(*builder, toLocation());
      setDummyArgsScope(scopeOp);
    }
    if (std::optional<language::Compability::lower::CalleeInterface::PassedEntity>
            passedResult = callee.getPassedResult()) {
      mapPassedEntity(*passedResult, /*isResult=*/true);
      // FIXME: need to make sure things are OK here. addSymbol may not be OK
      if (funit.primaryResult &&
          passedResult->entity->get() != *funit.primaryResult)
        mapBlockArgToDummyOrResult(
            *funit.primaryResult, getSymbolAddress(passedResult->entity->get()),
            /*isResult=*/true);
    }
  }

  /// Instantiate variable \p var and add it to the symbol map.
  /// See ConvertVariable.cpp.
  void instantiateVar(const language::Compability::lower::pft::Variable &var,
                      language::Compability::lower::AggregateStoreMap &storeMap) {
    language::Compability::lower::instantiateVariable(*this, var, localSymbols, storeMap);
    if (var.hasSymbol())
      genOpenMPSymbolProperties(*this, var);
  }

  /// Where applicable, save the exception state and halting, rounding, and
  /// underflow modes at function entry, and restore them at function exits.
  void manageFPEnvironment(language::Compability::lower::pft::FunctionLikeUnit &funit) {
    mlir::Location loc = toLocation();
    mlir::Location endLoc =
        toLocation(language::Compability::lower::pft::stmtSourceLoc(funit.endStmt));
    if (funit.hasIeeeAccess) {
      // Subject to F18 Clause 17.1p3, 17.3p3 states: If a flag is signaling
      // on entry to a procedure [...], the processor will set it to quiet
      // on entry and restore it to signaling on return. If a flag signals
      // during execution of a procedure, the processor shall not set it to
      // quiet on return.
      mlir::func::FuncOp testExcept = fir::factory::getFetestexcept(*builder);
      mlir::func::FuncOp clearExcept = fir::factory::getFeclearexcept(*builder);
      mlir::func::FuncOp raiseExcept = fir::factory::getFeraiseexcept(*builder);
      mlir::Value ones = builder->createIntegerConstant(
          loc, testExcept.getFunctionType().getInput(0), -1);
      mlir::Value exceptSet =
          fir::CallOp::create(*builder, loc, testExcept, ones).getResult(0);
      fir::CallOp::create(*builder, loc, clearExcept, exceptSet);
      bridge.fctCtx().attachCleanup([=]() {
        fir::CallOp::create(*builder, endLoc, raiseExcept, exceptSet);
      });
    }
    if (funit.mayModifyHaltingMode) {
      // F18 Clause 17.6p1: In a procedure [...], the processor shall not
      // change the halting mode on entry, and on return shall ensure that
      // the halting mode is the same as it was on entry.
      mlir::func::FuncOp getExcept = fir::factory::getFegetexcept(*builder);
      mlir::func::FuncOp disableExcept =
          fir::factory::getFedisableexcept(*builder);
      mlir::func::FuncOp enableExcept =
          fir::factory::getFeenableexcept(*builder);
      mlir::Value exceptSet =
          fir::CallOp::create(*builder, loc, getExcept).getResult(0);
      mlir::Value ones = builder->createIntegerConstant(
          loc, disableExcept.getFunctionType().getInput(0), -1);
      bridge.fctCtx().attachCleanup([=]() {
        fir::CallOp::create(*builder, endLoc, disableExcept, ones);
        fir::CallOp::create(*builder, endLoc, enableExcept, exceptSet);
      });
    }
    if (funit.mayModifyRoundingMode) {
      // F18 Clause 17.4p5: In a procedure [...], the processor shall not
      // change the rounding modes on entry, and on return shall ensure that
      // the rounding modes are the same as they were on entry.
      mlir::func::FuncOp getRounding =
          fir::factory::getLlvmGetRounding(*builder);
      mlir::func::FuncOp setRounding =
          fir::factory::getLlvmSetRounding(*builder);
      mlir::Value roundingMode =
          fir::CallOp::create(*builder, loc, getRounding).getResult(0);
      bridge.fctCtx().attachCleanup([=]() {
        fir::CallOp::create(*builder, endLoc, setRounding, roundingMode);
      });
    }
    if ((funit.mayModifyUnderflowMode) &&
        (bridge.getTargetCharacteristics().hasSubnormalFlushingControl(
            /*any=*/true))) {
      // F18 Clause 17.5p2: In a procedure [...], the processor shall not
      // change the underflow mode on entry, and on return shall ensure that
      // the underflow mode is the same as it was on entry.
      mlir::Value underflowMode =
          fir::runtime::genGetUnderflowMode(*builder, loc);
      bridge.fctCtx().attachCleanup([=]() {
        fir::runtime::genSetUnderflowMode(*builder, loc, {underflowMode});
      });
    }
  }

  /// Start translation of a function.
  void startNewFunction(language::Compability::lower::pft::FunctionLikeUnit &funit) {
    assert(!builder && "expected nullptr");
    bridge.fctCtx().pushScope();
    bridge.openAccCtx().pushScope();
    const language::Compability::semantics::Scope &scope = funit.getScope();
    LLVM_DEBUG(toolchain::dbgs() << "\n[bridge - startNewFunction]";
               if (auto *sym = scope.symbol()) toolchain::dbgs() << " " << *sym;
               toolchain::dbgs() << "\n");
    // Setting the builder is not necessary here, because callee
    // always looks up the FuncOp from the module. If there was a function that
    // was not declared yet, this call to callee will cause an assertion
    // failure.
    language::Compability::lower::CalleeInterface callee(funit, *this);
    mlir::func::FuncOp func = callee.addEntryBlockAndMapArguments();
    builder =
        new fir::FirOpBuilder(func, bridge.getKindMap(), &mlirSymbolTable);
    assert(builder && "FirOpBuilder did not instantiate");
    builder->setComplexDivisionToRuntimeFlag(
        bridge.getLoweringOptions().getComplexDivisionToRuntime());
    builder->setFastMathFlags(bridge.getLoweringOptions().getMathOptions());
    builder->setInsertionPointToStart(&func.front());
    if (funit.parent.isA<language::Compability::lower::pft::FunctionLikeUnit>()) {
      // Give internal linkage to internal functions. There are no name clash
      // risks, but giving global linkage to internal procedure will break the
      // static link register in shared libraries because of the system calls.
      // Also, it should be possible to eliminate the procedure code if all the
      // uses have been inlined.
      fir::factory::setInternalLinkage(func);
    } else {
      func.setVisibility(mlir::SymbolTable::Visibility::Public);
    }
    assert(blockId == 0 && "invalid blockId");
    assert(activeConstructStack.empty() && "invalid construct stack state");

    // Manage floating point exception, halting mode, and rounding mode
    // settings at function entry and exit.
    if (!funit.isMainProgram())
      manageFPEnvironment(funit);

    mapDummiesAndResults(funit, callee);

    // Map host associated symbols from parent procedure if any.
    if (funit.parentHasHostAssoc())
      funit.parentHostAssoc().internalProcedureBindings(*this, localSymbols);

    // Non-primary results of a function with multiple entry points.
    // These result values share storage with the primary result.
    toolchain::SmallVector<language::Compability::lower::pft::Variable> deferredFuncResultList;

    // Backup actual argument for entry character results with different
    // lengths. It needs to be added to the non-primary results symbol before
    // mapSymbolAttributes is called.
    language::Compability::lower::SymbolBox resultArg;
    if (std::optional<language::Compability::lower::CalleeInterface::PassedEntity>
            passedResult = callee.getPassedResult())
      resultArg = lookupSymbol(passedResult->entity->get());

    language::Compability::lower::AggregateStoreMap storeMap;

    // Map all containing submodule and module equivalences and variables, in
    // case they are referenced. It might be better to limit this to variables
    // that are actually referenced, although that is more complicated when
    // there are equivalenced variables.
    auto &scopeVariableListMap =
        language::Compability::lower::pft::getScopeVariableListMap(funit);
    for (auto *scp = &scope.parent(); !scp->IsGlobal(); scp = &scp->parent())
      if (scp->kind() == language::Compability::semantics::Scope::Kind::Module)
        for (const auto &var : language::Compability::lower::pft::getScopeVariableList(
                 *scp, scopeVariableListMap))
          if (!var.isRuntimeTypeInfoData())
            instantiateVar(var, storeMap);

    // Map function equivalences and variables.
    mlir::Value primaryFuncResultStorage;
    for (const language::Compability::lower::pft::Variable &var :
         language::Compability::lower::pft::getScopeVariableList(scope)) {
      // Always instantiate aggregate storage blocks.
      if (var.isAggregateStore()) {
        instantiateVar(var, storeMap);
        continue;
      }
      const language::Compability::semantics::Symbol &sym = var.getSymbol();
      if (funit.parentHasHostAssoc()) {
        // Never instantiate host associated variables, as they are already
        // instantiated from an argument tuple. Instead, just bind the symbol
        // to the host variable, which must be in the map.
        const language::Compability::semantics::Symbol &ultimate = sym.GetUltimate();
        if (funit.parentHostAssoc().isAssociated(ultimate)) {
          copySymbolBinding(ultimate, sym);
          continue;
        }
      }
      if (!sym.IsFuncResult() || !funit.primaryResult) {
        instantiateVar(var, storeMap);
      } else if (&sym == funit.primaryResult) {
        instantiateVar(var, storeMap);
        primaryFuncResultStorage = getSymbolAddress(sym);
      } else {
        deferredFuncResultList.push_back(var);
      }
    }

    // TODO: should use same mechanism as equivalence?
    // One blocking point is character entry returns that need special handling
    // since they are not locally allocated but come as argument. CHARACTER(*)
    // is not something that fits well with equivalence lowering.
    for (const language::Compability::lower::pft::Variable &altResult :
         deferredFuncResultList) {
      language::Compability::lower::StatementContext stmtCtx;
      if (std::optional<language::Compability::lower::CalleeInterface::PassedEntity>
              passedResult = callee.getPassedResult()) {
        mapBlockArgToDummyOrResult(altResult.getSymbol(), resultArg.getAddr(),
                                   /*isResult=*/true);
        language::Compability::lower::mapSymbolAttributes(*this, altResult, localSymbols,
                                            stmtCtx);
      } else {
        // catch cases where the allocation for the function result storage type
        // doesn't match the type of this symbol
        mlir::Value preAlloc = primaryFuncResultStorage;
        mlir::Type resTy = primaryFuncResultStorage.getType();
        mlir::Type symTy = genType(altResult);
        mlir::Type wrappedSymTy = fir::ReferenceType::get(symTy);
        if (resTy != wrappedSymTy) {
          // check size of the pointed to type so we can't overflow by writing
          // double precision to a single precision allocation, etc
          LLVM_ATTRIBUTE_UNUSED auto getBitWidth = [this](mlir::Type ty) {
            // 15.6.2.6.3: differering result types should be integer, real,
            // complex or logical
            if (auto cmplx = mlir::dyn_cast_or_null<mlir::ComplexType>(ty))
              return 2 * cmplx.getElementType().getIntOrFloatBitWidth();
            if (auto logical = mlir::dyn_cast_or_null<fir::LogicalType>(ty)) {
              fir::KindTy kind = logical.getFKind();
              return builder->getKindMap().getLogicalBitsize(kind);
            }
            return ty.getIntOrFloatBitWidth();
          };
          assert(getBitWidth(fir::unwrapRefType(resTy)) >= getBitWidth(symTy));

          // convert the storage to the symbol type so that the hlfir.declare
          // gets the correct type for this symbol
          preAlloc = fir::ConvertOp::create(*builder, getCurrentLocation(),
                                            wrappedSymTy, preAlloc);
        }

        language::Compability::lower::mapSymbolAttributes(*this, altResult, localSymbols,
                                            stmtCtx, preAlloc);
      }
    }

    // If this is a host procedure with host associations, then create the tuple
    // of pointers for passing to the internal procedures.
    if (!funit.getHostAssoc().empty())
      funit.getHostAssoc().hostProcedureBindings(*this, localSymbols);

    // Unregister all dummy symbols, so that their cloning (e.g. for OpenMP
    // privatization) does not create the cloned hlfir.declare operations
    // with dummy_scope operands.
    resetRegisteredDummySymbols();

    // Create most function blocks in advance.
    createEmptyBlocks(funit.evaluationList);

    // Reinstate entry block as the current insertion point.
    builder->setInsertionPointToEnd(&func.front());

    if (callee.hasAlternateReturns()) {
      // Create a local temp to hold the alternate return index.
      // Give it an integer index type and the subroutine name (for dumps).
      // Attach it to the subroutine symbol in the localSymbols map.
      // Initialize it to zero, the "fallthrough" alternate return value.
      const language::Compability::semantics::Symbol &symbol = funit.getSubprogramSymbol();
      mlir::Location loc = toLocation();
      mlir::Type idxTy = builder->getIndexType();
      mlir::Value altResult =
          builder->createTemporary(loc, idxTy, toStringRef(symbol.name()));
      addSymbol(symbol, altResult);
      mlir::Value zero = builder->createIntegerConstant(loc, idxTy, 0);
      fir::StoreOp::create(*builder, loc, zero, altResult);
    }

    if (language::Compability::lower::pft::Evaluation *alternateEntryEval =
            funit.getEntryEval())
      genBranch(alternateEntryEval->lexicalSuccessor->block);
  }

  /// Create global blocks for the current function. This eliminates the
  /// distinction between forward and backward targets when generating
  /// branches. A block is "global" if it can be the target of a GOTO or
  /// other source code branch. A block that can only be targeted by a
  /// compiler generated branch is "local". For example, a DO loop preheader
  /// block containing loop initialization code is global. A loop header
  /// block, which is the target of the loop back edge, is local. Blocks
  /// belong to a region. Any block within a nested region must be replaced
  /// with a block belonging to that region. Branches may not cross region
  /// boundaries.
  void createEmptyBlocks(
      std::list<language::Compability::lower::pft::Evaluation> &evaluationList) {
    mlir::Region *region = &builder->getRegion();
    for (language::Compability::lower::pft::Evaluation &eval : evaluationList) {
      if (eval.isNewBlock)
        eval.block = builder->createBlock(region);
      if (eval.isConstruct() || eval.isDirective()) {
        if (eval.lowerAsUnstructured()) {
          createEmptyBlocks(eval.getNestedEvaluations());
        } else if (eval.hasNestedEvaluations()) {
          // A structured construct that is a target starts a new block.
          language::Compability::lower::pft::Evaluation &constructStmt =
              eval.getFirstNestedEvaluation();
          if (constructStmt.isNewBlock)
            constructStmt.block = builder->createBlock(region);
        }
      }
    }
  }

  /// Return the predicate: "current block does not have a terminator branch".
  bool blockIsUnterminated() {
    mlir::Block *currentBlock = builder->getBlock();
    return currentBlock->empty() ||
           !currentBlock->back().hasTrait<mlir::OpTrait::IsTerminator>();
  }

  /// Unconditionally switch code insertion to a new block.
  void startBlock(mlir::Block *newBlock) {
    assert(newBlock && "missing block");
    // Default termination for the current block is a fallthrough branch to
    // the new block.
    if (blockIsUnterminated())
      genBranch(newBlock);
    // Some blocks may be re/started more than once, and might not be empty.
    // If the new block already has (only) a terminator, set the insertion
    // point to the start of the block. Otherwise set it to the end.
    builder->setInsertionPointToStart(newBlock);
    if (blockIsUnterminated())
      builder->setInsertionPointToEnd(newBlock);
  }

  /// Conditionally switch code insertion to a new block.
  void maybeStartBlock(mlir::Block *newBlock) {
    if (newBlock)
      startBlock(newBlock);
  }

  void eraseDeadCodeAndBlocks(mlir::RewriterBase &rewriter,
                              toolchain::MutableArrayRef<mlir::Region> regions) {
    // WARNING: Do not add passes that can do folding or code motion here
    // because they might cross omp.target region boundaries, which can result
    // in incorrect code. Optimization passes like these must be added after
    // OMP early outlining has been done.
    (void)mlir::eraseUnreachableBlocks(rewriter, regions);
    (void)mlir::runRegionDCE(rewriter, regions);
  }

  /// Finish translation of a function.
  void endNewFunction(language::Compability::lower::pft::FunctionLikeUnit &funit) {
    setCurrentPosition(language::Compability::lower::pft::stmtSourceLoc(funit.endStmt));
    if (funit.isMainProgram()) {
      genExitRoutine(false);
    } else {
      genFIRProcedureExit(funit, funit.getSubprogramSymbol());
    }
    funit.finalBlock = nullptr;
    LLVM_DEBUG(toolchain::dbgs() << "\n[bridge - endNewFunction";
               if (auto *sym = funit.scope->symbol()) toolchain::dbgs()
               << " " << sym->name();
               toolchain::dbgs() << "] generated IR:\n\n"
                            << *builder->getFunction() << '\n');
    // Eliminate dead code as a prerequisite to calling other IR passes.
    // FIXME: This simplification should happen in a normal pass, not here.
    mlir::IRRewriter rewriter(*builder);
    (void)eraseDeadCodeAndBlocks(rewriter, {builder->getRegion()});
    delete builder;
    builder = nullptr;
    hostAssocTuple = mlir::Value{};
    localSymbols.clear();
    blockId = 0;
    dummyArgsScope = mlir::Value{};
    resetRegisteredDummySymbols();
  }

  /// Helper to generate GlobalOps when the builder is not positioned in any
  /// region block. This is required because the FirOpBuilder assumes it is
  /// always positioned inside a region block when creating globals, the easiest
  /// way to comply is to create a dummy function and to throw it away
  /// afterwards.
  void createBuilderOutsideOfFuncOpAndDo(
      const std::function<void()> &createGlobals) {
    // FIXME: get rid of the bogus function context and instantiate the
    // globals directly into the module.
    mlir::MLIRContext *context = &getMLIRContext();
    mlir::SymbolTable *symbolTable = getMLIRSymbolTable();
    mlir::func::FuncOp func = fir::FirOpBuilder::createFunction(
        mlir::UnknownLoc::get(context), getModuleOp(),
        fir::NameUniquer::doGenerated("Sham"),
        mlir::FunctionType::get(context, {}, {}), symbolTable);
    func.addEntryBlock();
    CHECK(!builder && "Expected builder to be uninitialized");
    builder = new fir::FirOpBuilder(func, bridge.getKindMap(), symbolTable);
    assert(builder && "FirOpBuilder did not instantiate");
    builder->setFastMathFlags(bridge.getLoweringOptions().getMathOptions());
    createGlobals();
    if (mlir::Region *region = func.getCallableRegion())
      region->dropAllReferences();
    func.erase();
    delete builder;
    builder = nullptr;
    localSymbols.clear();
    resetRegisteredDummySymbols();
  }

  /// Instantiate the data from a BLOCK DATA unit.
  void lowerBlockData(language::Compability::lower::pft::BlockDataUnit &bdunit) {
    createBuilderOutsideOfFuncOpAndDo([&]() {
      language::Compability::lower::AggregateStoreMap fakeMap;
      for (const auto &[_, sym] : bdunit.symTab) {
        if (sym->has<language::Compability::semantics::ObjectEntityDetails>()) {
          language::Compability::lower::pft::Variable var(*sym, true);
          instantiateVar(var, fakeMap);
        }
      }
    });
  }

  /// Create fir::Global for all the common blocks that appear in the program.
  void
  lowerCommonBlocks(const language::Compability::semantics::CommonBlockList &commonBlocks) {
    createBuilderOutsideOfFuncOpAndDo(
        [&]() { language::Compability::lower::defineCommonBlocks(*this, commonBlocks); });
  }

  /// Create intrinsic module array constant definitions.
  void createIntrinsicModuleDefinitions(language::Compability::lower::pft::Program &pft) {
    // The intrinsic module scope, if present, is the first scope.
    const language::Compability::semantics::Scope *intrinsicModuleScope = nullptr;
    for (language::Compability::lower::pft::Program::Units &u : pft.getUnits()) {
      language::Compability::common::visit(
          language::Compability::common::visitors{
              [&](language::Compability::lower::pft::FunctionLikeUnit &f) {
                intrinsicModuleScope = &f.getScope().parent();
              },
              [&](language::Compability::lower::pft::ModuleLikeUnit &m) {
                intrinsicModuleScope = &m.getScope().parent();
              },
              [&](language::Compability::lower::pft::BlockDataUnit &b) {},
              [&](language::Compability::lower::pft::CompilerDirectiveUnit &d) {},
              [&](language::Compability::lower::pft::OpenACCDirectiveUnit &d) {},
          },
          u);
      if (intrinsicModuleScope) {
        while (!intrinsicModuleScope->IsGlobal())
          intrinsicModuleScope = &intrinsicModuleScope->parent();
        intrinsicModuleScope = &intrinsicModuleScope->children().front();
        break;
      }
    }
    if (!intrinsicModuleScope || !intrinsicModuleScope->IsIntrinsicModules())
      return;
    for (const auto &scope : intrinsicModuleScope->children()) {
      toolchain::StringRef modName = toStringRef(scope.symbol()->name());
      if (modName != "__fortran_ieee_exceptions")
        continue;
      for (auto &var : language::Compability::lower::pft::getScopeVariableList(scope)) {
        const language::Compability::semantics::Symbol &sym = var.getSymbol();
        if (sym.test(language::Compability::semantics::Symbol::Flag::CompilerCreated))
          continue;
        const auto *object =
            sym.detailsIf<language::Compability::semantics::ObjectEntityDetails>();
        if (object && object->IsArray() && object->init())
          language::Compability::lower::createIntrinsicModuleGlobal(*this, var);
      }
    }
  }

  /// Lower a procedure (nest).
  void lowerFunc(language::Compability::lower::pft::FunctionLikeUnit &funit) {
    setCurrentPosition(funit.getStartingSourceLoc());
    setCurrentFunctionUnit(&funit);
    for (int entryIndex = 0, last = funit.entryPointList.size();
         entryIndex < last; ++entryIndex) {
      funit.setActiveEntry(entryIndex);
      startNewFunction(funit); // the entry point for lowering this procedure
      for (language::Compability::lower::pft::Evaluation &eval : funit.evaluationList)
        genFIR(eval);
      endNewFunction(funit);
    }
    funit.setActiveEntry(0);
    setCurrentFunctionUnit(nullptr);
    for (language::Compability::lower::pft::ContainedUnit &unit : funit.containedUnitList)
      if (auto *f = std::get_if<language::Compability::lower::pft::FunctionLikeUnit>(&unit))
        lowerFunc(*f); // internal procedure
  }

  /// Lower module variable definitions to fir::globalOp and OpenMP/OpenACC
  /// declarative construct.
  void lowerModuleDeclScope(language::Compability::lower::pft::ModuleLikeUnit &mod) {
    setCurrentPosition(mod.getStartingSourceLoc());
    auto &scopeVariableListMap =
        language::Compability::lower::pft::getScopeVariableListMap(mod);
    for (const auto &var : language::Compability::lower::pft::getScopeVariableList(
             mod.getScope(), scopeVariableListMap)) {

      // Only define the variables owned by this module.
      const language::Compability::semantics::Scope *owningScope = var.getOwningScope();
      if (owningScope && mod.getScope() != *owningScope)
        continue;

      // Very special case: The value of numeric_storage_size depends on
      // compilation options and therefore its value is not yet known when
      // building the builtins runtime. Instead, the parameter is folding a
      // __numeric_storage_size() expression which is loaded into the user
      // program. For the iso_fortran_env object file, omit the symbol as it
      // is never used.
      if (var.hasSymbol()) {
        const language::Compability::semantics::Symbol &sym = var.getSymbol();
        const language::Compability::semantics::Scope &owner = sym.owner();
        if (sym.name() == "numeric_storage_size" && owner.IsModule() &&
            DEREF(owner.symbol()).name() == "iso_fortran_env")
          continue;
      }

      language::Compability::lower::defineModuleVariable(*this, var);
    }
    for (auto &eval : mod.evaluationList)
      genFIR(eval);
  }

  /// Lower functions contained in a module.
  void lowerMod(language::Compability::lower::pft::ModuleLikeUnit &mod) {
    for (language::Compability::lower::pft::ContainedUnit &unit : mod.containedUnitList)
      if (auto *f = std::get_if<language::Compability::lower::pft::FunctionLikeUnit>(&unit))
        lowerFunc(*f);
  }

  void setCurrentPosition(const language::Compability::parser::CharBlock &position) {
    if (position != language::Compability::parser::CharBlock{})
      currentPosition = position;
  }

  /// Set current position at the location of \p parseTreeNode. Note that the
  /// position is updated automatically when visiting statements, but not when
  /// entering higher level nodes like constructs or procedures. This helper is
  /// intended to cover the latter cases.
  template <typename A>
  void setCurrentPositionAt(const A &parseTreeNode) {
    setCurrentPosition(language::Compability::parser::FindSourceLocation(parseTreeNode));
  }

  //===--------------------------------------------------------------------===//
  // Utility methods
  //===--------------------------------------------------------------------===//

  /// Convert a parser CharBlock to a Location
  mlir::Location toLocation(const language::Compability::parser::CharBlock &cb) {
    return genLocation(cb);
  }

  mlir::Location toLocation() { return toLocation(currentPosition); }
  void setCurrentEval(language::Compability::lower::pft::Evaluation &eval) {
    evalPtr = &eval;
  }
  language::Compability::lower::pft::Evaluation &getEval() {
    assert(evalPtr);
    return *evalPtr;
  }

  std::optional<language::Compability::evaluate::Shape>
  getShape(const language::Compability::lower::SomeExpr &expr) {
    return language::Compability::evaluate::GetShape(foldingContext, expr);
  }

  //===--------------------------------------------------------------------===//
  // Analysis on a nested explicit iteration space.
  //===--------------------------------------------------------------------===//

  void analyzeExplicitSpace(const language::Compability::parser::ConcurrentHeader &header) {
    explicitIterSpace.pushLevel();
    for (const language::Compability::parser::ConcurrentControl &ctrl :
         std::get<std::list<language::Compability::parser::ConcurrentControl>>(header.t)) {
      const language::Compability::semantics::Symbol *ctrlVar =
          std::get<language::Compability::parser::Name>(ctrl.t).symbol;
      explicitIterSpace.addSymbol(ctrlVar);
    }
    if (const auto &mask =
            std::get<std::optional<language::Compability::parser::ScalarLogicalExpr>>(
                header.t);
        mask.has_value())
      analyzeExplicitSpace(*language::Compability::semantics::GetExpr(*mask));
  }
  template <bool LHS = false, typename A>
  void analyzeExplicitSpace(const language::Compability::evaluate::Expr<A> &e) {
    explicitIterSpace.exprBase(&e, LHS);
  }
  void analyzeExplicitSpace(const language::Compability::evaluate::Assignment *assign) {
    auto analyzeAssign = [&](const language::Compability::lower::SomeExpr &lhs,
                             const language::Compability::lower::SomeExpr &rhs) {
      analyzeExplicitSpace</*LHS=*/true>(lhs);
      analyzeExplicitSpace(rhs);
    };
    language::Compability::common::visit(
        language::Compability::common::visitors{
            [&](const language::Compability::evaluate::ProcedureRef &procRef) {
              // Ensure the procRef expressions are the one being visited.
              assert(procRef.arguments().size() == 2);
              const language::Compability::lower::SomeExpr *lhs =
                  procRef.arguments()[0].value().UnwrapExpr();
              const language::Compability::lower::SomeExpr *rhs =
                  procRef.arguments()[1].value().UnwrapExpr();
              assert(lhs && rhs &&
                     "user defined assignment arguments must be expressions");
              analyzeAssign(*lhs, *rhs);
            },
            [&](const auto &) { analyzeAssign(assign->lhs, assign->rhs); }},
        assign->u);
    explicitIterSpace.endAssign();
  }
  void analyzeExplicitSpace(const language::Compability::parser::ForallAssignmentStmt &stmt) {
    language::Compability::common::visit([&](const auto &s) { analyzeExplicitSpace(s); },
                           stmt.u);
  }
  void analyzeExplicitSpace(const language::Compability::parser::AssignmentStmt &s) {
    analyzeExplicitSpace(s.typedAssignment->v.operator->());
  }
  void analyzeExplicitSpace(const language::Compability::parser::PointerAssignmentStmt &s) {
    analyzeExplicitSpace(s.typedAssignment->v.operator->());
  }
  void analyzeExplicitSpace(const language::Compability::parser::WhereConstruct &c) {
    analyzeExplicitSpace(
        std::get<
            language::Compability::parser::Statement<language::Compability::parser::WhereConstructStmt>>(
            c.t)
            .statement);
    for (const language::Compability::parser::WhereBodyConstruct &body :
         std::get<std::list<language::Compability::parser::WhereBodyConstruct>>(c.t))
      analyzeExplicitSpace(body);
    for (const language::Compability::parser::WhereConstruct::MaskedElsewhere &e :
         std::get<std::list<language::Compability::parser::WhereConstruct::MaskedElsewhere>>(
             c.t))
      analyzeExplicitSpace(e);
    if (const auto &e =
            std::get<std::optional<language::Compability::parser::WhereConstruct::Elsewhere>>(
                c.t);
        e.has_value())
      analyzeExplicitSpace(e.operator->());
  }
  void analyzeExplicitSpace(const language::Compability::parser::WhereConstructStmt &ws) {
    const language::Compability::lower::SomeExpr *exp = language::Compability::semantics::GetExpr(
        std::get<language::Compability::parser::LogicalExpr>(ws.t));
    addMaskVariable(exp);
    analyzeExplicitSpace(*exp);
  }
  void analyzeExplicitSpace(
      const language::Compability::parser::WhereConstruct::MaskedElsewhere &ew) {
    analyzeExplicitSpace(
        std::get<
            language::Compability::parser::Statement<language::Compability::parser::MaskedElsewhereStmt>>(
            ew.t)
            .statement);
    for (const language::Compability::parser::WhereBodyConstruct &e :
         std::get<std::list<language::Compability::parser::WhereBodyConstruct>>(ew.t))
      analyzeExplicitSpace(e);
  }
  void analyzeExplicitSpace(const language::Compability::parser::WhereBodyConstruct &body) {
    language::Compability::common::visit(
        language::Compability::common::visitors{
            [&](const language::Compability::common::Indirection<
                language::Compability::parser::WhereConstruct> &wc) {
              analyzeExplicitSpace(wc.value());
            },
            [&](const auto &s) { analyzeExplicitSpace(s.statement); }},
        body.u);
  }
  void analyzeExplicitSpace(const language::Compability::parser::MaskedElsewhereStmt &stmt) {
    const language::Compability::lower::SomeExpr *exp = language::Compability::semantics::GetExpr(
        std::get<language::Compability::parser::LogicalExpr>(stmt.t));
    addMaskVariable(exp);
    analyzeExplicitSpace(*exp);
  }
  void
  analyzeExplicitSpace(const language::Compability::parser::WhereConstruct::Elsewhere *ew) {
    for (const language::Compability::parser::WhereBodyConstruct &e :
         std::get<std::list<language::Compability::parser::WhereBodyConstruct>>(ew->t))
      analyzeExplicitSpace(e);
  }
  void analyzeExplicitSpace(const language::Compability::parser::WhereStmt &stmt) {
    const language::Compability::lower::SomeExpr *exp = language::Compability::semantics::GetExpr(
        std::get<language::Compability::parser::LogicalExpr>(stmt.t));
    addMaskVariable(exp);
    analyzeExplicitSpace(*exp);
    const std::optional<language::Compability::evaluate::Assignment> &assign =
        std::get<language::Compability::parser::AssignmentStmt>(stmt.t).typedAssignment->v;
    assert(assign.has_value() && "WHERE has no statement");
    analyzeExplicitSpace(assign.operator->());
  }
  void analyzeExplicitSpace(const language::Compability::parser::ForallStmt &forall) {
    analyzeExplicitSpace(
        std::get<
            language::Compability::common::Indirection<language::Compability::parser::ConcurrentHeader>>(
            forall.t)
            .value());
    analyzeExplicitSpace(std::get<language::Compability::parser::UnlabeledStatement<
                             language::Compability::parser::ForallAssignmentStmt>>(forall.t)
                             .statement);
    analyzeExplicitSpacePop();
  }
  void
  analyzeExplicitSpace(const language::Compability::parser::ForallConstructStmt &forall) {
    analyzeExplicitSpace(
        std::get<
            language::Compability::common::Indirection<language::Compability::parser::ConcurrentHeader>>(
            forall.t)
            .value());
  }
  void analyzeExplicitSpace(const language::Compability::parser::ForallConstruct &forall) {
    analyzeExplicitSpace(
        std::get<
            language::Compability::parser::Statement<language::Compability::parser::ForallConstructStmt>>(
            forall.t)
            .statement);
    for (const language::Compability::parser::ForallBodyConstruct &s :
         std::get<std::list<language::Compability::parser::ForallBodyConstruct>>(forall.t)) {
      language::Compability::common::visit(
          language::Compability::common::visitors{
              [&](const language::Compability::common::Indirection<
                  language::Compability::parser::ForallConstruct> &b) {
                analyzeExplicitSpace(b.value());
              },
              [&](const language::Compability::parser::WhereConstruct &w) {
                analyzeExplicitSpace(w);
              },
              [&](const auto &b) { analyzeExplicitSpace(b.statement); }},
          s.u);
    }
    analyzeExplicitSpacePop();
  }

  void analyzeExplicitSpacePop() { explicitIterSpace.popLevel(); }

  void addMaskVariable(language::Compability::lower::FrontEndExpr exp) {
    // Note: use i8 to store bool values. This avoids round-down behavior found
    // with sequences of i1. That is, an array of i1 will be truncated in size
    // and be too small. For example, a buffer of type fir.array<7xi1> will have
    // 0 size.
    mlir::Type i64Ty = builder->getIntegerType(64);
    mlir::TupleType ty = fir::factory::getRaggedArrayHeaderType(*builder);
    mlir::Type buffTy = ty.getType(1);
    mlir::Type shTy = ty.getType(2);
    mlir::Location loc = toLocation();
    mlir::Value hdr = builder->createTemporary(loc, ty);
    // FIXME: Is there a way to create a `zeroinitializer` in LLVM-IR dialect?
    // For now, explicitly set lazy ragged header to all zeros.
    // auto nilTup = builder->createNullConstant(loc, ty);
    // fir::StoreOp::create(*builder, loc, nilTup, hdr);
    mlir::Type i32Ty = builder->getIntegerType(32);
    mlir::Value zero = builder->createIntegerConstant(loc, i32Ty, 0);
    mlir::Value zero64 = builder->createIntegerConstant(loc, i64Ty, 0);
    mlir::Value flags = fir::CoordinateOp::create(
        *builder, loc, builder->getRefType(i64Ty), hdr, zero);
    fir::StoreOp::create(*builder, loc, zero64, flags);
    mlir::Value one = builder->createIntegerConstant(loc, i32Ty, 1);
    mlir::Value nullPtr1 = builder->createNullConstant(loc, buffTy);
    mlir::Value var = fir::CoordinateOp::create(
        *builder, loc, builder->getRefType(buffTy), hdr, one);
    fir::StoreOp::create(*builder, loc, nullPtr1, var);
    mlir::Value two = builder->createIntegerConstant(loc, i32Ty, 2);
    mlir::Value nullPtr2 = builder->createNullConstant(loc, shTy);
    mlir::Value shape = fir::CoordinateOp::create(
        *builder, loc, builder->getRefType(shTy), hdr, two);
    fir::StoreOp::create(*builder, loc, nullPtr2, shape);
    implicitIterSpace.addMaskVariable(exp, var, shape, hdr);
    explicitIterSpace.outermostContext().attachCleanup(
        [builder = this->builder, hdr, loc]() {
          fir::runtime::genRaggedArrayDeallocate(loc, *builder, hdr);
        });
  }

  void createRuntimeTypeInfoGlobals() {}

  bool lowerToHighLevelFIR() const {
    return bridge.getLoweringOptions().getLowerToHighLevelFIR();
  }

  // Returns the mangling prefix for the given constant expression.
  std::string getConstantExprManglePrefix(mlir::Location loc,
                                          const language::Compability::lower::SomeExpr &expr,
                                          mlir::Type eleTy) {
    return language::Compability::common::visit(
        [&](const auto &x) -> std::string {
          using T = std::decay_t<decltype(x)>;
          if constexpr (language::Compability::common::HasMember<
                            T, language::Compability::lower::CategoryExpression>) {
            if constexpr (T::Result::category ==
                          language::Compability::common::TypeCategory::Derived) {
              if (const auto *constant =
                      std::get_if<language::Compability::evaluate::Constant<
                          language::Compability::evaluate::SomeDerived>>(&x.u))
                return language::Compability::lower::mangle::mangleArrayLiteral(eleTy,
                                                                  *constant);
              fir::emitFatalError(loc,
                                  "non a constant derived type expression");
            } else {
              return language::Compability::common::visit(
                  [&](const auto &someKind) -> std::string {
                    using T = std::decay_t<decltype(someKind)>;
                    using TK = language::Compability::evaluate::Type<T::Result::category,
                                                       T::Result::kind>;
                    if (const auto *constant =
                            std::get_if<language::Compability::evaluate::Constant<TK>>(
                                &someKind.u)) {
                      return language::Compability::lower::mangle::mangleArrayLiteral(
                          nullptr, *constant);
                    }
                    fir::emitFatalError(
                        loc, "not a language::Compability::evaluate::Constant<T> expression");
                    return {};
                  },
                  x.u);
            }
          } else {
            fir::emitFatalError(loc, "unexpected expression");
          }
        },
        expr.u);
  }

  /// Performing OpenMP lowering actions that were deferred to the end of
  /// lowering.
  void finalizeOpenMPLowering(
      const language::Compability::semantics::Symbol *globalOmpRequiresSymbol) {
    if (!ompDeferredDeclareTarget.empty()) {
      bool deferredDeviceFuncFound =
          language::Compability::lower::markOpenMPDeferredDeclareTargetFunctions(
              getModuleOp().getOperation(), ompDeferredDeclareTarget, *this);
      ompDeviceCodeFound = ompDeviceCodeFound || deferredDeviceFuncFound;
    }

    // Set the module attribute related to OpenMP requires directives
    if (ompDeviceCodeFound)
      language::Compability::lower::genOpenMPRequires(getModuleOp().getOperation(),
                                        globalOmpRequiresSymbol);
  }

  /// Record fir.dummy_scope operation for this function.
  /// It will be used to set dummy_scope operand of the hlfir.declare
  /// operations.
  void setDummyArgsScope(mlir::Value val) {
    assert(!dummyArgsScope && val);
    dummyArgsScope = val;
  }

  /// Record the given symbol as a dummy argument of this function.
  void registerDummySymbol(language::Compability::semantics::SymbolRef symRef) {
    auto *sym = &*symRef;
    registeredDummySymbols.insert(sym);
  }

  /// Reset all registered dummy symbols.
  void resetRegisteredDummySymbols() { registeredDummySymbols.clear(); }

  void setCurrentFunctionUnit(language::Compability::lower::pft::FunctionLikeUnit *unit) {
    currentFunctionUnit = unit;
  }

  //===--------------------------------------------------------------------===//

  language::Compability::lower::LoweringBridge &bridge;
  language::Compability::evaluate::FoldingContext foldingContext;
  fir::FirOpBuilder *builder = nullptr;
  language::Compability::lower::pft::Evaluation *evalPtr = nullptr;
  language::Compability::lower::pft::FunctionLikeUnit *currentFunctionUnit = nullptr;
  language::Compability::lower::SymMap localSymbols;
  language::Compability::parser::CharBlock currentPosition;
  TypeInfoConverter typeInfoConverter;

  // Stack to manage object deallocation and finalization at construct exits.
  toolchain::SmallVector<ConstructContext> activeConstructStack;

  /// BLOCK name mangling component map
  int blockId = 0;
  language::Compability::lower::mangle::ScopeBlockIdMap scopeBlockIdMap;

  /// FORALL statement/construct context
  language::Compability::lower::ExplicitIterSpace explicitIterSpace;

  /// WHERE statement/construct mask expression stack
  language::Compability::lower::ImplicitIterSpace implicitIterSpace;

  /// Tuple of host associated variables
  mlir::Value hostAssocTuple;

  /// Value of fir.dummy_scope operation for this function.
  mlir::Value dummyArgsScope;

  /// A set of dummy argument symbols for this function.
  /// The set is only preserved during the instatiation
  /// of variables for this function.
  toolchain::SmallPtrSet<const language::Compability::semantics::Symbol *, 16>
      registeredDummySymbols;

  /// A map of unique names for constant expressions.
  /// The names are used for representing the constant expressions
  /// with global constant initialized objects.
  /// The names are usually prefixed by a mangling string based
  /// on the element type of the constant expression, but the element
  /// type is not used as a key into the map (so the assumption is that
  /// the equivalent constant expressions are prefixed using the same
  /// element type).
  toolchain::DenseMap<const language::Compability::lower::SomeExpr *, std::string> literalNamesMap;

  /// Storage for Constant expressions used as keys for literalNamesMap.
  toolchain::SmallVector<std::unique_ptr<language::Compability::lower::SomeExpr>>
      literalExprsStorage;

  /// A counter for uniquing names in `literalNamesMap`.
  std::uint64_t uniqueLitId = 0;

  /// Whether an OpenMP target region or declare target function/subroutine
  /// intended for device offloading has been detected
  bool ompDeviceCodeFound = false;

  /// Keeps track of symbols defined as declare target that could not be
  /// processed at the time of lowering the declare target construct, such
  /// as certain cases where interfaces are declared but not defined within
  /// a module.
  toolchain::SmallVector<language::Compability::lower::OMPDeferredDeclareTargetInfo>
      ompDeferredDeclareTarget;

  const language::Compability::lower::ExprToValueMap *exprValueOverrides{nullptr};

  /// Stack of derived type under construction to avoid infinite loops when
  /// dealing with recursive derived types. This is held in the bridge because
  /// the state needs to be maintained between data and function type lowering
  /// utilities to deal with procedure pointer components whose arguments have
  /// the type of the containing derived type.
  language::Compability::lower::TypeConstructionStack typeConstructionStack;
  /// MLIR symbol table of the fir.global/func.func operations. Note that it is
  /// not guaranteed to contain all operations of the ModuleOp with Symbol
  /// attribute since mlirSymbolTable must pro-actively be maintained when
  /// new Symbol operations are created.
  mlir::SymbolTable mlirSymbolTable;

  /// Used to store context while recursing into regions during lowering.
  mlir::StateStack stateStack;
};

} // namespace

language::Compability::evaluate::FoldingContext
language::Compability::lower::LoweringBridge::createFoldingContext() {
  return {getDefaultKinds(), getIntrinsicTable(), getTargetCharacteristics(),
          getLanguageFeatures(), tempNames};
}

void language::Compability::lower::LoweringBridge::lower(
    const language::Compability::parser::Program &prg,
    const language::Compability::semantics::SemanticsContext &semanticsContext) {
  std::unique_ptr<language::Compability::lower::pft::Program> pft =
      language::Compability::lower::createPFT(prg, semanticsContext);
  if (dumpBeforeFir)
    language::Compability::lower::dumpPFT(toolchain::errs(), *pft);
  FirConverter converter{*this};
  converter.run(*pft);
}

void language::Compability::lower::LoweringBridge::parseSourceFile(toolchain::SourceMgr &srcMgr) {
  module = mlir::parseSourceFile<mlir::ModuleOp>(srcMgr, &context);
}

language::Compability::lower::LoweringBridge::LoweringBridge(
    mlir::MLIRContext &context,
    language::Compability::semantics::SemanticsContext &semanticsContext,
    const language::Compability::common::IntrinsicTypeDefaultKinds &defaultKinds,
    const language::Compability::evaluate::IntrinsicProcTable &intrinsics,
    const language::Compability::evaluate::TargetCharacteristics &targetCharacteristics,
    const language::Compability::parser::AllCookedSources &cooked, toolchain::StringRef triple,
    fir::KindMapping &kindMap,
    const language::Compability::lower::LoweringOptions &loweringOptions,
    const std::vector<language::Compability::lower::EnvironmentDefault> &envDefaults,
    const language::Compability::common::LanguageFeatureControl &languageFeatures,
    const toolchain::TargetMachine &targetMachine,
    const language::Compability::frontend::TargetOptions &targetOpts,
    const language::Compability::frontend::CodeGenOptions &cgOpts)
    : semanticsContext{semanticsContext}, defaultKinds{defaultKinds},
      intrinsics{intrinsics}, targetCharacteristics{targetCharacteristics},
      cooked{&cooked}, context{context}, kindMap{kindMap},
      loweringOptions{loweringOptions}, envDefaults{envDefaults},
      languageFeatures{languageFeatures} {
  // Register the diagnostic handler.
  if (loweringOptions.getRegisterMLIRDiagnosticsHandler()) {
    diagHandlerID =
        context.getDiagEngine().registerHandler([](mlir::Diagnostic &diag) {
          toolchain::raw_ostream &os = toolchain::errs();
          switch (diag.getSeverity()) {
          case mlir::DiagnosticSeverity::Error:
            os << "error: ";
            break;
          case mlir::DiagnosticSeverity::Remark:
            os << "info: ";
            break;
          case mlir::DiagnosticSeverity::Warning:
            os << "warning: ";
            break;
          default:
            break;
          }
          if (!mlir::isa<mlir::UnknownLoc>(diag.getLocation()))
            os << diag.getLocation() << ": ";
          os << diag << '\n';
          os.flush();
          return mlir::success();
        });
  }

  auto getPathLocation = [&semanticsContext, &context]() -> mlir::Location {
    std::optional<std::string> path;
    const auto &allSources{semanticsContext.allCookedSources().allSources()};
    if (auto initial{allSources.GetFirstFileProvenance()};
        initial && !initial->empty()) {
      if (const auto *sourceFile{allSources.GetSourceFile(initial->start())}) {
        path = sourceFile->path();
      }
    }

    if (path.has_value()) {
      toolchain::SmallString<256> curPath(*path);
      toolchain::sys::fs::make_absolute(curPath);
      toolchain::sys::path::remove_dots(curPath);
      return mlir::FileLineColLoc::get(&context, curPath.str(), /*line=*/0,
                                       /*col=*/0);
    } else {
      return mlir::UnknownLoc::get(&context);
    }
  };

  // Create the module and attach the attributes.
  module = mlir::OwningOpRef<mlir::ModuleOp>(
      mlir::ModuleOp::create(getPathLocation()));
  assert(*module && "module was not created");
  fir::setTargetTriple(*module, triple);
  fir::setKindMapping(*module, kindMap);
  fir::setTargetCPU(*module, targetMachine.getTargetCPU());
  fir::setTuneCPU(*module, targetOpts.cpuToTuneFor);
  fir::setAtomicIgnoreDenormalMode(*module,
                                   targetOpts.atomicIgnoreDenormalMode);
  fir::setAtomicFineGrainedMemory(*module, targetOpts.atomicFineGrainedMemory);
  fir::setAtomicRemoteMemory(*module, targetOpts.atomicRemoteMemory);
  fir::setTargetFeatures(*module, targetMachine.getTargetFeatureString());
  fir::support::setMLIRDataLayout(*module, targetMachine.createDataLayout());
  fir::setIdent(*module, language::Compability::common::getFlangFullVersion());
  if (cgOpts.RecordCommandLine)
    fir::setCommandline(*module, *cgOpts.RecordCommandLine);
}

language::Compability::lower::LoweringBridge::~LoweringBridge() {
  if (diagHandlerID)
    context.getDiagEngine().eraseHandler(*diagHandlerID);
}

void language::Compability::lower::genCleanUpInRegionIfAny(
    mlir::Location loc, fir::FirOpBuilder &builder, mlir::Region &region,
    language::Compability::lower::StatementContext &context) {
  if (!context.hasCode())
    return;
  mlir::OpBuilder::InsertPoint insertPt = builder.saveInsertionPoint();
  if (region.empty())
    builder.createBlock(&region);
  else
    builder.setInsertionPointToEnd(&region.front());
  context.finalizeAndPop();
  hlfir::YieldOp::ensureTerminator(region, builder, loc);
  builder.restoreInsertionPoint(insertPt);
}
