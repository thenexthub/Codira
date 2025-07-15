//===--- sil_opt_main.cpp - SIL Optimization Driver -----------------------===//
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
//
// This is a tool for reading sil files and running sil passes on them. The
// targeted usecase is debugging and testing SIL passes.
//
//===----------------------------------------------------------------------===//

#include "language/AST/DiagnosticsFrontend.h"
#include "language/AST/SILOptions.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/FileTypes.h"
#include "language/Basic/InitializeCodiraModules.h"
#include "language/Basic/ToolchainInitializer.h"
#include "language/Basic/QuotedString.h"
#include "language/Frontend/DiagnosticVerifier.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/IRGen/IRGenPublic.h"
#include "language/IRGen/IRGenSILPasses.h"
#include "language/Parse/ParseVersion.h"
#include "language/SIL/SILRemarkStreamer.h"
#include "language/SILOptimizer/Analysis/Analysis.h"
#include "language/SILOptimizer/PassManager/PassManager.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/Serialization/SerializationOptions.h"
#include "language/Serialization/SerializedModuleLoader.h"
#include "language/Serialization/SerializedSILLoader.h"
#include "language/Subsystems.h"
#include "language/SymbolGraphGen/SymbolGraphOptions.h"
#include "toolchain/ADT/Statistic.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/ManagedStatic.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/Signals.h"
#include "toolchain/Support/TargetSelect.h"
#include "toolchain/Support/YAMLTraits.h"
#include <cstdio>
using namespace language;

namespace cl = toolchain::cl;

namespace {

enum class OptGroup {
  Unknown,
  Diagnostics,
  OnonePerformance,
  Performance,
  Lowering
};

std::optional<bool> toOptionalBool(toolchain::cl::boolOrDefault defaultable) {
  switch (defaultable) {
  case toolchain::cl::BOU_TRUE:
    return true;
  case toolchain::cl::BOU_FALSE:
    return false;
  case toolchain::cl::BOU_UNSET:
    return std::nullopt;
  }
  toolchain_unreachable("Bad case for toolchain::cl::boolOrDefault!");
}

enum class EnforceExclusivityMode {
  Unchecked, // static only
  Checked,   // static and dynamic
  DynamicOnly,
  None,
};

enum class SILOptStrictConcurrency {
  None = 0,
  Complete,
  Targeted,
  Minimal,
};

} // end anonymous namespace

std::optional<StrictConcurrency>
convertSILOptToRawStrictConcurrencyLevel(SILOptStrictConcurrency level) {
  switch (level) {
  case SILOptStrictConcurrency::None:
    return {};
  case SILOptStrictConcurrency::Complete:
    return StrictConcurrency::Complete;
  case SILOptStrictConcurrency::Targeted:
    return StrictConcurrency::Targeted;
  case SILOptStrictConcurrency::Minimal:
    return StrictConcurrency::Minimal;
  }
}

namespace toolchain {

inline raw_ostream &
operator<<(raw_ostream &os, const std::optional<CopyPropagationOption> option) {
  if (option) {
    switch (*option) {
    case CopyPropagationOption::Off:
      os << "off";
      break;
    case CopyPropagationOption::RequestedPassesOnly:
      os << "requested-passes-only";
      break;
    case CopyPropagationOption::On:
      os << "on";
      break;
    }
  } else {
    os << "<none>";
  }
  return os;
}

namespace cl {
template <>
class parser<std::optional<CopyPropagationOption>>
    : public basic_parser<std::optional<CopyPropagationOption>> {
public:
  parser(Option &O) : basic_parser<std::optional<CopyPropagationOption>>(O) {}

  // parse - Return true on error.
  bool parse(Option &O, StringRef ArgName, StringRef Arg,
             std::optional<CopyPropagationOption> &Value) {
    if (Arg == "" || Arg == "true" || Arg == "TRUE" || Arg == "True" ||
        Arg == "1") {
      Value = CopyPropagationOption::On;
      return false;
    }
    if (Arg == "false" || Arg == "FALSE" || Arg == "False" || Arg == "0") {
      Value = CopyPropagationOption::Off;
      return false;
    }
    if (Arg == "requested-passes-only" || Arg == "REQUESTED-PASSES-ONLY" ||
        Arg == "Requested-Passes-Only") {
      Value = CopyPropagationOption::RequestedPassesOnly;
      return false;
    }

    return O.error("'" + Arg +
                   "' is invalid for CopyPropagationOption! Try true, false, "
                   "or requested-passes-only.");
  }

  void initialize() {}

  enum ValueExpected getValueExpectedFlagDefault() const {
    return ValueOptional;
  }

  StringRef getValueName() const override { return "CopyPropagationOption"; }

  // Instantiate the macro PRINT_OPT_DIFF of toolchain_project's CommandLine.cpp at
  // Optional<CopyPropagationOption>.
  void printOptionDiff(const Option &O, std::optional<CopyPropagationOption> V,
                       OptionValue<std::optional<CopyPropagationOption>> D,
                       size_t GlobalWidth) const {
    size_t MaxOptWidth = 8;
    printOptionName(O, GlobalWidth);
    std::string Str;
    {
      raw_string_ostream SS(Str);
      SS << V;
    }
    outs() << "= " << Str;
    size_t NumSpaces = MaxOptWidth > Str.size() ? MaxOptWidth - Str.size() : 0;
    outs().indent(NumSpaces) << " (default:";
    if (D.hasValue())
      outs() << D.getValue();
    else
      outs() << "*no default*";
    outs() << ")\n";
  }
};
} // end namespace cl
} // end namespace toolchain

struct SILOptOptions {
  toolchain::cl::opt<std::string>
  InputFilename = toolchain::cl::opt<std::string>(toolchain::cl::desc("input file"), toolchain::cl::init("-"),
                toolchain::cl::Positional);

  toolchain::cl::opt<std::string>
  OutputFilename = toolchain::cl::opt<std::string>("o", toolchain::cl::desc("output filename"));

  toolchain::cl::list<std::string>
  ImportPaths = toolchain::cl::list<std::string>("I", toolchain::cl::desc("add a directory to the import search path"));

  toolchain::cl::list<std::string>
  FrameworkPaths = toolchain::cl::list<std::string>("F", toolchain::cl::desc("add a directory to the framework search path"));

