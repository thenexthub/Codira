//===--- SwiftDemangle.cpp - Public demangling interface ------------------===//
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
// Functions in the libswiftDemangle library, which provides external
// access to Swift's demangler.
//
//===----------------------------------------------------------------------===//

#include "language/Basic/Assertions.h"
#include "language/Demangling/Demangle.h"
#include "language/SwiftDemangle/SwiftDemangle.h"

static size_t swift_demangle_getDemangledName_Options(const char *MangledName,
    char *OutputBuffer, size_t Length,
    swift::Demangle::DemangleOptions DemangleOptions) {
  assert(MangledName != nullptr && "null input");
  assert(OutputBuffer != nullptr || Length == 0);

  if (!swift::Demangle::isSwiftSymbol(MangledName))
    return 0; // Not a mangled name

  std::string Result = swift::Demangle::demangleSymbolAsString(
      llvm::StringRef(MangledName), DemangleOptions);

  if (Result == MangledName)
    return 0; // Not a mangled name

  // Copy the result to an output buffer and ensure '\0' termination.
  if (OutputBuffer && Length > 0) {
    auto Dest = strncpy(OutputBuffer, Result.c_str(), Length);
    Dest[Length - 1] = '\0';
  }
  return Result.length();
}

size_t swift_demangle_getDemangledName(const char *MangledName,
                                       char *OutputBuffer,
                                       size_t Length) {
  swift::Demangle::DemangleOptions DemangleOptions;
  DemangleOptions.SynthesizeSugarOnTypes = true;
  return swift_demangle_getDemangledName_Options(MangledName, OutputBuffer,
                                                 Length, DemangleOptions);
}

size_t swift_demangle_getSimplifiedDemangledName(const char *MangledName,
                                                 char *OutputBuffer,
                                                 size_t Length) {
  auto Opts = swift::Demangle::DemangleOptions::SimplifiedUIDemangleOptions();
  return swift_demangle_getDemangledName_Options(MangledName, OutputBuffer,
                                                 Length, Opts);
}

size_t swift_demangle_getModuleName(const char *MangledName,
                                    char *OutputBuffer,
                                    size_t Length) {

  swift::Demangle::Context DCtx;
  std::string Result = DCtx.getModuleName(llvm::StringRef(MangledName));

  // Copy the result to an output buffer and ensure '\0' termination.
  if (OutputBuffer && Length > 0) {
    auto Dest = strncpy(OutputBuffer, Result.c_str(), Length);
    Dest[Length - 1] = '\0';
  }
  return Result.length();
}


int swift_demangle_hasSwiftCallingConvention(const char *MangledName) {
  swift::Demangle::Context DCtx;
  if (DCtx.hasSwiftCallingConvention(llvm::StringRef(MangledName)))
    return 1;
  return 0;
}

size_t fnd_get_demangled_name(const char *MangledName, char *OutputBuffer,
                              size_t Length) {
  return swift_demangle_getDemangledName(MangledName, OutputBuffer, Length);
}

