//===--- FileTypes.cpp - Input & output formats used by the tools ---------===//
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

#include "language/Basic/Assertions.h"
#include "language/Basic/FileTypes.h"

#include "language/Strings.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Support/ErrorHandling.h"

using namespace language;
using namespace language::file_types;

namespace {
struct TypeInfo {
  const char *Name;
  const char *Flags;
  const char *Extension;
};
} // end anonymous namespace

static const TypeInfo TypeInfos[] = {
#define TYPE(NAME, ID, EXTENSION, FLAGS) \
  { NAME, FLAGS, EXTENSION },
#include "language/Basic/FileTypes.def"
};

static const TypeInfo &getInfo(unsigned Id) {
  assert(Id >= 0 && Id < TY_INVALID && "Invalid Type ID.");
  return TypeInfos[Id];
}

StringRef file_types::getTypeName(ID Id) { return getInfo(Id).Name; }

StringRef file_types::getExtension(ID Id) {
  return getInfo(Id).Extension;
}

ID file_types::lookupTypeForExtension(StringRef Ext) {
  if (Ext.empty())
    return TY_INVALID;
  assert(Ext.front() == '.' && "not a file extension");
  return toolchain::StringSwitch<file_types::ID>(Ext.drop_front())
#define TYPE(NAME, ID, EXTENSION, FLAGS) \
           .Case(EXTENSION, TY_##ID)
#include "language/Basic/FileTypes.def"
      .Default(TY_INVALID);
}

// Compute the file type from filename. This handles the lookup for extensions
// with multiple dots, like `.private.codeinterface` correctly.
ID file_types::lookupTypeFromFilename(StringRef Filename) {
  StringRef MaybeExt = Filename;
  // Search from leftmost `.`, return the first match or till all dots are
  // consumed.
  size_t Pos = MaybeExt.find_first_of('.');
  while(Pos != StringRef::npos) {
    MaybeExt = MaybeExt.substr(Pos);
    // If size is 1, that means only `.` is left, return invalid.
    if (MaybeExt.size() == 1)
      return TY_INVALID;
    ID Type = lookupTypeForExtension(MaybeExt);
    if (Type != TY_INVALID)
      return Type;
    // Drop `.` and keep looking.
    MaybeExt = MaybeExt.drop_front();
    Pos = MaybeExt.find_first_of('.');
  }

  return TY_INVALID;
}

ID file_types::lookupTypeForName(StringRef Name) {
  return toolchain::StringSwitch<file_types::ID>(Name)
#define TYPE(NAME, ID, EXTENSION, FLAGS) \
           .Case(NAME, TY_##ID)
#include "language/Basic/FileTypes.def"
      .Case("objc-header", TY_ClangHeader)
      .Default(TY_INVALID);
}

bool file_types::isTextual(ID Id) {
  switch (Id) {
  case file_types::TY_Codira:
  case file_types::TY_SIL:
  case file_types::TY_LoweredSIL:
  case file_types::TY_Dependencies:
  case file_types::TY_Assembly:
  case file_types::TY_ASTDump:
  case file_types::TY_RawSIL:
  case file_types::TY_RawTOOLCHAIN_IR:
  case file_types::TY_TOOLCHAIN_IR:
  case file_types::TY_ClangHeader:
  case file_types::TY_AutolinkFile:
  case file_types::TY_ImportedModules:
  case file_types::TY_TBD:
  case file_types::TY_ModuleTrace:
  case file_types::TY_FineModuleTrace:
  case file_types::TY_YAMLOptRecord:
  case file_types::TY_CodiraModuleInterfaceFile:
  case file_types::TY_PrivateCodiraModuleInterfaceFile:
  case file_types::TY_PackageCodiraModuleInterfaceFile:
  case file_types::TY_CodiraOverlayFile:
  case file_types::TY_JSONDependencies:
  case file_types::TY_JSONArguments:
  case file_types::TY_CodiraABIDescriptor:
  case file_types::TY_CodiraAPIDescriptor:
  case file_types::TY_ConstValues:
  case file_types::TY_SymbolGraphFile:
    return true;
  case file_types::TY_Image:
  case file_types::TY_Object:
  case file_types::TY_dSYM:
  case file_types::TY_PCH:
  case file_types::TY_SIB:
  case file_types::TY_RawSIB:
  case file_types::TY_CodiraModuleFile:
  case file_types::TY_CodiraModuleDocFile:
  case file_types::TY_CodiraSourceInfoFile:
  case file_types::TY_CodiraCrossImportDir:
  case file_types::TY_CodiraModuleSummaryFile:
  case file_types::TY_TOOLCHAIN_BC:
  case file_types::TY_SerializedDiagnostics:
  case file_types::TY_ClangModuleFile:
  case file_types::TY_CodiraDeps:
  case file_types::TY_ExternalCodiraDeps:
  case file_types::TY_Nothing:
  case file_types::TY_Remapping:
  case file_types::TY_IndexData:
  case file_types::TY_BitstreamOptRecord:
  case file_types::TY_IndexUnitOutputPath:
  case file_types::TY_CodiraFixIt:
  case file_types::TY_ModuleSemanticInfo:
  case file_types::TY_CachedDiagnostics:
    return false;
  case file_types::TY_INVALID:
    toolchain_unreachable("Invalid type ID.");
  }

  // Work around MSVC warning: not all control paths return a value
  toolchain_unreachable("All switch cases are covered");
}

bool file_types::isAfterLLVM(ID Id) {
  switch (Id) {
  case file_types::TY_Assembly:
  case file_types::TY_TOOLCHAIN_IR:
  case file_types::TY_TOOLCHAIN_BC:
  case file_types::TY_Object:
    return true;
  case file_types::TY_Codira:
  case file_types::TY_PCH:
  case file_types::TY_ImportedModules:
  case file_types::TY_TBD:
  case file_types::TY_SIL:
  case file_types::TY_LoweredSIL:
  case file_types::TY_Dependencies:
  case file_types::TY_ASTDump:
  case file_types::TY_RawSIL:
  case file_types::TY_ClangHeader:
  case file_types::TY_AutolinkFile:
  case file_types::TY_Image:
  case file_types::TY_RawTOOLCHAIN_IR:
  case file_types::TY_dSYM:
  case file_types::TY_SIB:
  case file_types::TY_RawSIB:
  case file_types::TY_CodiraModuleFile:
  case file_types::TY_CodiraModuleDocFile:
  case file_types::TY_CodiraSourceInfoFile:
  case file_types::TY_CodiraCrossImportDir:
  case file_types::TY_CodiraModuleSummaryFile:
  case file_types::TY_CodiraOverlayFile:
  case file_types::TY_SerializedDiagnostics:
  case file_types::TY_ClangModuleFile:
  case file_types::TY_CodiraDeps:
  case file_types::TY_ExternalCodiraDeps:
  case file_types::TY_Nothing:
  case file_types::TY_Remapping:
  case file_types::TY_IndexData:
  case file_types::TY_ModuleTrace:
  case file_types::TY_FineModuleTrace:
  case file_types::TY_YAMLOptRecord:
  case file_types::TY_BitstreamOptRecord:
  case file_types::TY_CodiraModuleInterfaceFile:
  case file_types::TY_PrivateCodiraModuleInterfaceFile:
  case file_types::TY_PackageCodiraModuleInterfaceFile:
  case file_types::TY_JSONDependencies:
  case file_types::TY_JSONArguments:
  case file_types::TY_IndexUnitOutputPath:
  case file_types::TY_CodiraABIDescriptor:
  case file_types::TY_CodiraAPIDescriptor:
  case file_types::TY_ConstValues:
  case file_types::TY_CodiraFixIt:
  case file_types::TY_ModuleSemanticInfo:
  case file_types::TY_CachedDiagnostics:
  case file_types::TY_SymbolGraphFile:
    return false;
  case file_types::TY_INVALID:
    toolchain_unreachable("Invalid type ID.");
  }

  // Work around MSVC warning: not all control paths return a value
  toolchain_unreachable("All switch cases are covered");
}

bool file_types::isPartOfCodiraCompilation(ID Id) {
  switch (Id) {
  case file_types::TY_Codira:
  case file_types::TY_SIL:
  case file_types::TY_LoweredSIL:
  case file_types::TY_RawSIL:
  case file_types::TY_SIB:
  case file_types::TY_RawSIB:
    return true;
  case file_types::TY_Assembly:
  case file_types::TY_RawTOOLCHAIN_IR:
  case file_types::TY_TOOLCHAIN_IR:
  case file_types::TY_TOOLCHAIN_BC:
  case file_types::TY_Object:
  case file_types::TY_Dependencies:
  case file_types::TY_ClangHeader:
  case file_types::TY_AutolinkFile:
  case file_types::TY_PCH:
  case file_types::TY_ImportedModules:
  case file_types::TY_TBD:
  case file_types::TY_Image:
  case file_types::TY_dSYM:
  case file_types::TY_CodiraModuleFile:
  case file_types::TY_CodiraModuleDocFile:
  case file_types::TY_CodiraModuleInterfaceFile:
  case file_types::TY_PrivateCodiraModuleInterfaceFile:
  case file_types::TY_PackageCodiraModuleInterfaceFile:
  case file_types::TY_CodiraSourceInfoFile:
  case file_types::TY_CodiraCrossImportDir:
  case file_types::TY_CodiraOverlayFile:
  case file_types::TY_CodiraModuleSummaryFile:
  case file_types::TY_SerializedDiagnostics:
  case file_types::TY_ClangModuleFile:
  case file_types::TY_CodiraDeps:
  case file_types::TY_ExternalCodiraDeps:
  case file_types::TY_Nothing:
  case file_types::TY_ASTDump:
  case file_types::TY_Remapping:
  case file_types::TY_IndexData:
  case file_types::TY_ModuleTrace:
  case file_types::TY_FineModuleTrace:
  case file_types::TY_YAMLOptRecord:
  case file_types::TY_BitstreamOptRecord:
  case file_types::TY_JSONDependencies:
  case file_types::TY_JSONArguments:
  case file_types::TY_IndexUnitOutputPath:
  case file_types::TY_CodiraABIDescriptor:
  case file_types::TY_CodiraAPIDescriptor:
  case file_types::TY_ConstValues:
  case file_types::TY_CodiraFixIt:
  case file_types::TY_ModuleSemanticInfo:
  case file_types::TY_CachedDiagnostics:
  case file_types::TY_SymbolGraphFile:
    return false;
  case file_types::TY_INVALID:
    toolchain_unreachable("Invalid type ID.");
  }

  // Work around MSVC warning: not all control paths return a value
  toolchain_unreachable("All switch cases are covered");
}

bool file_types::isProducedFromDiagnostics(ID Id) {
  switch (Id) {
  case file_types::TY_SerializedDiagnostics:
  case file_types::TY_CodiraFixIt:
  case file_types::TY_CachedDiagnostics:
    return true;
  case file_types::TY_Codira:
  case file_types::TY_SIL:
  case file_types::TY_LoweredSIL:
  case file_types::TY_RawSIL:
  case file_types::TY_SIB:
  case file_types::TY_RawSIB:
  case file_types::TY_Assembly:
  case file_types::TY_RawTOOLCHAIN_IR:
  case file_types::TY_TOOLCHAIN_IR:
  case file_types::TY_TOOLCHAIN_BC:
  case file_types::TY_Object:
  case file_types::TY_Dependencies:
  case file_types::TY_ClangHeader:
  case file_types::TY_AutolinkFile:
  case file_types::TY_PCH:
  case file_types::TY_ImportedModules:
  case file_types::TY_TBD:
  case file_types::TY_Image:
  case file_types::TY_dSYM:
  case file_types::TY_CodiraModuleFile:
  case file_types::TY_CodiraModuleDocFile:
  case file_types::TY_CodiraModuleInterfaceFile:
  case file_types::TY_PrivateCodiraModuleInterfaceFile:
  case file_types::TY_PackageCodiraModuleInterfaceFile:
  case file_types::TY_CodiraSourceInfoFile:
  case file_types::TY_CodiraCrossImportDir:
  case file_types::TY_CodiraOverlayFile:
  case file_types::TY_CodiraModuleSummaryFile:
  case file_types::TY_ClangModuleFile:
  case file_types::TY_CodiraDeps:
  case file_types::TY_ExternalCodiraDeps:
  case file_types::TY_Nothing:
  case file_types::TY_ASTDump:
  case file_types::TY_Remapping:
  case file_types::TY_IndexData:
  case file_types::TY_ModuleTrace:
  case file_types::TY_FineModuleTrace:
  case file_types::TY_YAMLOptRecord:
  case file_types::TY_BitstreamOptRecord:
  case file_types::TY_JSONDependencies:
  case file_types::TY_JSONArguments:
  case file_types::TY_IndexUnitOutputPath:
  case file_types::TY_CodiraABIDescriptor:
  case file_types::TY_CodiraAPIDescriptor:
  case file_types::TY_ConstValues:
  case file_types::TY_ModuleSemanticInfo:
  case file_types::TY_SymbolGraphFile:
    return false;
  case file_types::TY_INVALID:
    toolchain_unreachable("Invalid type ID.");
  }

  // Work around MSVC warning: not all control paths return a value
  toolchain_unreachable("All switch cases are covered");
}