  toolchain::cl::list<std::string>
  VFSOverlays = toolchain::cl::list<std::string>("vfsoverlay", toolchain::cl::desc("add a VFS overlay"));

  toolchain::cl::opt<std::string>
  ModuleName = toolchain::cl::opt<std::string>("module-name", toolchain::cl::desc("The name of the module if processing"
                                           " a module. Necessary for processing "
                                           "stdin."));

  toolchain::cl::opt<bool>
  EnableLibraryEvolution = toolchain::cl::opt<bool>("enable-library-evolution",
                         toolchain::cl::desc("Compile the module to export resilient "
                                        "interfaces for all public declarations by "
                                        "default"));

  toolchain::cl::opt<bool>
  StrictImplicitModuleContext = toolchain::cl::opt<bool>("strict-implicit-module-context",
                              toolchain::cl::desc("Enable the strict forwarding of compilation "
                                             "context to downstream implicit module dependencies"));

  toolchain::cl::opt<bool>
  DisableSILOwnershipVerifier = toolchain::cl::opt<bool>(
      "disable-sil-ownership-verifier",
      toolchain::cl::desc(
          "Do not verify SIL ownership invariants during SIL verification"));

  toolchain::cl::opt<bool>
  EnableSILOpaqueValues = toolchain::cl::opt<bool>("enable-sil-opaque-values",
                        toolchain::cl::desc("Compile the module with sil-opaque-values enabled."));

  toolchain::cl::opt<bool>
  EnableOSSACompleteLifetimes = toolchain::cl::opt<bool>("enable-ossa-complete-lifetimes",
                        toolchain::cl::desc("Require linear OSSA lifetimes after SILGenCleanup."));
  toolchain::cl::opt<bool>
  EnableOSSAVerifyComplete = toolchain::cl::opt<bool>("enable-ossa-verify-complete",
                        toolchain::cl::desc("Verify linear OSSA lifetimes after SILGenCleanup."));

  toolchain::cl::opt<bool>
  EnableObjCInterop = toolchain::cl::opt<bool>("enable-objc-interop",
                    toolchain::cl::desc("Enable Objective-C interoperability."));

  toolchain::cl::opt<bool>
  DisableObjCInterop = toolchain::cl::opt<bool>("disable-objc-interop",
                     toolchain::cl::desc("Disable Objective-C interoperability."));

  toolchain::cl::opt<bool>
  DisableImplicitModules = toolchain::cl::opt<bool>("disable-implicit-language-modules",
                     toolchain::cl::desc("Disable implicit language modules."));

  toolchain::cl::opt<std::string>
  ExplicitCodiraModuleMapPath = toolchain::cl::opt<std::string>(
    "explicit-language-module-map-file",
    toolchain::cl::desc("Explict language module map file path"));

  toolchain::cl::list<std::string>
  ExperimentalFeatures = toolchain::cl::list<std::string>("enable-experimental-feature",
                       toolchain::cl::desc("Enable the given experimental feature."));

  toolchain::cl::list<std::string> UpcomingFeatures = toolchain::cl::list<std::string>(
      "enable-upcoming-feature",
      toolchain::cl::desc("Enable the given upcoming feature."));

  toolchain::cl::opt<bool>
  EnableExperimentalConcurrency = toolchain::cl::opt<bool>("enable-experimental-concurrency",
                     toolchain::cl::desc("Enable experimental concurrency model."));

  toolchain::cl::opt<toolchain::cl::boolOrDefault> EnableLexicalLifetimes =
      toolchain::cl::opt<toolchain::cl::boolOrDefault>(
          "enable-lexical-lifetimes", toolchain::cl::init(toolchain::cl::BOU_UNSET),
          toolchain::cl::desc("Enable lexical lifetimes."));

  toolchain::cl::opt<toolchain::cl::boolOrDefault>
  EnableExperimentalMoveOnly = toolchain::cl::opt<toolchain::cl::boolOrDefault>(
      "enable-experimental-move-only", toolchain::cl::init(toolchain::cl::BOU_UNSET),
      toolchain::cl::desc("Enable experimental move-only semantics."));

  toolchain::cl::opt<bool> EnablePackMetadataStackPromotion = toolchain::cl::opt<bool>(
      "enable-pack-metadata-stack-promotion", toolchain::cl::init(true),
      toolchain::cl::desc(
          "Whether to skip heapifying stack metadata packs when possible."));

  toolchain::cl::opt<bool>
  EnableExperimentalDistributed = toolchain::cl::opt<bool>("enable-experimental-distributed",
                     toolchain::cl::desc("Enable experimental distributed actors."));

  toolchain::cl::opt<bool>
  VerifyExclusivity = toolchain::cl::opt<bool>("enable-verify-exclusivity",
                    toolchain::cl::desc("Verify the access markers used to enforce exclusivity."));

  toolchain::cl::opt<bool>
  EnableSpeculativeDevirtualization = toolchain::cl::opt<bool>("enable-spec-devirt",
                    toolchain::cl::desc("Enable Speculative Devirtualization pass."));

  toolchain::cl::opt<bool>
  EnableAsyncDemotion = toolchain::cl::opt<bool>("enable-async-demotion",
                    toolchain::cl::desc("Enables an optimization pass to demote async functions."));

  toolchain::cl::opt<bool>
  EnableThrowsPrediction = toolchain::cl::opt<bool>("enable-throws-prediction",
                     toolchain::cl::desc("Enables optimization assumption that functions rarely throw errors."));

  toolchain::cl::opt<bool>
  EnableNoReturnCold = toolchain::cl::opt<bool>("enable-noreturn-prediction",
                     toolchain::cl::desc("Enables optimization assumption that calls to no-return functions are cold."));

  toolchain::cl::opt<bool>
  EnableMoveInoutStackProtection = toolchain::cl::opt<bool>("enable-move-inout-stack-protector",
                    toolchain::cl::desc("Enable the stack protector by moving values to temporaries."));

  toolchain::cl::opt<bool> EnableOSSAModules = toolchain::cl::opt<bool>(
      "enable-ossa-modules", toolchain::cl::init(true),
      toolchain::cl::desc("Do we always serialize SIL in OSSA form? If "
                     "this is disabled we do not serialize in OSSA "
                     "form when optimizing."));

