//===--- FrontendOptions.cpp ----------------------------------------------===//
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

#include "language/Frontend/FrontendOptions.h"

#include "language/AST/DiagnosticsFrontend.h"
#include "language/AST/ModuleLoader.h"
#include "language/Basic/Assertions.h"
#include "language/Option/Options.h"
#include "language/Parse/Lexer.h"
#include "language/Strings.h"
#include "toolchain/Option/Arg.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Option/Option.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/LineIterator.h"
#include "toolchain/Support/Path.h"

using namespace language;
using namespace toolchain::opt;

bool FrontendOptions::needsProperModuleName(ActionType action) {
  switch (action) {
  case ActionType::NoneAction:
  case ActionType::Parse:
  case ActionType::ResolveImports:
  case ActionType::Typecheck:
  case ActionType::DumpParse:
  case ActionType::DumpAST:
  case ActionType::DumpInterfaceHash:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::DumpPCM:
  case ActionType::EmitPCH:
    return false;
  case ActionType::EmitSILGen:
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitSIBGen:
  case ActionType::EmitSIB:
  case ActionType::EmitModuleOnly:
  case ActionType::MergeModules:
  case ActionType::CompileModuleFromInterface:
  case ActionType::TypecheckModuleFromInterface:
    return true;
  case ActionType::Immediate:
  case ActionType::REPL:
  case ActionType::PrintVersion:
  case ActionType::PrintArguments:
    return false;
  case ActionType::EmitAssembly:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::EmitBC:
  case ActionType::EmitObject:
  case ActionType::EmitImportedModules:
  case ActionType::DumpTypeInfo:
  case ActionType::EmitPCM:
  case ActionType::ScanDependencies:
    return true;
  }
  toolchain_unreachable("Unknown ActionType");
}

bool FrontendOptions::shouldActionOnlyParse(ActionType action) {
  switch (action) {
  case ActionType::Parse:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::EmitImportedModules:
  case ActionType::ScanDependencies:
  case ActionType::PrintVersion:
  case ActionType::PrintArguments:
    return true;
  default:
    return false;
  }
}

bool FrontendOptions::doesActionRequireCodiraStandardLibrary(ActionType action) {
  switch (action) {
  case ActionType::NoneAction:
  case ActionType::Parse:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::EmitImportedModules:
  case ActionType::ScanDependencies:
  case ActionType::PrintVersion:
  case ActionType::EmitPCH:
  case ActionType::EmitPCM:
  case ActionType::DumpPCM:
  case ActionType::CompileModuleFromInterface:
  case ActionType::TypecheckModuleFromInterface:
  case ActionType::PrintArguments:
    return false;
  case ActionType::ResolveImports:
  case ActionType::Typecheck:
  case ActionType::DumpAST:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::EmitSILGen:
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitModuleOnly:
  case ActionType::MergeModules:
  case ActionType::EmitSIBGen:
  case ActionType::EmitSIB:
  case ActionType::Immediate:
  case ActionType::REPL:
  case ActionType::EmitAssembly:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::EmitBC:
  case ActionType::EmitObject:
  case ActionType::DumpTypeInfo:
    assert(!FrontendOptions::shouldActionOnlyParse(action) &&
           "Parse-only actions should not load modules!");
    return true;
  }
  toolchain_unreachable("Unknown ActionType");
}

bool FrontendOptions::doesActionRequireInputs(ActionType action) {
  switch (action) {
  case ActionType::NoneAction:
  case ActionType::PrintVersion:
  case ActionType::PrintArguments:
    return false;
  case ActionType::REPL:
  case ActionType::Parse:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::EmitImportedModules:
  case ActionType::ScanDependencies:
  case ActionType::EmitPCH:
  case ActionType::EmitPCM:
  case ActionType::DumpPCM:
  case ActionType::CompileModuleFromInterface:
  case ActionType::TypecheckModuleFromInterface:
  case ActionType::ResolveImports:
  case ActionType::Typecheck:
  case ActionType::DumpAST:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::EmitSILGen:
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitModuleOnly:
  case ActionType::MergeModules:
  case ActionType::EmitSIBGen:
  case ActionType::EmitSIB:
  case ActionType::Immediate:
  case ActionType::EmitAssembly:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::EmitBC:
  case ActionType::EmitObject:
  case ActionType::DumpTypeInfo:
    return true;
  }
  toolchain_unreachable("Unknown ActionType");
}

