
//===--- Passes.cpp - Codira Compiler SIL Pass Entrypoints -----------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
///
///  \file
///  This file provides implementations of a few helper functions
///  which provide abstracted entrypoints to the SILPasses stage.
///
///  \note The actual SIL passes should be implemented in per-pass source files,
///  not in this file.
///
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "sil-optimizer"

#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/AST/ASTContext.h"
#include "language/AST/Module.h"
#include "language/SIL/SILModule.h"
#include "language/SILOptimizer/Analysis/Analysis.h"
#include "language/SILOptimizer/OptimizerBridging.h"
#include "language/SILOptimizer/PassManager/PassManager.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/InstOptUtils.h"
#include "toolchain/ADT/Statistic.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/ErrorOr.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/YAMLParser.h"

using namespace language;

bool language::runSILDiagnosticPasses(SILModule &Module) {
  auto &opts = Module.getOptions();

  // Verify the module, if required.
  // OSSA lifetimes are incomplete until the SILGenCleanup pass runs.
  if (opts.VerifyAll)
    Module.verifyIncompleteOSSA();

  // If we parsed a .sil file that is already in canonical form, don't rerun
  // the diagnostic passes.
  if (Module.getStage() != SILStage::Raw)
    return false;

  executePassPipelinePlan(&Module,
                          SILPassPipelinePlan::getSILGenPassPipeline(opts),
                          /*isMandatory*/ true);

  if (opts.VerifyAll && opts.OSSAVerifyComplete)
    Module.verifyOwnership();

  executePassPipelinePlan(&Module,
                          SILPassPipelinePlan::getDiagnosticPassPipeline(opts),
                          /*isMandatory*/ true);

  // If we were asked to debug serialization, exit now.
  auto &Ctx = Module.getASTContext();
  if (opts.DebugSerialization)
    return Ctx.hadError();

  // Generate diagnostics.
  Module.setStage(SILStage::Canonical);

  // Verify the module, if required.
  if (opts.VerifyAll)
    Module.verify();
  else {
    TOOLCHAIN_DEBUG(Module.verify());
  }

  // If errors were produced during SIL analysis, return true.
  return Ctx.hadError();
}

bool language::runSILLowerHopToActorPass(SILModule &Module) {
  auto &Ctx = Module.getASTContext();

  auto &opts = Module.getOptions();
  executePassPipelinePlan(
      &Module, SILPassPipelinePlan::getLowerHopToActorPassPipeline(opts));

  return Ctx.hadError();
}

bool language::runSILOwnershipEliminatorPass(SILModule &Module) {
  auto &Ctx = Module.getASTContext();

  auto &opts = Module.getOptions();
  executePassPipelinePlan(
      &Module, SILPassPipelinePlan::getOwnershipEliminatorPassPipeline(opts));

  return Ctx.hadError();
}

void language::runSILOptimizationPasses(SILModule &Module) {
  auto &opts = Module.getOptions();

  // Verify the module, if required.
  if (opts.VerifyAll)
    Module.verify();

  if (opts.DisableSILPerfOptimizations) {
    // If we are not supposed to run SIL perf optzns, we may still need to
    // serialize. So serialize now.
    executePassPipelinePlan(
        &Module, SILPassPipelinePlan::getSerializeSILPassPipeline(opts),
        /*isMandatory*/ true);
    return;
  }

  executePassPipelinePlan(
      &Module, SILPassPipelinePlan::getPerformancePassPipeline(opts));

  // Check if we actually serialized our module. If we did not, serialize now.
  if (!Module.isSerialized()) {
    executePassPipelinePlan(
        &Module, SILPassPipelinePlan::getSerializeSILPassPipeline(opts),
        /*isMandatory*/ true);
  }

  // If we were asked to debug serialization, exit now.
  if (opts.DebugSerialization)
    return;

  // Verify the module, if required.
  if (opts.VerifyAll)
    Module.verify();
  else {
    TOOLCHAIN_DEBUG(Module.verify());
  }
}

void language::runSILPassesForOnone(SILModule &Module) {
  // Verify the module, if required.
  if (Module.getOptions().VerifyAll)
    Module.verify();

  // We want to run the Onone passes also for function which have an explicit
  // Onone attribute.
  executePassPipelinePlan(
      &Module, SILPassPipelinePlan::getOnonePassPipeline(Module.getOptions()),
      /*isMandatory*/ true);

  // Verify the module, if required.
  if (Module.getOptions().VerifyAll)
    Module.verify();
  else {
    TOOLCHAIN_DEBUG(Module.verify());
  }
}

void language::runSILOptimizationPassesWithFileSpecification(SILModule &M,
                                                          StringRef Filename) {
  auto &opts = M.getOptions();
  executePassPipelinePlan(
      &M, SILPassPipelinePlan::getPassPipelineFromFile(opts, Filename));
}

/// Get the Pass ID enum value from an ID string.
PassKind language::PassKindFromString(StringRef IDString) {
  return toolchain::StringSwitch<PassKind>(IDString)
#define PASS(ID, TAG, DESCRIPTION) .Case(#ID, PassKind::ID)
#include "language/SILOptimizer/PassManager/Passes.def"
      .Default(PassKind::invalidPassKind);
}