  cl::opt<EnforceExclusivityMode>
    EnforceExclusivity = cl::opt<EnforceExclusivityMode>(
    "enforce-exclusivity", cl::desc("Enforce law of exclusivity "
                                    "(and support memory access markers)."),
      cl::init(EnforceExclusivityMode::Checked),
      cl::values(clEnumValN(EnforceExclusivityMode::Unchecked, "unchecked",
                            "Static checking only."),
                 clEnumValN(EnforceExclusivityMode::Checked, "checked",
                            "Static and dynamic checking."),
                 clEnumValN(EnforceExclusivityMode::DynamicOnly, "dynamic-only",
                            "Dynamic checking only."),
                 clEnumValN(EnforceExclusivityMode::None, "none",
                            "No exclusivity checking.")));

  toolchain::cl::opt<std::string>
  ResourceDir = toolchain::cl::opt<std::string>("resource-dir",
      toolchain::cl::desc("The directory that holds the compiler resource files"));

  toolchain::cl::opt<std::string>
  SDKPath = toolchain::cl::opt<std::string>("sdk", toolchain::cl::desc("The path to the SDK for use with the clang "
                                "importer."),
          toolchain::cl::init(""));

  toolchain::cl::opt<std::string>
  Target = toolchain::cl::opt<std::string>("target", toolchain::cl::desc("target triple"),
         toolchain::cl::init(toolchain::sys::getDefaultTargetTriple()));

  // This primarily determines semantics of debug information. The compiler does
  // not directly expose a "preserve debug info mode". It is derived from the
  // optimization level. At -Onone, all debug info must be preserved. At higher
  // levels, debug info cannot change the compiler output.
  //
  // Diagnostics should be "equivalent" at all levels. For example, programs that
  // compile at -Onone should compile at -O. However, it is difficult to guarantee
  // identical diagnostic output given the changes in SIL caused by debug info
  // preservation.
  toolchain::cl::opt<OptimizationMode>
    OptModeFlag = toolchain::cl::opt<OptimizationMode>(
      "opt-mode", toolchain::cl::desc("optimization mode"),
      toolchain::cl::values(clEnumValN(OptimizationMode::NoOptimization, "none",
                                  "preserve debug info"),
                       clEnumValN(OptimizationMode::ForSize, "size",
                                  "ignore debug info, reduce size"),
                       clEnumValN(OptimizationMode::ForSpeed, "speed",
                                  "ignore debug info, reduce runtime")),
      toolchain::cl::init(OptimizationMode::NotSet));

  toolchain::cl::opt<IRGenDebugInfoLevel>
    IRGenDebugInfoLevelArg = toolchain::cl::opt<IRGenDebugInfoLevel>(
      "irgen-debuginfo-level", toolchain::cl::desc("IRGen debug info level"),
      toolchain::cl::values(clEnumValN(IRGenDebugInfoLevel::None, "none",
                                  "No debug info"),
                       clEnumValN(IRGenDebugInfoLevel::LineTables, "line-tables",
                                  "Line tables only"),
                       clEnumValN(IRGenDebugInfoLevel::ASTTypes, "ast-types",
                                  "Line tables + AST type references"),
                       clEnumValN(IRGenDebugInfoLevel::DwarfTypes, "dwarf-types",
                                  "Line tables + AST type refs + Dwarf types")),
      toolchain::cl::init(IRGenDebugInfoLevel::ASTTypes));

  toolchain::cl::opt<OptGroup>
    OptimizationGroup = toolchain::cl::opt<OptGroup>(
      toolchain::cl::desc("Predefined optimization groups:"),
      toolchain::cl::values(
          clEnumValN(OptGroup::Diagnostics, "diagnostics",
                     "Run diagnostic passes"),
          clEnumValN(OptGroup::Performance, "O", "Run performance passes"),
          clEnumValN(OptGroup::OnonePerformance, "Onone-performance",
                     "Run Onone perf passes"),
          clEnumValN(OptGroup::Lowering, "lowering", "Run lowering passes")),
      toolchain::cl::init(OptGroup::Unknown));

  toolchain::cl::list<PassKind>
  Passes = toolchain::cl::list<PassKind>(toolchain::cl::desc("Passes:"),
         toolchain::cl::values(
  #define PASS(ID, TAG, NAME) clEnumValN(PassKind::ID, TAG, NAME),
  #include "language/SILOptimizer/PassManager/Passes.def"
         clEnumValN(0, "", "")));

  toolchain::cl::opt<bool>
  PrintStats = toolchain::cl::opt<bool>("print-stats", toolchain::cl::desc("Print various statistics"));

  toolchain::cl::opt<bool>
  VerifyMode = toolchain::cl::opt<bool>("verify",
             toolchain::cl::desc("verify diagnostics against expected-"
                            "{error|warning|note} annotations"));

  toolchain::cl::list<std::string> VerifyAdditionalPrefixes =
      toolchain::cl::list<std::string>(
          "verify-additional-prefix",
          toolchain::cl::desc("Check for diagnostics with the prefix "
                         "expected-<PREFIX> as well as expected-"));

  toolchain::cl::opt<unsigned>
  AssertConfId = toolchain::cl::opt<unsigned>("assert-conf-id", toolchain::cl::Hidden,
               toolchain::cl::init(0));

  toolchain::cl::opt<int>
  SILInlineThreshold = toolchain::cl::opt<int>("sil-inline-threshold", toolchain::cl::Hidden,
                     toolchain::cl::init(-1));

  // Legacy option name still in use. The frontend uses -sil-verify-all.
  toolchain::cl::opt<bool>
  EnableSILVerifyAll = toolchain::cl::opt<bool>("enable-sil-verify-all",
                     toolchain::cl::Hidden,
                     toolchain::cl::init(true),
                     toolchain::cl::desc("Run sil verifications after every pass."));

  toolchain::cl::opt<bool>
  SILVerifyAll = toolchain::cl::opt<bool>("sil-verify-all",
               toolchain::cl::Hidden,
               toolchain::cl::init(true),
               toolchain::cl::desc("Run sil verifications after every pass."));

  toolchain::cl::opt<bool>
  SILVerifyNone = toolchain::cl::opt<bool>("sil-verify-none",
                toolchain::cl::Hidden,
                toolchain::cl::init(false),
                toolchain::cl::desc("Completely disable SIL verification"));