bool FrontendOptions::doesActionPerformEndOfPipelineActions(ActionType action) {
  switch (action) {
  case ActionType::NoneAction:
  case ActionType::PrintVersion:
  case ActionType::PrintArguments:
  case ActionType::EmitPCH:
  case ActionType::EmitPCM:
  case ActionType::DumpPCM:
    return false;
  case ActionType::REPL:
  case ActionType::Parse:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::EmitImportedModules:
  case ActionType::ScanDependencies:
  case ActionType::CompileModuleFromInterface:
  case ActionType::TypecheckModuleFromInterface:
  case ActionType::ResolveImports:
  case ActionType::Typecheck:
  case ActionType::DumpAST:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::EmitSILGen:
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitModuleOnly:
  case ActionType::MergeModules:
  case ActionType::EmitSIBGen:
  case ActionType::EmitSIB:
  case ActionType::Immediate:
  case ActionType::EmitAssembly:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::EmitBC:
  case ActionType::EmitObject:
  case ActionType::DumpTypeInfo:
    return true;
  }
  toolchain_unreachable("Unknown ActionType");
}

bool FrontendOptions::supportCompilationCaching(ActionType action) {
  // TODO: need to audit this list to make sure everything marked as true are
  // all supported and tested.
  switch (action) {
  case ActionType::NoneAction:
  case ActionType::PrintVersion:
  case ActionType::PrintArguments:
  case ActionType::DumpPCM:
  case ActionType::REPL:
  case ActionType::Parse:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::EmitImportedModules:
  case ActionType::ScanDependencies:
  case ActionType::ResolveImports:
  case ActionType::DumpAST:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::MergeModules:
  case ActionType::Immediate:
  case ActionType::DumpTypeInfo:
    return false;
  case ActionType::Typecheck:
  case ActionType::TypecheckModuleFromInterface:
  case ActionType::CompileModuleFromInterface:
  case ActionType::EmitPCH:
  case ActionType::EmitPCM:
  case ActionType::EmitAssembly:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::EmitBC:
  case ActionType::EmitObject:
  case ActionType::EmitSILGen:
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitModuleOnly:
  case ActionType::EmitSIBGen:
  case ActionType::EmitSIB:
    return true;
  }
  toolchain_unreachable("Unknown ActionType");
}

void FrontendOptions::forAllOutputPaths(
    const InputFile &input, toolchain::function_ref<void(StringRef)> fn) const {
  if (RequestedAction != FrontendOptions::ActionType::EmitModuleOnly &&
      RequestedAction != FrontendOptions::ActionType::MergeModules) {
    if (InputsAndOutputs.isWholeModule())
      InputsAndOutputs.forEachOutputFilename(fn);
    else
      fn(input.outputFilename());
  }
  const SupplementaryOutputPaths &outs =
      input.getPrimarySpecificPaths().SupplementaryOutputs;
  const std::string *outputs[] = {
      &outs.ModuleOutputPath,          &outs.ModuleDocOutputPath,
      &outs.ModuleInterfaceOutputPath, &outs.PrivateModuleInterfaceOutputPath,
      &outs.PackageModuleInterfaceOutputPath, &outs.ClangHeaderOutputPath,
      &outs.ModuleSourceInfoOutputPath};
  for (const std::string *next : outputs) {
    if (!next->empty())
      fn(*next);
  }
}