/// Get an ID string for the given pass Kind.
/// This is useful for tools that identify a pass
/// by its type name.
StringRef language::PassKindID(PassKind Kind) {
  switch (Kind) {
#define PASS(ID, TAG, DESCRIPTION)                                             \
  case PassKind::ID:                                                           \
    return #ID;
#include "language/SILOptimizer/PassManager/Passes.def"
  case PassKind::invalidPassKind:
    toolchain_unreachable("Invalid pass kind?!");
  }

  toolchain_unreachable("Unhandled PassKind in switch.");
}

/// Get a tag string for the given pass Kind.
/// This format is useful for command line options.
StringRef language::PassKindTag(PassKind Kind) {
  switch (Kind) {
#define PASS(ID, TAG, DESCRIPTION)                                             \
  case PassKind::ID:                                                           \
    return TAG;
#include "language/SILOptimizer/PassManager/Passes.def"
  case PassKind::invalidPassKind:
    toolchain_unreachable("Invalid pass kind?!");
  }

  toolchain_unreachable("Unhandled PassKind in switch.");
}

// During SIL Lowering, passes may see partially lowered SIL, which is
// inconsistent with the current (canonical) stage. We don't change the SIL
// stage until lowering is complete. Consequently, any pass added to this
// PassManager needs to be able to handle the output of the previous pass. If
// the function pass needs to read SIL from other functions, it may be best to
// convert it to a module pass to ensure that the SIL input is always at the
// same stage of lowering.
void language::runSILLoweringPasses(SILModule &Module) {
  auto &opts = Module.getOptions();
  executePassPipelinePlan(&Module,
                          SILPassPipelinePlan::getLoweringPassPipeline(opts),
                          /*isMandatory*/ true);

  Module.setStage(SILStage::Lowered);
}

/// Registered briged pass run functions.
static toolchain::StringMap<BridgedModulePassRunFn> bridgedModulePassRunFunctions;
static toolchain::StringMap<BridgedFunctionPassRunFn> bridgedFunctionPassRunFunctions;
static bool passesRegistered = false;

/// Runs a bridged module pass.
///
/// \p runFunction is a cache for the run function, so that it has to be looked
/// up only once in bridgedPassRunFunctions.
static void runBridgedModulePass(BridgedModulePassRunFn &runFunction,
                                 SILPassManager *passManager,
                                 StringRef passName) {
  if (!runFunction) {
    runFunction = bridgedModulePassRunFunctions[passName];
    if (!runFunction) {
      if (passesRegistered) {
        ABORT([&](auto &out) {
          out << "Codira pass " << passName << " is not registered";
        });
      }
      return;
    }
  }
  runFunction({passManager->getCodiraPassInvocation()});
}

/// Runs a bridged function pass.
///
/// \p runFunction is a cache for the run function, so that it has to be looked
/// up only once in bridgedPassRunFunctions.
static void runBridgedFunctionPass(BridgedFunctionPassRunFn &runFunction,
                                   SILPassManager *passManager,
                                   SILFunction *f, StringRef passName) {
  if (!runFunction) {
    runFunction = bridgedFunctionPassRunFunctions[passName];
    if (!runFunction) {
      if (passesRegistered) {
        ABORT([&](auto &out) {
          out << "Codira pass " << passName << " is not registered";
        });
      }
      return;
    }
  }
  if (!f->isBridged()) {
    ABORT("SILFunction metatype is not registered");
  }
  runFunction({{f}, {passManager->getCodiraPassInvocation()}});
}

// Called from initializeCodiraModules().
void SILPassManager_registerModulePass(BridgedStringRef name,
                                       BridgedModulePassRunFn runFn) {
  bridgedModulePassRunFunctions[name.unbridged()] = runFn;
  passesRegistered = true;
}

void SILPassManager_registerFunctionPass(BridgedStringRef name,
                                         BridgedFunctionPassRunFn runFn) {
  bridgedFunctionPassRunFunctions[name.unbridged()] = runFn;
  passesRegistered = true;
}

#define LEGACY_PASS(ID, TAG, DESCRIPTION)

#define PASS(ID, TAG, DESCRIPTION) \
class ID##Pass : public SILFunctionTransform {                             \
  static BridgedFunctionPassRunFn runFunction;                             \
  void run() override {                                                    \
    runBridgedFunctionPass(runFunction, PM, getFunction(), TAG);           \
  }                                                                        \
};                                                                         \
BridgedFunctionPassRunFn ID##Pass::runFunction = nullptr;                  \
SILTransform *language::create##ID() { return new ID##Pass(); }               \

#define MODULE_PASS(ID, TAG, DESCRIPTION) \
class ID##Pass : public SILModuleTransform {                               \
  static BridgedModulePassRunFn runFunction;                               \
  void run() override {                                                    \
    runBridgedModulePass(runFunction, PM, TAG);                            \
  }                                                                        \
};                                                                         \
BridgedModulePassRunFn ID##Pass::runFunction = nullptr;                    \
SILTransform *language::create##ID() { return new ID##Pass(); }               \

#include "language/SILOptimizer/PassManager/Passes.def"

#undef LANGUAGE_FUNCTION_PASS_COMMON