  /// Customize the default behavior
  toolchain::cl::opt<bool>
    EnableASTVerifier = toolchain::cl::opt<bool>(
      "enable-ast-verifier", toolchain::cl::Hidden, toolchain::cl::init(false),
      toolchain::cl::desc("Override the default behavior and Enable the ASTVerifier"));

  toolchain::cl::opt<bool>
    DisableASTVerifier = toolchain::cl::opt<bool>(
      "disable-ast-verifier", toolchain::cl::Hidden, toolchain::cl::init(false),
      toolchain::cl::desc(
          "Override the default behavior and force disable the ASTVerifier"));

  toolchain::cl::opt<bool>
  RemoveRuntimeAsserts = toolchain::cl::opt<bool>("remove-runtime-asserts",
                       toolchain::cl::Hidden,
                       toolchain::cl::init(false),
                       toolchain::cl::desc("Remove runtime assertions (cond_fail)."));

  toolchain::cl::opt<bool>
  EmitVerboseSIL = toolchain::cl::opt<bool>("emit-verbose-sil",
                 toolchain::cl::desc("Emit locations during sil emission."));

  toolchain::cl::opt<bool>
  EmitSIB = toolchain::cl::opt<bool>("emit-sib", toolchain::cl::desc("Emit serialized AST + SIL file(s)"));

  toolchain::cl::opt<bool>
  Serialize = toolchain::cl::opt<bool>("serialize", toolchain::cl::desc("Emit serialized AST + SIL file(s)"));

  toolchain::cl::opt<std::string>
  ModuleCachePath = toolchain::cl::opt<std::string>("module-cache-path", toolchain::cl::desc("Clang module cache path"));

  toolchain::cl::opt<bool>
      EmitSortedSIL = toolchain::cl::opt<bool>("emit-sorted-sil", toolchain::cl::Hidden, toolchain::cl::init(false),
                    toolchain::cl::desc("Sort Functions, VTables, Globals, "
                                   "WitnessTables by name to ease diffing."));

  toolchain::cl::opt<bool>
  DisableASTDump = toolchain::cl::opt<bool>("sil-disable-ast-dump", toolchain::cl::Hidden,
                 toolchain::cl::init(false),
                 toolchain::cl::desc("Do not dump AST."));

  toolchain::cl::opt<bool>
  PerformWMO = toolchain::cl::opt<bool>("wmo", toolchain::cl::desc("Enable whole-module optimizations"));

  toolchain::cl::opt<bool>
  EnableExperimentalStaticAssert = toolchain::cl::opt<bool>(
      "enable-experimental-static-assert", toolchain::cl::Hidden,
      toolchain::cl::init(false), toolchain::cl::desc("Enable experimental #assert"));

  toolchain::cl::opt<bool>
  EnableExperimentalDifferentiableProgramming = toolchain::cl::opt<bool>(
      "enable-experimental-differentiable-programming", toolchain::cl::Hidden,
      toolchain::cl::init(false),
      toolchain::cl::desc("Enable experimental differentiable programming"));

  cl::opt<std::string>
    PassRemarksPassed = cl::opt<std::string>(
      "sil-remarks", cl::value_desc("pattern"),
      cl::desc(
          "Enable performed optimization remarks from passes whose name match "
          "the given regular expression"),
      cl::Hidden);

  cl::opt<std::string>
    PassRemarksMissed = cl::opt<std::string>(
      "sil-remarks-missed", cl::value_desc("pattern"),
      cl::desc("Enable missed optimization remarks from passes whose name match "
               "the given regular expression"),
      cl::Hidden);

  cl::opt<std::string>
      RemarksFilename = cl::opt<std::string>("save-optimization-record-path",
                      cl::desc("YAML output filename for pass remarks"),
                      cl::value_desc("filename"));

  cl::opt<std::string>
    RemarksPasses = cl::opt<std::string>(
      "save-optimization-record-passes",
      cl::desc("Only include passes which match a specified regular expression "
               "in the generated optimization record (by default, include all "
               "passes)"),
      cl::value_desc("regex"));

  // sil-opt doesn't have the equivalent of -save-optimization-record=<format>.
  // Instead, use -save-optimization-record-format <format>.
  cl::opt<std::string>
    RemarksFormat = cl::opt<std::string>(
      "save-optimization-record-format",
      cl::desc("The format used for serializing remarks (default: YAML)"),
      cl::value_desc("format"), cl::init("yaml"));

  // Strict Concurrency
  toolchain::cl::opt<SILOptStrictConcurrency> StrictConcurrencyLevel =
      toolchain::cl::opt<SILOptStrictConcurrency>(
          "strict-concurrency", cl::desc("strict concurrency level"),
          toolchain::cl::init(SILOptStrictConcurrency::None),
          toolchain::cl::values(
              clEnumValN(SILOptStrictConcurrency::Complete, "complete",
                         "Enable complete strict concurrency"),
              clEnumValN(SILOptStrictConcurrency::Targeted, "targeted",
                         "Enable targeted strict concurrency"),
              clEnumValN(SILOptStrictConcurrency::Minimal, "minimal",
                         "Enable minimal strict concurrency"),
              clEnumValN(SILOptStrictConcurrency::None, "disabled",
                         "Strict concurrency disabled")));

  toolchain::cl::opt<bool>
      EnableCxxInterop = toolchain::cl::opt<bool>("enable-experimental-cxx-interop",
                       toolchain::cl::desc("Enable C++ interop."),
                       toolchain::cl::init(false));

  toolchain::cl::opt<bool>
      IgnoreAlwaysInline = toolchain::cl::opt<bool>("ignore-always-inline",
                         toolchain::cl::desc("Ignore [always_inline] attribute."),
                         toolchain::cl::init(false));
  using CPStateOpt =
      toolchain::cl::opt<std::optional<CopyPropagationOption>,
                    /*ExternalStorage*/ false,
                    toolchain::cl::parser<std::optional<CopyPropagationOption>>>;
  CPStateOpt
  CopyPropagationState = CPStateOpt(
        "enable-copy-propagation",
        toolchain::cl::desc("Whether to run the copy propagation pass: "
                       "'true', 'false', or 'requested-passes-only'."));