file_types::ID
FrontendOptions::formatForPrincipalOutputFileForAction(ActionType action) {
  using namespace file_types;

  switch (action) {
  case ActionType::NoneAction:
    return TY_Nothing;

  case ActionType::Parse:
  case ActionType::ResolveImports:
  case ActionType::Typecheck:
  case ActionType::TypecheckModuleFromInterface:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::DumpAST:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::DumpTypeInfo:
  case ActionType::DumpPCM:
  case ActionType::PrintVersion:
    return TY_Nothing;

  case ActionType::EmitPCH:
    return TY_PCH;

  case ActionType::EmitSILGen:
    return TY_RawSIL;

  case ActionType::EmitSIL:
    return TY_SIL;

  case ActionType::EmitLoweredSIL:
    return TY_LoweredSIL;

  case ActionType::EmitSIBGen:
    return TY_RawSIB;

  case ActionType::EmitSIB:
    return TY_SIB;

  case ActionType::MergeModules:
  case ActionType::EmitModuleOnly:
  case ActionType::CompileModuleFromInterface:
    return TY_CodiraModuleFile;

  case ActionType::Immediate:
  case ActionType::REPL:
    // These modes have no frontend-generated output.
    return TY_Nothing;

  case ActionType::EmitAssembly:
    return TY_Assembly;

  case ActionType::EmitIRGen:
    return TY_RawTOOLCHAIN_IR;

  case ActionType::EmitIR:
    return TY_TOOLCHAIN_IR;

  case ActionType::EmitBC:
    return TY_TOOLCHAIN_BC;

  case ActionType::EmitObject:
    return TY_Object;

  case ActionType::EmitImportedModules:
    return TY_ImportedModules;

  case ActionType::EmitPCM:
    return TY_ClangModuleFile;

  case ActionType::ScanDependencies:
    return TY_JSONDependencies;
  case ActionType::PrintArguments:
    return TY_JSONArguments;
  }
  toolchain_unreachable("unhandled action");
}

bool FrontendOptions::canActionEmitDependencies(ActionType action) {
  switch (action) {
  case ActionType::NoneAction:
  case ActionType::Parse:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::DumpAST:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::DumpTypeInfo:
  case ActionType::CompileModuleFromInterface:
  case ActionType::TypecheckModuleFromInterface:
  case ActionType::Immediate:
  case ActionType::REPL:
  case ActionType::DumpPCM:
  case ActionType::PrintVersion:
  case ActionType::PrintArguments:
    return false;
  case ActionType::ResolveImports:
  case ActionType::Typecheck:
  case ActionType::MergeModules:
  case ActionType::EmitModuleOnly:
  case ActionType::EmitPCH:
  case ActionType::EmitSILGen:
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitSIBGen:
  case ActionType::EmitSIB:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::EmitBC:
  case ActionType::EmitAssembly:
  case ActionType::EmitObject:
  case ActionType::EmitImportedModules:
  case ActionType::EmitPCM:
  case ActionType::ScanDependencies:
    return true;
  }
  toolchain_unreachable("unhandled action");
}

bool FrontendOptions::canActionEmitReferenceDependencies(ActionType action) {
  switch (action) {
  case ActionType::NoneAction:
  case ActionType::Parse:
  case ActionType::ResolveImports:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::DumpAST:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::DumpTypeInfo:
  case ActionType::CompileModuleFromInterface:
  case ActionType::TypecheckModuleFromInterface:
  case ActionType::Immediate:
  case ActionType::REPL:
  case ActionType::EmitPCM:
  case ActionType::DumpPCM:
  case ActionType::ScanDependencies:
  case ActionType::PrintVersion:
  case ActionType::PrintArguments:
    return false;
  case ActionType::Typecheck:
  case ActionType::MergeModules:
  case ActionType::EmitModuleOnly:
  case ActionType::EmitPCH:
  case ActionType::EmitSILGen:
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitSIBGen:
  case ActionType::EmitSIB:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::EmitBC:
  case ActionType::EmitAssembly:
  case ActionType::EmitObject:
  case ActionType::EmitImportedModules:
    return true;
  }
  toolchain_unreachable("unhandled action");
}

