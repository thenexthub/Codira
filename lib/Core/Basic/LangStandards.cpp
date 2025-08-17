//===--- LangStandards.cpp - Language Standard Definitions ----------------===//
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

#include "language/Core/Basic/LangStandard.h"
#include "language/Core/Config/config.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/TargetParser/Triple.h"
using namespace language::Core;

StringRef language::Core::languageToString(Language L) {
  switch (L) {
  case Language::Unknown:
    return "Unknown";
  case Language::Asm:
    return "Asm";
  case Language::LLVM_IR:
    return "LLVM IR";
  case Language::CIR:
    return "ClangIR";
  case Language::C:
    return "C";
  case Language::CXX:
    return "C++";
  case Language::ObjC:
    return "Objective-C";
  case Language::ObjCXX:
    return "Objective-C++";
  case Language::OpenCL:
    return "OpenCL";
  case Language::OpenCLCXX:
    return "OpenCLC++";
  case Language::CUDA:
    return "CUDA";
  case Language::HIP:
    return "HIP";
  case Language::HLSL:
    return "HLSL";
  }

  toolchain_unreachable("unhandled language kind");
}

#define LANGSTANDARD(id, name, lang, desc, features)                           \
  static const LangStandard Lang_##id = {name, desc, features, Language::lang};
#include "language/Core/Basic/LangStandards.def"

const LangStandard &LangStandard::getLangStandardForKind(Kind K) {
  switch (K) {
  case lang_unspecified:
    toolchain::report_fatal_error("getLangStandardForKind() on unspecified kind");
#define LANGSTANDARD(id, name, lang, desc, features) \
    case lang_##id: return Lang_##id;
#include "language/Core/Basic/LangStandards.def"
  }
  toolchain_unreachable("Invalid language kind!");
}

LangStandard::Kind LangStandard::getLangKind(StringRef Name) {
  return toolchain::StringSwitch<Kind>(Name)
#define LANGSTANDARD(id, name, lang, desc, features) .Case(name, lang_##id)
#define LANGSTANDARD_ALIAS(id, alias) .Case(alias, lang_##id)
#include "language/Core/Basic/LangStandards.def"
      .Default(lang_unspecified);
}

LangStandard::Kind LangStandard::getHLSLLangKind(StringRef Name) {
  return toolchain::StringSwitch<LangStandard::Kind>(Name)
      .Case("2016", LangStandard::lang_hlsl2016)
      .Case("2017", LangStandard::lang_hlsl2017)
      .Case("2018", LangStandard::lang_hlsl2018)
      .Case("2021", LangStandard::lang_hlsl2021)
      .Case("202x", LangStandard::lang_hlsl202x)
      .Case("202y", LangStandard::lang_hlsl202y)
      .Default(LangStandard::lang_unspecified);
}

const LangStandard *LangStandard::getLangStandardForName(StringRef Name) {
  Kind K = getLangKind(Name);
  if (K == lang_unspecified)
    return nullptr;

  return &getLangStandardForKind(K);
}

LangStandard::Kind language::Core::getDefaultLanguageStandard(language::Core::Language Lang,
                                                     const toolchain::Triple &T) {
  switch (Lang) {
  case Language::Unknown:
  case Language::LLVM_IR:
  case Language::CIR:
    toolchain_unreachable("Invalid input kind!");
  case Language::OpenCL:
    return LangStandard::lang_opencl12;
  case Language::OpenCLCXX:
    return LangStandard::lang_openclcpp10;
  case Language::Asm:
  case Language::C:
    // The PS4 uses C99 as the default C standard.
    if (T.isPS4())
      return LangStandard::lang_gnu99;
    return LangStandard::lang_gnu17;
  case Language::ObjC:
    return LangStandard::lang_gnu11;
  case Language::CXX:
  case Language::ObjCXX:
  case Language::CUDA:
  case Language::HIP:
    return LangStandard::lang_gnucxx17;
  case Language::HLSL:
    return LangStandard::lang_hlsl202x;
  }
  toolchain_unreachable("unhandled Language kind!");
}