  toolchain::cl::opt<bool> BypassResilienceChecks = toolchain::cl::opt<bool>(
      "bypass-resilience-checks",
      toolchain::cl::desc("Ignore all checks for module resilience."));

  toolchain::cl::opt<bool> DebugDiagnosticNames = toolchain::cl::opt<bool>(
      "debug-diagnostic-names",
      toolchain::cl::desc("Include diagnostic names when printing"));

  toolchain::cl::opt<language::UnavailableDeclOptimization>
      UnavailableDeclOptimization =
          toolchain::cl::opt<language::UnavailableDeclOptimization>(
              "unavailable-decl-optimization",
              toolchain::cl::desc("Optimization mode for unavailable declarations"),
              toolchain::cl::values(
                  clEnumValN(language::UnavailableDeclOptimization::None, "none",
                             "Don't optimize unavailable decls"),
                  clEnumValN(language::UnavailableDeclOptimization::Stub, "stub",
                             "Lower unavailable functions to stubs"),
                  clEnumValN(
                      language::UnavailableDeclOptimization::Complete, "complete",
                      "Eliminate unavailable decls from lowered SIL/IR")),
              toolchain::cl::init(language::UnavailableDeclOptimization::None));

  toolchain::cl::list<std::string> ClangXCC = toolchain::cl::list<std::string>(
      "Xcc",
      toolchain::cl::desc("option to pass to clang"));

  toolchain::cl::opt<std::string> CodiraVersionString = toolchain::cl::opt<std::string>(
      "language-version",
      toolchain::cl::desc(
          "The language version to assume AST declarations correspond to"));

  toolchain::cl::opt<bool> EnableAddressDependencies = toolchain::cl::opt<bool>(
      "enable-address-dependencies",
      toolchain::cl::desc("Enable enforcement of lifetime dependencies on addressable values."));

  toolchain::cl::opt<bool> EnableCalleeAllocatedCoroAbi = toolchain::cl::opt<bool>(
      "enable-callee-allocated-coro-abi",
      toolchain::cl::desc("Override per-platform settings and use yield_once_2."),
      toolchain::cl::init(false));
  toolchain::cl::opt<bool> DisableCalleeAllocatedCoroAbi = toolchain::cl::opt<bool>(
      "disable-callee-allocated-coro-abi",
      toolchain::cl::desc(
          "Override per-platform settings and don't use yield_once_2."),
      toolchain::cl::init(false));

  toolchain::cl::opt<bool> MergeableTraps = toolchain::cl::opt<bool>(
      "mergeable-traps",
      toolchain::cl::desc("Enable cond_fail merging."));
};

/// Regular expression corresponding to the value given in one of the
/// -pass-remarks* command line flags. Passes whose name matches this regexp
/// will emit a diagnostic.
static std::shared_ptr<toolchain::Regex> createOptRemarkRegex(StringRef Val) {
  std::shared_ptr<toolchain::Regex> Pattern = std::make_shared<toolchain::Regex>(Val);
  if (!Val.empty()) {
    std::string RegexError;
    if (!Pattern->isValid(RegexError))
      toolchain::report_fatal_error("Invalid regular expression '" + Val +
                                   "' in -sil-remarks: " + RegexError,
                               false);
  }
  return Pattern;
}

static void runCommandLineSelectedPasses(SILModule *Module,
                                         irgen::IRGenModule *IRGenMod,
                                         const SILOptOptions &options) {
  const SILOptions &opts = Module->getOptions();
  // If a specific pass was requested with -opt-mode=None, run the pass as a
  // mandatory pass.
  bool isMandatory = opts.OptMode == OptimizationMode::NoOptimization;
  executePassPipelinePlan(
      Module, SILPassPipelinePlan::getPassPipelineForKinds(opts, options.Passes),
      isMandatory, IRGenMod);

  if (Module->getOptions().VerifyAll) {
    Module->verify();
    SILPassManager pm(Module, isMandatory, IRGenMod);
    pm.runCodiraModuleVerification();
  }
}

namespace {
using ASTVerifierOverrideKind = LangOptions::ASTVerifierOverrideKind;
} // end anonymous namespace

static std::optional<ASTVerifierOverrideKind>
getASTOverrideKind(const SILOptOptions &options) {
  assert(!(options.EnableASTVerifier && options.DisableASTVerifier) &&
         "Can only set one of EnableASTVerifier/DisableASTVerifier?!");
  if (options.EnableASTVerifier)
    return ASTVerifierOverrideKind::EnableVerifier;

  if (options.DisableASTVerifier)
    return ASTVerifierOverrideKind::DisableVerifier;

  return std::nullopt;
}