bool FrontendOptions::canActionEmitModuleSummary(ActionType action) {
  switch (action) {
  case ActionType::NoneAction:
  case ActionType::Parse:
  case ActionType::ResolveImports:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::DumpAST:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::EmitImportedModules:
  case ActionType::EmitPCH:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::DumpTypeInfo:
  case ActionType::EmitSILGen:
  case ActionType::EmitSIBGen:
  case ActionType::CompileModuleFromInterface:
  case ActionType::TypecheckModuleFromInterface:
  case ActionType::Immediate:
  case ActionType::REPL:
  case ActionType::EmitPCM:
  case ActionType::DumpPCM:
  case ActionType::ScanDependencies:
  case ActionType::Typecheck:
  case ActionType::MergeModules:
  case ActionType::EmitModuleOnly:
  case ActionType::PrintVersion:
  case ActionType::PrintArguments:
    return false;
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitSIB:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::EmitBC:
  case ActionType::EmitAssembly:
  case ActionType::EmitObject:
    return true;
  }
  toolchain_unreachable("unhandled action");
}

bool FrontendOptions::canActionEmitClangHeader(ActionType action) {
  switch (action) {
  case ActionType::NoneAction:
  case ActionType::Parse:
  case ActionType::ResolveImports:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::DumpAST:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::EmitPCH:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::DumpTypeInfo:
  case ActionType::CompileModuleFromInterface:
  case ActionType::TypecheckModuleFromInterface:
  case ActionType::Immediate:
  case ActionType::REPL:
  case ActionType::EmitPCM:
  case ActionType::DumpPCM:
  case ActionType::ScanDependencies:
  case ActionType::PrintVersion:
  case ActionType::PrintArguments:
    return false;
  case ActionType::Typecheck:
  case ActionType::MergeModules:
  case ActionType::EmitModuleOnly:
  case ActionType::EmitSILGen:
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitSIBGen:
  case ActionType::EmitSIB:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::EmitBC:
  case ActionType::EmitAssembly:
  case ActionType::EmitObject:
  case ActionType::EmitImportedModules:
    return true;
  }
  toolchain_unreachable("unhandled action");
}

