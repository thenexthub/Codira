//===-- Lower/Bridge.h -- main interface to lowering ------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_LOWER_BRIDGE_H
#define LANGUAGE_COMPABILITY_LOWER_BRIDGE_H

#include "language/Compability/Frontend/CodeGenOptions.h"
#include "language/Compability/Frontend/TargetOptions.h"
#include "language/Compability/Lower/AbstractConverter.h"
#include "language/Compability/Lower/EnvironmentDefault.h"
#include "language/Compability/Lower/LoweringOptions.h"
#include "language/Compability/Lower/StatementContext.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Dialect/Support/KindMapping.h"
#include "language/Compability/Support/Fortran.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/OwningOpRef.h"
#include <set>

namespace toolchain {
class TargetMachine;
} // namespace toolchain

namespace language::Compability {
namespace common {
class IntrinsicTypeDefaultKinds;
} // namespace common
namespace evaluate {
class IntrinsicProcTable;
class TargetCharacteristics;
} // namespace evaluate
namespace parser {
class AllCookedSources;
struct Program;
} // namespace parser
namespace semantics {
class SemanticsContext;
} // namespace semantics

namespace lower {

//===----------------------------------------------------------------------===//
// Lowering bridge
//===----------------------------------------------------------------------===//

/// The lowering bridge converts the front-end parse trees and semantics
/// checking residual to MLIR (FIR dialect) code.
class LoweringBridge {
public:
  /// Create a lowering bridge instance.
  static LoweringBridge
  create(mlir::MLIRContext &ctx,
         language::Compability::semantics::SemanticsContext &semanticsContext,
         const language::Compability::common::IntrinsicTypeDefaultKinds &defaultKinds,
         const language::Compability::evaluate::IntrinsicProcTable &intrinsics,
         const language::Compability::evaluate::TargetCharacteristics &targetCharacteristics,
         const language::Compability::parser::AllCookedSources &allCooked,
         toolchain::StringRef triple, fir::KindMapping &kindMap,
         const language::Compability::lower::LoweringOptions &loweringOptions,
         const std::vector<language::Compability::lower::EnvironmentDefault> &envDefaults,
         const language::Compability::common::LanguageFeatureControl &languageFeatures,
         const toolchain::TargetMachine &targetMachine,
         const language::Compability::frontend::TargetOptions &targetOptions,
         const language::Compability::frontend::CodeGenOptions &codeGenOptions) {
    return LoweringBridge(ctx, semanticsContext, defaultKinds, intrinsics,
                          targetCharacteristics, allCooked, triple, kindMap,
                          loweringOptions, envDefaults, languageFeatures,
                          targetMachine, targetOptions, codeGenOptions);
  }
  ~LoweringBridge();

  //===--------------------------------------------------------------------===//
  // Getters
  //===--------------------------------------------------------------------===//

  mlir::MLIRContext &getMLIRContext() { return context; }

  /// Get the ModuleOp. It can never be null, which is asserted in the ctor.
  mlir::ModuleOp getModule() { return *module; }
  mlir::ModuleOp getModuleAndRelease() { return module.release(); }

  const language::Compability::common::IntrinsicTypeDefaultKinds &getDefaultKinds() const {
    return defaultKinds;
  }
  const language::Compability::evaluate::IntrinsicProcTable &getIntrinsicTable() const {
    return intrinsics;
  }
  const language::Compability::evaluate::TargetCharacteristics &
  getTargetCharacteristics() const {
    return targetCharacteristics;
  }
  const language::Compability::parser::AllCookedSources *getCookedSource() const {
    return cooked;
  }

  /// Get the kind map.
  const fir::KindMapping &getKindMap() const { return kindMap; }

  const language::Compability::lower::LoweringOptions &getLoweringOptions() const {
    return loweringOptions;
  }

  const std::vector<language::Compability::lower::EnvironmentDefault> &
  getEnvironmentDefaults() const {
    return envDefaults;
  }

  const language::Compability::common::LanguageFeatureControl &getLanguageFeatures() const {
    return languageFeatures;
  }

  /// Create a folding context. Careful: this is very expensive.
  language::Compability::evaluate::FoldingContext createFoldingContext();

  language::Compability::semantics::SemanticsContext &getSemanticsContext() const {
    return semanticsContext;
  }

  language::Compability::lower::StatementContext &fctCtx() { return functionContext; }

  language::Compability::lower::StatementContext &openAccCtx() { return openAccContext; }

  bool validModule() { return getModule(); }

  //===--------------------------------------------------------------------===//
  // Perform the creation of an mlir::ModuleOp
  //===--------------------------------------------------------------------===//

  /// Read in an MLIR input file rather than lowering Fortran sources.
  /// This is intended to be used for testing.
  void parseSourceFile(toolchain::SourceMgr &);

  /// Cross the bridge from the Fortran parse-tree, etc. to MLIR dialects
  void lower(const language::Compability::parser::Program &program,
             const language::Compability::semantics::SemanticsContext &semanticsContext);

private:
  explicit LoweringBridge(
      mlir::MLIRContext &ctx,
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
      const language::Compability::frontend::TargetOptions &targetOptions,
      const language::Compability::frontend::CodeGenOptions &codeGenOptions);
  LoweringBridge() = delete;
  LoweringBridge(const LoweringBridge &) = delete;

  language::Compability::semantics::SemanticsContext &semanticsContext;
  language::Compability::lower::StatementContext functionContext;
  language::Compability::lower::StatementContext openAccContext;
  const language::Compability::common::IntrinsicTypeDefaultKinds &defaultKinds;
  const language::Compability::evaluate::IntrinsicProcTable &intrinsics;
  const language::Compability::evaluate::TargetCharacteristics &targetCharacteristics;
  const language::Compability::parser::AllCookedSources *cooked;
  mlir::MLIRContext &context;
  mlir::OwningOpRef<mlir::ModuleOp> module;
  fir::KindMapping &kindMap;
  const language::Compability::lower::LoweringOptions &loweringOptions;
  const std::vector<language::Compability::lower::EnvironmentDefault> &envDefaults;
  const language::Compability::common::LanguageFeatureControl &languageFeatures;
  std::set<std::string> tempNames;
  std::optional<mlir::DiagnosticEngine::HandlerID> diagHandlerID;
};

} // namespace lower
} // namespace language::Compability

#endif // FORTRAN_LOWER_BRIDGE_H