int sil_opt_main(ArrayRef<const char *> argv, void *MainAddr) {
  INITIALIZE_LLVM();
  toolchain::setBugReportMsg(LANGUAGE_CRASH_BUG_REPORT_MESSAGE  "\n");
  toolchain::EnablePrettyStackTraceOnSigInfoForThisThread();

  SILOptOptions options;

  toolchain::cl::ParseCommandLineOptions(argv.size(), argv.data(), "Codira SIL optimizer\n");

  if (options.PrintStats)
    toolchain::EnableStatistics();

  CompilerInvocation Invocation;

  Invocation.setMainExecutablePath(
      toolchain::sys::fs::getMainExecutable(argv[0], MainAddr));

  // Give the context the list of search paths to use for modules.
  std::vector<SearchPathOptions::SearchPath> ImportPaths;
  for (const auto &path : options.ImportPaths) {
    ImportPaths.push_back({path, /*isSystem=*/false});
  }
  Invocation.setImportSearchPaths(ImportPaths);
  std::vector<SearchPathOptions::SearchPath> FramePaths;
  for (const auto &path : options.FrameworkPaths) {
    FramePaths.push_back({path, /*isSystem=*/false});
  }
  Invocation.setFrameworkSearchPaths(FramePaths);

  Invocation.setVFSOverlays(options.VFSOverlays);

  // Set the SDK path and target if given.
  if (options.SDKPath.getNumOccurrences() == 0) {
    const char *SDKROOT = getenv("SDKROOT");
    if (SDKROOT)
      options.SDKPath = SDKROOT;
  }
  if (!options.SDKPath.empty())
    Invocation.setSDKPath(options.SDKPath);
  if (!options.Target.empty())
    Invocation.setTargetTriple(options.Target);
  if (!options.ResourceDir.empty())
    Invocation.setRuntimeResourcePath(options.ResourceDir);
  Invocation.getFrontendOptions().EnableLibraryEvolution
    = options.EnableLibraryEvolution;
  Invocation.getFrontendOptions().StrictImplicitModuleContext
    = options.StrictImplicitModuleContext;

  Invocation.getFrontendOptions().DisableImplicitModules =
    options.DisableImplicitModules;
  Invocation.getSearchPathOptions().ExplicitCodiraModuleMapPath =
    options.ExplicitCodiraModuleMapPath;

  // Set the module cache path. If not passed in we use the default language module
  // cache.
  Invocation.getClangImporterOptions().ModuleCachePath = options.ModuleCachePath;
  Invocation.setParseStdlib();
  if (options.CodiraVersionString.size()) {
    auto vers = VersionParser::parseVersionString(options.CodiraVersionString,
                                                  SourceLoc(), nullptr);
    bool isValid = false;
    if (vers.has_value()) {
      if (auto effectiveVers = vers.value().getEffectiveLanguageVersion()) {
        Invocation.getLangOptions().EffectiveLanguageVersion =
            effectiveVers.value();
        isValid = true;
      }
    }
    if (!isValid) {
      toolchain::errs() << "error: invalid language version "
                   << options.CodiraVersionString << '\n';
      exit(-1);
    }
  }
  Invocation.getLangOptions().DisableAvailabilityChecking = true;
  Invocation.getLangOptions().EnableAccessControl = false;
  Invocation.getLangOptions().EnableObjCAttrRequiresFoundation = false;
  Invocation.getLangOptions().EnableDeserializationSafety = false;
  if (auto overrideKind = getASTOverrideKind(options)) {
    Invocation.getLangOptions().ASTVerifierOverride = *overrideKind;
  }
  Invocation.getLangOptions().EnableExperimentalConcurrency =
    options.EnableExperimentalConcurrency;
  std::optional<bool> enableExperimentalMoveOnly =
      toOptionalBool(options.EnableExperimentalMoveOnly);
  if (enableExperimentalMoveOnly && *enableExperimentalMoveOnly) {
    // FIXME: drop addition of Feature::MoveOnly once its queries are gone.
    Invocation.getLangOptions().enableFeature(Feature::MoveOnly);
    Invocation.getLangOptions().enableFeature(Feature::NoImplicitCopy);
    Invocation.getLangOptions().enableFeature(
        Feature::OldOwnershipOperatorSpellings);
  }

  Invocation.getLangOptions().BypassResilienceChecks =
      options.BypassResilienceChecks;
  if (options.DebugDiagnosticNames) {
    Invocation.getDiagnosticOptions().PrintDiagnosticNames =
        PrintDiagnosticNamesMode::Identifier;
  }

  for (auto &featureName : options.UpcomingFeatures) {
    auto feature = Feature::getUpcomingFeature(featureName);
    if (!feature) {
      toolchain::errs() << "error: unknown upcoming feature "
                   << QuotedString(featureName) << "\n";
      exit(-1);
    }

    if (auto firstVersion = feature->getLanguageVersion()) {
      if (Invocation.getLangOptions().isCodiraVersionAtLeast(*firstVersion)) {
        toolchain::errs() << "error: upcoming feature " << QuotedString(featureName)
                     << " is already enabled as of Codira version "
                     << *firstVersion << '\n';
        exit(-1);
      }
    }
    Invocation.getLangOptions().enableFeature(*feature);
  }

  for (auto &featureName : options.ExperimentalFeatures) {
    if (auto feature = Feature::getExperimentalFeature(featureName)) {
      Invocation.getLangOptions().enableFeature(*feature);
    } else {
      toolchain::errs() << "error: unknown experimental feature "
                   << QuotedString(featureName) << "\n";
      exit(-1);
    }
  }

  Invocation.getLangOptions().EnableObjCInterop =
    options.EnableObjCInterop ? true :
    options.DisableObjCInterop ? false : toolchain::Triple(options.Target).isOSDarwin();

  Invocation.getLangOptions().OptimizationRemarkPassedPattern =
      createOptRemarkRegex(options.PassRemarksPassed);
  Invocation.getLangOptions().OptimizationRemarkMissedPattern =
      createOptRemarkRegex(options.PassRemarksMissed);

  if (options.EnableExperimentalStaticAssert)
    Invocation.getLangOptions().enableFeature(Feature::StaticAssert);

  if (options.EnableExperimentalDifferentiableProgramming) {
    Invocation.getLangOptions().enableFeature(
        Feature::DifferentiableProgramming);
  }

  Invocation.getLangOptions().EnableCXXInterop = options.EnableCxxInterop;
  Invocation.computeCXXStdlibOptions();

  Invocation.getLangOptions().UnavailableDeclOptimizationMode =
      options.UnavailableDeclOptimization;

  // Enable strict concurrency if we have the feature specified or if it was
  // specified via a command line option to sil-opt.
  if (Invocation.getLangOptions().hasFeature(Feature::StrictConcurrency)) {
    Invocation.getLangOptions().StrictConcurrencyLevel =
        StrictConcurrency::Complete;
  } else if (auto level = convertSILOptToRawStrictConcurrencyLevel(
                 options.StrictConcurrencyLevel)) {
    // If strict concurrency was enabled from the cmdline so the feature flag as
    // well.
    if (*level == StrictConcurrency::Complete)
      Invocation.getLangOptions().enableFeature(Feature::StrictConcurrency);
    Invocation.getLangOptions().StrictConcurrencyLevel = *level;
  }

  // If we have strict concurrency set as a feature and were told to turn off
  // region-based isolation... do so now.
  if (Invocation.getLangOptions().hasFeature(Feature::StrictConcurrency)) {
    Invocation.getLangOptions().enableFeature(Feature::RegionBasedIsolation);
  }

  Invocation.getDiagnosticOptions().VerifyMode =
      options.VerifyMode ? DiagnosticOptions::Verify
                         : DiagnosticOptions::NoVerify;
  for (auto &additionalPrefixes : options.VerifyAdditionalPrefixes) {
    Invocation.getDiagnosticOptions()
        .AdditionalDiagnosticVerifierPrefixes.push_back(additionalPrefixes);
  }

  ClangImporterOptions &clangImporterOptions =
      Invocation.getClangImporterOptions();
  for (const auto &xcc : options.ClangXCC) {
    clangImporterOptions.ExtraArgs.push_back(xcc);
  }

  // Setup the SIL Options.
  SILOptions &SILOpts = Invocation.getSILOptions();
  SILOpts.InlineThreshold = options.SILInlineThreshold;
  SILOpts.VerifyAll = options.SILVerifyAll || options.EnableSILVerifyAll;
  SILOpts.VerifyNone = options.SILVerifyNone;
  SILOpts.RemoveRuntimeAsserts = options.RemoveRuntimeAsserts;
  SILOpts.AssertConfig = options.AssertConfId;
  SILOpts.VerifySILOwnership = !options.DisableSILOwnershipVerifier;
  SILOpts.OptRecordFile = options.RemarksFilename;
  SILOpts.OptRecordPasses = options.RemarksPasses;
  SILOpts.EnableStackProtection = true;
  SILOpts.EnableMoveInoutStackProtection = options.EnableMoveInoutStackProtection;

  SILOpts.VerifyExclusivity = options.VerifyExclusivity;
  if (options.EnforceExclusivity.getNumOccurrences() != 0) {
    switch (options.EnforceExclusivity) {
    case EnforceExclusivityMode::Unchecked:
      // This option is analogous to the -Ounchecked optimization setting.
      // It will disable dynamic checking but still diagnose statically.
      SILOpts.EnforceExclusivityStatic = true;
      SILOpts.EnforceExclusivityDynamic = false;
      break;
    case EnforceExclusivityMode::Checked:
      SILOpts.EnforceExclusivityStatic = true;
      SILOpts.EnforceExclusivityDynamic = true;
      break;
    case EnforceExclusivityMode::DynamicOnly:
      // This option is intended for staging purposes. The intent is that
      // it will eventually be removed.
      SILOpts.EnforceExclusivityStatic = false;
      SILOpts.EnforceExclusivityDynamic = true;
      break;
    case EnforceExclusivityMode::None:
      // This option is for staging purposes.
      SILOpts.EnforceExclusivityStatic = false;
      SILOpts.EnforceExclusivityDynamic = false;
      break;
    }
  }
  SILOpts.EmitVerboseSIL |= options.EmitVerboseSIL;
  SILOpts.EmitSortedSIL |= options.EmitSortedSIL;

  SILOpts.EnableSpeculativeDevirtualization = options.EnableSpeculativeDevirtualization;
  SILOpts.EnableAsyncDemotion = options.EnableAsyncDemotion;
  SILOpts.EnableThrowsPrediction = options.EnableThrowsPrediction;
  SILOpts.EnableNoReturnCold = options.EnableNoReturnCold;
  SILOpts.IgnoreAlwaysInline = options.IgnoreAlwaysInline;
  SILOpts.EnableOSSAModules = options.EnableOSSAModules;
  SILOpts.EnableSILOpaqueValues = options.EnableSILOpaqueValues;
  SILOpts.OSSACompleteLifetimes = options.EnableOSSACompleteLifetimes;
  SILOpts.OSSAVerifyComplete = options.EnableOSSAVerifyComplete;
  SILOpts.StopOptimizationAfterSerialization |= options.EmitSIB;
  if (options.CopyPropagationState) {
    SILOpts.CopyPropagation = *options.CopyPropagationState;
  }

  // Unless overridden below, enabling copy propagation means enabling lexical
  // lifetimes.
  if (SILOpts.CopyPropagation == CopyPropagationOption::On)
    SILOpts.LexicalLifetimes = LexicalLifetimesOption::On;

  // Unless overridden below, disable copy propagation means disabling lexical
  // lifetimes.
  if (SILOpts.CopyPropagation == CopyPropagationOption::Off)
    SILOpts.LexicalLifetimes = LexicalLifetimesOption::DiagnosticMarkersOnly;

  std::optional<bool> enableLexicalLifetimes =
      toOptionalBool(options.EnableLexicalLifetimes);

  if (enableLexicalLifetimes)
    SILOpts.LexicalLifetimes =
        *enableLexicalLifetimes ? LexicalLifetimesOption::On
                                : LexicalLifetimesOption::DiagnosticMarkersOnly;

  SILOpts.EnablePackMetadataStackPromotion =
      options.EnablePackMetadataStackPromotion;

  SILOpts.EnableAddressDependencies = options.EnableAddressDependencies;
  if (options.EnableCalleeAllocatedCoroAbi)
    SILOpts.CoroutineAccessorsUseYieldOnce2 = true;
  if (options.DisableCalleeAllocatedCoroAbi)
    SILOpts.CoroutineAccessorsUseYieldOnce2 = false;
  SILOpts.MergeableTraps = options.MergeableTraps;

  if (options.OptModeFlag == OptimizationMode::NotSet) {
    if (options.OptimizationGroup == OptGroup::Diagnostics)
      SILOpts.OptMode = OptimizationMode::NoOptimization;
    else
      SILOpts.OptMode = OptimizationMode::ForSpeed;
  } else {
    SILOpts.OptMode = options.OptModeFlag;
  }

  auto &IRGenOpts = Invocation.getIRGenOptions();
  if (options.OptModeFlag == OptimizationMode::NotSet) {
    if (options.OptimizationGroup == OptGroup::Diagnostics)
      IRGenOpts.OptMode = OptimizationMode::NoOptimization;
    else
      IRGenOpts.OptMode = OptimizationMode::ForSpeed;
  } else {
    IRGenOpts.OptMode = options.OptModeFlag;
  }
  IRGenOpts.DebugInfoLevel = options.IRGenDebugInfoLevelArg;

  // Note: SILOpts, LangOpts, and IRGenOpts must be set before the
  // CompilerInstance is initializer below based on Invocation.

  serialization::ExtendedValidationInfo extendedInfo;
  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> FileBufOrErr =
      Invocation.setUpInputForSILTool(options.InputFilename, options.ModuleName,
                                      /*alwaysSetModuleToMain*/ false,
                                      /*bePrimary*/ !options.PerformWMO, extendedInfo);
  if (!FileBufOrErr) {
    fprintf(stderr, "Error! Failed to open file: %s\n", options.InputFilename.c_str());
    exit(-1);
  }

  CompilerInstance CI;
  PrintingDiagnosticConsumer PrintDiags;
  CI.addDiagnosticConsumer(&PrintDiags);

  if (options.VerifyMode)
    PrintDiags.setSuppressOutput(true);

  struct FinishDiagProcessingCheckRAII {
    bool CalledFinishDiagProcessing = false;
    ~FinishDiagProcessingCheckRAII() {
      assert(CalledFinishDiagProcessing &&
             "returned from the function "
             "without calling finishDiagProcessing");
    }
  } FinishDiagProcessingCheckRAII;

  auto finishDiagProcessing = [&](int retValue) -> int {
    FinishDiagProcessingCheckRAII.CalledFinishDiagProcessing = true;
    PrintDiags.setSuppressOutput(false);
    bool diagnosticsError = CI.getDiags().finishProcessing();
    // If the verifier is enabled and did not encounter any verification errors,
    // return 0 even if the compile failed. This behavior isn't ideal, but large
    // parts of the test suite are reliant on it.
    if (options.VerifyMode && !diagnosticsError) {
      return 0;
    }
    return retValue ? retValue : diagnosticsError;
  };

  std::string InstanceSetupError;
  if (CI.setup(Invocation, InstanceSetupError)) {
    toolchain::errs() << InstanceSetupError << '\n';
    // Rather than finish Diag processing, exit -1 here to show we failed to
    // setup here. The reason we do this is if the setup fails, we want to fail
    // hard. We shouldn't be testing that we setup correctly with
    // -verify/etc. We should be testing that later.
    exit(-1);
  }

  CI.performSema();

  // If parsing produced an error, don't run any passes.
  bool HadError = CI.getASTContext().hadError();
  if (HadError)
    return finishDiagProcessing(1);

  auto *mod = CI.getMainModule();
  assert(mod->getFiles().size() == 1);

  std::unique_ptr<SILModule> SILMod;
  if (options.PerformWMO) {
    SILMod = performASTLowering(mod, CI.getSILTypes(), CI.getSILOptions());
  } else {
    SILMod = performASTLowering(*mod->getFiles()[0], CI.getSILTypes(),
                                CI.getSILOptions());
  }
  SILMod->setSerializeSILAction([]{});

  if (!options.RemarksFilename.empty()) {
    toolchain::Expected<toolchain::remarks::Format> formatOrErr =
        toolchain::remarks::parseFormat(options.RemarksFormat);
    if (toolchain::Error E = formatOrErr.takeError()) {
      CI.getDiags().diagnose(SourceLoc(),
                             diag::error_creating_remark_serializer,
                             toString(std::move(E)));
      HadError = true;
      SILOpts.OptRecordFormat = toolchain::remarks::Format::YAML;
    } else {
      SILOpts.OptRecordFormat = *formatOrErr;
    }

    SILMod->installSILRemarkStreamer();
  }

  switch (options.OptimizationGroup) {
  case OptGroup::Diagnostics:
    runSILDiagnosticPasses(*SILMod.get());
    break;
  case OptGroup::Performance:
    runSILOptimizationPasses(*SILMod.get());
    break;
  case OptGroup::Lowering:
    runSILLoweringPasses(*SILMod.get());
    break;
  case OptGroup::OnonePerformance:
    runSILPassesForOnone(*SILMod.get());
    break;
  case OptGroup::Unknown: {
    auto T = irgen::createIRGenModule(
        SILMod.get(), Invocation.getOutputFilenameForAtMostOnePrimary(),
        Invocation.getMainInputFilenameForDebugInfoForAtMostOnePrimary(), "",
        IRGenOpts);
    runCommandLineSelectedPasses(SILMod.get(), T.second, options);
    irgen::deleteIRGenModule(T);
    break;
  }
  }

  if (options.EmitSIB || options.Serialize) {
    toolchain::SmallString<128> OutputFile;
    if (options.OutputFilename.size()) {
      OutputFile = options.OutputFilename;
    } else if (options.ModuleName.size()) {
      OutputFile = options.ModuleName;
      toolchain::sys::path::replace_extension(
          OutputFile, file_types::getExtension(file_types::TY_SIB));
    } else {
      OutputFile = CI.getMainModule()->getName().str();
      toolchain::sys::path::replace_extension(
          OutputFile, file_types::getExtension(file_types::TY_SIB));
    }

    SerializationOptions serializationOpts;
    serializationOpts.OutputPath = OutputFile;
    serializationOpts.SerializeAllSIL = options.EmitSIB;
    serializationOpts.IsSIB = options.EmitSIB;
    serializationOpts.IsOSSA = SILOpts.EnableOSSAModules;

    symbolgraphgen::SymbolGraphOptions symbolGraphOptions;

    serialize(CI.getMainModule(), serializationOpts, symbolGraphOptions, SILMod.get());
  } else {
    const StringRef OutputFile = options.OutputFilename.size() ?
                                   StringRef(options.OutputFilename) : "-";
    auto SILOpts = SILOptions();
    SILOpts.EmitVerboseSIL = options.EmitVerboseSIL;
    SILOpts.EmitSortedSIL = options.EmitSortedSIL;
    if (OutputFile == "-") {
      SILMod->print(toolchain::outs(), CI.getMainModule(), SILOpts, !options.DisableASTDump);
    } else {
      std::error_code EC;
      toolchain::raw_fd_ostream OS(OutputFile, EC, toolchain::sys::fs::OF_None);
      if (EC) {
        toolchain::errs() << "while opening '" << OutputFile << "': "
                     << EC.message() << '\n';
        return finishDiagProcessing(1);
      }
      SILMod->print(OS, CI.getMainModule(), SILOpts, !options.DisableASTDump);
    }
  }

  HadError |= CI.getASTContext().hadError();

  if (options.VerifyMode) {
    DiagnosticEngine &diags = CI.getDiags();
    if (diags.hasFatalErrorOccurred() &&
        !Invocation.getDiagnosticOptions().ShowDiagnosticsAfterFatalError) {
      diags.resetHadAnyError();
      diags.diagnose(SourceLoc(), diag::verify_encountered_fatal);
      HadError = true;
    }
  }

  return finishDiagProcessing(HadError);
}