bool FrontendOptions::canActionEmitLoadedModuleTrace(ActionType action) {
  switch (action) {
  case ActionType::NoneAction:
  case ActionType::Parse:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::DumpAST:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::DumpTypeInfo:
  case ActionType::CompileModuleFromInterface:
  case ActionType::TypecheckModuleFromInterface:
  case ActionType::Immediate:
  case ActionType::REPL:
  case ActionType::EmitPCM:
  case ActionType::DumpPCM:
  case ActionType::ScanDependencies:
  case ActionType::PrintVersion:
  case ActionType::PrintArguments:
    return false;
  case ActionType::ResolveImports:
  case ActionType::Typecheck:
  case ActionType::MergeModules:
  case ActionType::EmitModuleOnly:
  case ActionType::EmitPCH:
  case ActionType::EmitSILGen:
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitSIBGen:
  case ActionType::EmitSIB:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::EmitBC:
  case ActionType::EmitAssembly:
  case ActionType::EmitObject:
  case ActionType::EmitImportedModules:
    return true;
  }
  toolchain_unreachable("unhandled action");
}
bool FrontendOptions::canActionEmitModuleSemanticInfo(ActionType action) {
  switch (action) {
  case ActionType::MergeModules:
  case ActionType::EmitModuleOnly:
  case ActionType::CompileModuleFromInterface:
  // For test
  case ActionType::Typecheck:
    return true;
  case ActionType::NoneAction:
  case ActionType::Parse:
  case ActionType::ResolveImports:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::DumpAST:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::EmitPCH:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::DumpTypeInfo:
  case ActionType::EmitSILGen:
  case ActionType::TypecheckModuleFromInterface:
  case ActionType::Immediate:
  case ActionType::REPL:
  case ActionType::EmitPCM:
  case ActionType::DumpPCM:
  case ActionType::ScanDependencies:
  case ActionType::PrintVersion:
  case ActionType::PrintArguments:
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitSIBGen:
  case ActionType::EmitSIB:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::EmitBC:
  case ActionType::EmitAssembly:
  case ActionType::EmitObject:
  case ActionType::EmitImportedModules:
    return false;
  }
  toolchain_unreachable("unhandled action");
}
bool FrontendOptions::canActionEmitABIDescriptor(ActionType action) {
  if (canActionEmitModule(action))
    return true;
  if (action == ActionType::CompileModuleFromInterface)
    return true;
  return false;
}
bool FrontendOptions::canActionEmitConstValues(ActionType action) {
  switch (action) {
  case ActionType::NoneAction:
  case ActionType::Parse:
  case ActionType::ResolveImports:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::DumpAST:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::DumpTypeInfo:
  case ActionType::CompileModuleFromInterface:
  case ActionType::TypecheckModuleFromInterface:
  case ActionType::Immediate:
  case ActionType::REPL:
  case ActionType::EmitPCM:
  case ActionType::DumpPCM:
  case ActionType::ScanDependencies:
  case ActionType::PrintVersion:
  case ActionType::PrintArguments:
    return false;
  case ActionType::Typecheck:
  case ActionType::MergeModules:
  case ActionType::EmitModuleOnly:
  case ActionType::EmitPCH:
  case ActionType::EmitSILGen:
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitSIBGen:
  case ActionType::EmitSIB:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::EmitBC:
  case ActionType::EmitAssembly:
  case ActionType::EmitObject:
  case ActionType::EmitImportedModules:
    return true;
  }
  toolchain_unreachable("unhandled action");
}
bool FrontendOptions::canActionEmitModule(ActionType action) {
  switch (action) {
  case ActionType::NoneAction:
  case ActionType::Parse:
  case ActionType::ResolveImports:
  case ActionType::Typecheck:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::DumpAST:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::EmitPCH:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::DumpTypeInfo:
  case ActionType::EmitSILGen:
  case ActionType::CompileModuleFromInterface:
  case ActionType::TypecheckModuleFromInterface:
  case ActionType::Immediate:
  case ActionType::REPL:
  case ActionType::EmitPCM:
  case ActionType::DumpPCM:
  case ActionType::ScanDependencies:
  case ActionType::PrintVersion:
  case ActionType::PrintArguments:
    return false;
  case ActionType::MergeModules:
  case ActionType::EmitModuleOnly:
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitSIBGen:
  case ActionType::EmitSIB:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::EmitBC:
  case ActionType::EmitAssembly:
  case ActionType::EmitObject:
  case ActionType::EmitImportedModules:
    return true;
  }
  toolchain_unreachable("unhandled action");
}

bool FrontendOptions::canActionEmitModuleDoc(ActionType action) {
  return canActionEmitModule(action);
}

bool FrontendOptions::canActionEmitInterface(ActionType action) {
  switch (action) {
  case ActionType::NoneAction:
  case ActionType::Parse:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::DumpAST:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::EmitImportedModules:
  case ActionType::EmitPCH:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::DumpTypeInfo:
  case ActionType::EmitSILGen:
  case ActionType::EmitSIBGen:
  case ActionType::CompileModuleFromInterface:
  case ActionType::TypecheckModuleFromInterface:
  case ActionType::Immediate:
  case ActionType::REPL:
  case ActionType::EmitPCM:
  case ActionType::DumpPCM:
  case ActionType::ScanDependencies:
  case ActionType::PrintArguments:
    return false;
  case ActionType::ResolveImports:
  case ActionType::Typecheck:
  case ActionType::MergeModules:
  case ActionType::EmitModuleOnly:
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitSIB:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::EmitBC:
  case ActionType::EmitAssembly:
  case ActionType::EmitObject:
  case ActionType::PrintVersion:
    return true;
  }
  toolchain_unreachable("unhandled action");
}

