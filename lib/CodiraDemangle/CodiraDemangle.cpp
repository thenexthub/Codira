//===--- CodiraDemangle.cpp - Public demangling interface ------------------===//
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
// Functions in the liblanguageDemangle library, which provides external
// access to Codira's demangler.
//
//===----------------------------------------------------------------------===//

#include "language/Basic/Assertions.h"
#include "language/Demangling/Demangle.h"
#include "language/CodiraDemangle/CodiraDemangle.h"

static size_t language_demangle_getDemangledName_Options(const char *MangledName,
    char *OutputBuffer, size_t Length,
    language::Demangle::DemangleOptions DemangleOptions) {
  assert(MangledName != nullptr && "null input");
  assert(OutputBuffer != nullptr || Length == 0);

  if (!language::Demangle::isCodiraSymbol(MangledName))
    return 0; // Not a mangled name

  std::string Result = language::Demangle::demangleSymbolAsString(
      toolchain::StringRef(MangledName), DemangleOptions);

  if (Result == MangledName)
    return 0; // Not a mangled name

  // Copy the result to an output buffer and ensure '\0' termination.
  if (OutputBuffer && Length > 0) {
    auto Dest = strncpy(OutputBuffer, Result.c_str(), Length);
    Dest[Length - 1] = '\0';
  }
  return Result.length();
}

size_t language_demangle_getDemangledName(const char *MangledName,
                                       char *OutputBuffer,
                                       size_t Length) {
  language::Demangle::DemangleOptions DemangleOptions;
  DemangleOptions.SynthesizeSugarOnTypes = true;
  return language_demangle_getDemangledName_Options(MangledName, OutputBuffer,
                                                 Length, DemangleOptions);
}

size_t language_demangle_getSimplifiedDemangledName(const char *MangledName,
                                                 char *OutputBuffer,
                                                 size_t Length) {
  auto Opts = language::Demangle::DemangleOptions::SimplifiedUIDemangleOptions();
  return language_demangle_getDemangledName_Options(MangledName, OutputBuffer,
                                                 Length, Opts);
}

size_t language_demangle_getModuleName(const char *MangledName,
                                    char *OutputBuffer,
                                    size_t Length) {

  language::Demangle::Context DCtx;
  std::string Result = DCtx.getModuleName(toolchain::StringRef(MangledName));

  // Copy the result to an output buffer and ensure '\0' termination.
  if (OutputBuffer && Length > 0) {
    auto Dest = strncpy(OutputBuffer, Result.c_str(), Length);
    Dest[Length - 1] = '\0';
  }
  return Result.length();
}


int language_demangle_hasCodiraCallingConvention(const char *MangledName) {
  language::Demangle::Context DCtx;
  if (DCtx.hasCodiraCallingConvention(toolchain::StringRef(MangledName)))
    return 1;
  return 0;
}

size_t fnd_get_demangled_name(const char *MangledName, char *OutputBuffer,
                              size_t Length) {
  return language_demangle_getDemangledName(MangledName, OutputBuffer, Length);
}

