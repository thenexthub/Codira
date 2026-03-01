//===-- Demangle.cpp - Common demangling functions ------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
///
/// \file This file contains definitions of common demangling functions.
///
//===----------------------------------------------------------------------===//

#include "vm/core/Demangle/Demangle.h"
#include "vm/core/Demangle/StringViewExtras.h"
#include <cstdlib>
#include <string_view>

using toolchain::itanium_demangle::starts_with;

std::string toolchain::demangle(std::string_view MangledName) {
  std::string Result;

  if (nonMicrosoftDemangle(MangledName, Result))
    return Result;

  if (starts_with(MangledName, '_') &&
      nonMicrosoftDemangle(MangledName.substr(1), Result,
                           /*CanHaveLeadingDot=*/false))
    return Result;

  if (char *Demangled = microsoftDemangle(MangledName, nullptr, nullptr)) {
    Result = Demangled;
    std::free(Demangled);
  } else {
    Result = MangledName;
  }
  return Result;
}

static bool isItaniumEncoding(std::string_view S) {
  // Itanium demangler supports prefixes with 1-4 underscores.
  const size_t Pos = S.find_first_not_of('_');
  return Pos > 0 && Pos <= 4 && S[Pos] == 'Z';
}

static bool isRustEncoding(std::string_view S) { return starts_with(S, "_R"); }

static bool isDLangEncoding(std::string_view S) { return starts_with(S, "_D"); }

bool toolchain::nonMicrosoftDemangle(std::string_view MangledName,
                                std::string &Result, bool CanHaveLeadingDot,
                                bool ParseParams) {
  char *Demangled = nullptr;

  // Do not consider the dot prefix as part of the demangled symbol name.
  if (CanHaveLeadingDot && MangledName.size() > 0 && MangledName[0] == '.') {
    MangledName.remove_prefix(1);
    Result = ".";
  }

  if (isItaniumEncoding(MangledName))
    Demangled = itaniumDemangle(MangledName, ParseParams);
  else if (isRustEncoding(MangledName))
    Demangled = rustDemangle(MangledName);
  else if (isDLangEncoding(MangledName))
    Demangled = dlangDemangle(MangledName);

  if (!Demangled)
    return false;

  Result += Demangled;
  std::free(Demangled);
  return true;
}