bool FrontendOptions::canActionEmitAPIDescriptor(ActionType action) {
  switch (action) {
  case ActionType::NoneAction:
  case ActionType::Parse:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::DumpAST:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::EmitImportedModules:
  case ActionType::EmitPCH:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::DumpTypeInfo:
  case ActionType::EmitSILGen:
  case ActionType::EmitSIBGen:
  case ActionType::CompileModuleFromInterface:
  case ActionType::TypecheckModuleFromInterface:
  case ActionType::Immediate:
  case ActionType::REPL:
  case ActionType::EmitPCM:
  case ActionType::DumpPCM:
  case ActionType::ScanDependencies:
  case ActionType::PrintArguments:
    return false;
  case ActionType::ResolveImports:
  case ActionType::Typecheck:
  case ActionType::MergeModules:
  case ActionType::EmitModuleOnly:
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitSIB:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::EmitBC:
  case ActionType::EmitAssembly:
  case ActionType::EmitObject:
  case ActionType::PrintVersion:
    return true;
  }
  toolchain_unreachable("unhandled action");
}

bool FrontendOptions::doesActionProduceOutput(ActionType action) {
  switch (action) {
  case ActionType::Parse:
  case ActionType::ResolveImports:
  case ActionType::Typecheck:
  case ActionType::DumpParse:
  case ActionType::DumpAST:
  case ActionType::DumpInterfaceHash:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::EmitPCH:
  case ActionType::EmitSILGen:
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitSIBGen:
  case ActionType::EmitSIB:
  case ActionType::EmitModuleOnly:
  case ActionType::EmitAssembly:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::EmitBC:
  case ActionType::EmitObject:
  case ActionType::EmitImportedModules:
  case ActionType::MergeModules:
  case ActionType::CompileModuleFromInterface:
  case ActionType::DumpTypeInfo:
  case ActionType::EmitPCM:
  case ActionType::DumpPCM:
  case ActionType::ScanDependencies:
  case ActionType::PrintArguments:
    return true;

  case ActionType::TypecheckModuleFromInterface:
  case ActionType::NoneAction:
  case ActionType::Immediate:
  case ActionType::REPL:
  case ActionType::PrintVersion:
    return false;
  }
  toolchain_unreachable("Unknown ActionType");
}

bool FrontendOptions::doesActionProduceTextualOutput(ActionType action) {
  switch (action) {
  case ActionType::NoneAction:
  case ActionType::EmitPCH:
  case ActionType::EmitSIBGen:
  case ActionType::EmitSIB:
  case ActionType::MergeModules:
  case ActionType::EmitModuleOnly:
  case ActionType::CompileModuleFromInterface:
  case ActionType::EmitBC:
  case ActionType::EmitObject:
  case ActionType::Immediate:
  case ActionType::REPL:
  case ActionType::EmitPCM:
  case ActionType::TypecheckModuleFromInterface:
    return false;

  case ActionType::Parse:
  case ActionType::ResolveImports:
  case ActionType::Typecheck:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::DumpAST:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::EmitImportedModules:
  case ActionType::EmitSILGen:
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitAssembly:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::DumpTypeInfo:
  case ActionType::DumpPCM:
  case ActionType::ScanDependencies:
  case ActionType::PrintVersion:
  case ActionType::PrintArguments:
    return true;
  }
  toolchain_unreachable("unhandled action");
}

bool FrontendOptions::doesActionGenerateSIL(ActionType action) {
  switch (action) {
  case ActionType::NoneAction:
  case ActionType::Parse:
  case ActionType::ResolveImports:
  case ActionType::Typecheck:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::DumpAST:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::EmitImportedModules:
  case ActionType::EmitPCH:
  case ActionType::CompileModuleFromInterface:
  case ActionType::TypecheckModuleFromInterface:
  case ActionType::EmitPCM:
  case ActionType::DumpPCM:
  case ActionType::ScanDependencies:
  case ActionType::PrintVersion:
  case ActionType::PrintArguments:
    return false;
  case ActionType::EmitSILGen:
  case ActionType::EmitSIBGen:
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitSIB:
  case ActionType::EmitModuleOnly:
  case ActionType::MergeModules:
  case ActionType::Immediate:
  case ActionType::REPL:
  case ActionType::EmitAssembly:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::EmitBC:
  case ActionType::EmitObject:
  case ActionType::DumpTypeInfo:
    return true;
  }
  toolchain_unreachable("unhandled action");
}

bool FrontendOptions::doesActionGenerateIR(ActionType action) {
  switch (action) {
  case ActionType::NoneAction:
  case ActionType::Parse:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::DumpAST:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::DumpTypeInfo:
  case ActionType::CompileModuleFromInterface:
  case ActionType::TypecheckModuleFromInterface:
  case ActionType::Typecheck:
  case ActionType::ResolveImports:
  case ActionType::MergeModules:
  case ActionType::EmitModuleOnly:
  case ActionType::EmitPCH:
  case ActionType::EmitSILGen:
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitSIBGen:
  case ActionType::EmitSIB:
  case ActionType::EmitImportedModules:
  case ActionType::EmitPCM:
  case ActionType::DumpPCM:
  case ActionType::ScanDependencies:
  case ActionType::PrintVersion:
  case ActionType::PrintArguments:
    return false;
  case ActionType::Immediate:
  case ActionType::REPL:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::EmitBC:
  case ActionType::EmitAssembly:
  case ActionType::EmitObject:
    return true;
  }
  toolchain_unreachable("unhandled action");
}

bool FrontendOptions::doesActionBuildModuleFromInterface(ActionType action) {
  switch (action) {
  case ActionType::CompileModuleFromInterface:
  case ActionType::TypecheckModuleFromInterface:
    return true;
  case ActionType::NoneAction:
  case ActionType::Parse:
  case ActionType::DumpParse:
  case ActionType::DumpInterfaceHash:
  case ActionType::DumpAST:
  case ActionType::PrintAST:
  case ActionType::PrintASTDecl:
  case ActionType::DumpScopeMaps:
  case ActionType::DumpAvailabilityScopes:
  case ActionType::DumpTypeInfo:
  case ActionType::Typecheck:
  case ActionType::ResolveImports:
  case ActionType::MergeModules:
  case ActionType::EmitModuleOnly:
  case ActionType::EmitPCH:
  case ActionType::EmitSILGen:
  case ActionType::EmitSIL:
  case ActionType::EmitLoweredSIL:
  case ActionType::EmitSIBGen:
  case ActionType::EmitSIB:
  case ActionType::EmitImportedModules:
  case ActionType::EmitPCM:
  case ActionType::DumpPCM:
  case ActionType::ScanDependencies:
  case ActionType::PrintVersion:
  case ActionType::PrintArguments:
  case ActionType::Immediate:
  case ActionType::REPL:
  case ActionType::EmitIRGen:
  case ActionType::EmitIR:
  case ActionType::EmitBC:
  case ActionType::EmitAssembly:
  case ActionType::EmitObject:
    return false;
  }
  toolchain_unreachable("unhandled action");
}

const PrimarySpecificPaths &
FrontendOptions::getPrimarySpecificPathsForAtMostOnePrimary() const {
  return InputsAndOutputs.getPrimarySpecificPathsForAtMostOnePrimary();
}

const PrimarySpecificPaths &
FrontendOptions::getPrimarySpecificPathsForPrimary(StringRef filename) const {
  return InputsAndOutputs.getPrimarySpecificPathsForPrimary(filename);
}

bool FrontendOptions::shouldTrackSystemDependencies() const {
  return IntermoduleDependencyTracking ==
         IntermoduleDepTrackingMode::IncludeSystem;
}

bool FrontendOptions::isTypeCheckAction() const {
  return RequestedAction == FrontendOptions::ActionType::Typecheck ||
  RequestedAction == FrontendOptions::ActionType::TypecheckModuleFromInterface;
}
