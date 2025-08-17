//===- LangOptions.cpp - C Language Family Language Options ---------------===//
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
//  This file defines the LangOptions class.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Basic/LangOptions.h"
#include "toolchain/Support/Path.h"

using namespace language::Core;

LangOptions::LangOptions() : LangStd(LangStandard::lang_unspecified) {
#define LANGOPT(Name, Bits, Default, Compatibility, Description) Name = Default;
#define ENUM_LANGOPT(Name, Type, Bits, Default, Compatibility, Description)    \
  set##Name(Default);
#include "language/Core/Basic/LangOptions.def"
}

void LangOptions::resetNonModularOptions() {
#define LANGOPT(Name, Bits, Default, Compatibility, Description)               \
  if constexpr (CompatibilityKind::Compatibility == CompatibilityKind::Benign) \
    Name = Default;
#define ENUM_LANGOPT(Name, Type, Bits, Default, Compatibility, Description)    \
  if constexpr (CompatibilityKind::Compatibility == CompatibilityKind::Benign) \
    Name = static_cast<unsigned>(Default);
#include "language/Core/Basic/LangOptions.def"

  // Reset "benign" options with implied values (Options.td ImpliedBy relations)
  // rather than their defaults. This avoids unexpected combinations and
  // invocations that cannot be round-tripped to arguments.
  // FIXME: we should derive this automatically from ImpliedBy in tablegen.
  AllowFPReassoc = UnsafeFPMath;
  NoHonorInfs = FastMath;
  NoHonorNaNs = FastMath;

  // These options do not affect AST generation.
  NoSanitizeFiles.clear();
  XRayAlwaysInstrumentFiles.clear();
  XRayNeverInstrumentFiles.clear();

  CurrentModule.clear();
  IsHeaderFile = false;
}

bool LangOptions::isNoBuiltinFunc(StringRef FuncName) const {
  for (unsigned i = 0, e = NoBuiltinFuncs.size(); i != e; ++i)
    if (FuncName == NoBuiltinFuncs[i])
      return true;
  return false;
}

VersionTuple LangOptions::getOpenCLVersionTuple() const {
  const int Ver = OpenCLCPlusPlus ? OpenCLCPlusPlusVersion : OpenCLVersion;
  if (OpenCLCPlusPlus && Ver != 100)
    return VersionTuple(Ver / 100);
  return VersionTuple(Ver / 100, (Ver % 100) / 10);
}

unsigned LangOptions::getOpenCLCompatibleVersion() const {
  if (!OpenCLCPlusPlus)
    return OpenCLVersion;
  if (OpenCLCPlusPlusVersion == 100)
    return 200;
  if (OpenCLCPlusPlusVersion == 202100)
    return 300;
  toolchain_unreachable("Unknown OpenCL version");
}

void LangOptions::remapPathPrefix(SmallVectorImpl<char> &Path) const {
  for (const auto &Entry : MacroPrefixMap)
    if (toolchain::sys::path::replace_path_prefix(Path, Entry.first, Entry.second))
      break;
}

std::string LangOptions::getOpenCLVersionString() const {
  std::string Result;
  {
    toolchain::raw_string_ostream Out(Result);
    Out << (OpenCLCPlusPlus ? "C++ for OpenCL" : "OpenCL C") << " version "
        << getOpenCLVersionTuple().getAsString();
  }
  return Result;
}

void LangOptions::setLangDefaults(LangOptions &Opts, Language Lang,
                                  const toolchain::Triple &T,
                                  std::vector<std::string> &Includes,
                                  LangStandard::Kind LangStd) {
  // Set some properties which depend solely on the input kind; it would be nice
  // to move these to the language standard, and have the driver resolve the
  // input kind + language standard.
  //
  // FIXME: Perhaps a better model would be for a single source file to have
  // multiple language standards (C / C++ std, ObjC std, OpenCL std, OpenMP std)
  // simultaneously active?
  if (Lang == Language::Asm) {
    Opts.AsmPreprocessor = 1;
  } else if (Lang == Language::ObjC || Lang == Language::ObjCXX) {
    Opts.ObjC = 1;
  }

  if (LangStd == LangStandard::lang_unspecified)
    LangStd = getDefaultLanguageStandard(Lang, T);
  const LangStandard &Std = LangStandard::getLangStandardForKind(LangStd);
  Opts.LangStd = LangStd;
  Opts.LineComment = Std.hasLineComments();
  Opts.C99 = Std.isC99();
  Opts.C11 = Std.isC11();
  Opts.C17 = Std.isC17();
  Opts.C23 = Std.isC23();
  Opts.C2y = Std.isC2y();
  Opts.CPlusPlus = Std.isCPlusPlus();
  Opts.CPlusPlus11 = Std.isCPlusPlus11();
  Opts.CPlusPlus14 = Std.isCPlusPlus14();
  Opts.CPlusPlus17 = Std.isCPlusPlus17();
  Opts.CPlusPlus20 = Std.isCPlusPlus20();
  Opts.CPlusPlus23 = Std.isCPlusPlus23();
  Opts.CPlusPlus26 = Std.isCPlusPlus26();
  Opts.GNUMode = Std.isGNUMode();
  Opts.GNUCVersion = 0;
  Opts.HexFloats = Std.hasHexFloats();
  Opts.WChar = Std.isCPlusPlus();
  Opts.Digraphs = Std.hasDigraphs();
  Opts.RawStringLiterals = Std.hasRawStringLiterals();

  Opts.HLSL = Lang == Language::HLSL;
  if (Opts.HLSL && Opts.IncludeDefaultHeader)
    Includes.push_back("hlsl.h");

  // Set OpenCL Version.
  Opts.OpenCL = Std.isOpenCL();
  if (LangStd == LangStandard::lang_opencl10)
    Opts.OpenCLVersion = 100;
  else if (LangStd == LangStandard::lang_opencl11)
    Opts.OpenCLVersion = 110;
  else if (LangStd == LangStandard::lang_opencl12)
    Opts.OpenCLVersion = 120;
  else if (LangStd == LangStandard::lang_opencl20)
    Opts.OpenCLVersion = 200;
  else if (LangStd == LangStandard::lang_opencl30)
    Opts.OpenCLVersion = 300;
  else if (LangStd == LangStandard::lang_openclcpp10)
    Opts.OpenCLCPlusPlusVersion = 100;
  else if (LangStd == LangStandard::lang_openclcpp2021)
    Opts.OpenCLCPlusPlusVersion = 202100;
  else if (LangStd == LangStandard::lang_hlsl2015)
    Opts.HLSLVersion = (unsigned)LangOptions::HLSL_2015;
  else if (LangStd == LangStandard::lang_hlsl2016)
    Opts.HLSLVersion = (unsigned)LangOptions::HLSL_2016;
  else if (LangStd == LangStandard::lang_hlsl2017)
    Opts.HLSLVersion = (unsigned)LangOptions::HLSL_2017;
  else if (LangStd == LangStandard::lang_hlsl2018)
    Opts.HLSLVersion = (unsigned)LangOptions::HLSL_2018;
  else if (LangStd == LangStandard::lang_hlsl2021)
    Opts.HLSLVersion = (unsigned)LangOptions::HLSL_2021;
  else if (LangStd == LangStandard::lang_hlsl202x)
    Opts.HLSLVersion = (unsigned)LangOptions::HLSL_202x;
  else if (LangStd == LangStandard::lang_hlsl202y)
    Opts.HLSLVersion = (unsigned)LangOptions::HLSL_202y;

  // OpenCL has some additional defaults.
  if (Opts.OpenCL) {
    Opts.AltiVec = 0;
    Opts.ZVector = 0;
    Opts.setDefaultFPContractMode(LangOptions::FPM_On);
    Opts.OpenCLCPlusPlus = Opts.CPlusPlus;
    Opts.OpenCLPipes = Opts.getOpenCLCompatibleVersion() == 200;
    Opts.OpenCLGenericAddressSpace = Opts.getOpenCLCompatibleVersion() == 200;

    // Include default header file for OpenCL.
    if (Opts.IncludeDefaultHeader) {
      if (Opts.DeclareOpenCLBuiltins) {
        // Only include base header file for builtin types and constants.
        Includes.push_back("opencl-c-base.h");
      } else {
        Includes.push_back("opencl-c.h");
      }
    }
  }

  Opts.HIP = Lang == Language::HIP;
  Opts.CUDA = Lang == Language::CUDA || Opts.HIP;
  if (Opts.HIP) {
    // HIP toolchain does not support 'Fast' FPOpFusion in backends since it
    // fuses multiplication/addition instructions without contract flag from
    // device library functions in LLVM bitcode, which causes accuracy loss in
    // certain math functions, e.g. tan(-1e20) becomes -0.933 instead of 0.8446.
    // For device library functions in bitcode to work, 'Strict' or 'Standard'
    // FPOpFusion options in backends is needed. Therefore 'fast-honor-pragmas'
    // FP contract option is used to allow fuse across statements in frontend
    // whereas respecting contract flag in backend.
    Opts.setDefaultFPContractMode(LangOptions::FPM_FastHonorPragmas);
  } else if (Opts.CUDA) {
    if (T.isSPIRV()) {
      // Emit OpenCL version metadata in LLVM IR when targeting SPIR-V.
      Opts.OpenCLVersion = 200;
    }
    // Allow fuse across statements disregarding pragmas.
    Opts.setDefaultFPContractMode(LangOptions::FPM_Fast);
  }

  // OpenCL, C++ and C23 have bool, true, false keywords.
  Opts.Bool = Opts.OpenCL || Opts.CPlusPlus || Opts.C23;

  // OpenCL and HLSL have half keyword
  Opts.Half = Opts.OpenCL || Opts.HLSL;

  Opts.PreserveVec3Type = Opts.HLSL;
}

FPOptions FPOptions::defaultWithoutTrailingStorage(const LangOptions &LO) {
  FPOptions result(LO);
  return result;
}

FPOptionsOverride FPOptions::getChangesSlow(const FPOptions &Base) const {
  FPOptions::storage_type OverrideMask = 0;
#define FP_OPTION(NAME, TYPE, WIDTH, PREVIOUS)                                 \
  if (get##NAME() != Base.get##NAME())                                         \
    OverrideMask |= NAME##Mask;
#include "language/Core/Basic/FPOptions.def"
  return FPOptionsOverride(*this, OverrideMask);
}

LLVM_DUMP_METHOD void FPOptions::dump() {
#define FP_OPTION(NAME, TYPE, WIDTH, PREVIOUS)                                 \
  toolchain::errs() << "\n " #NAME " " << get##NAME();
#include "language/Core/Basic/FPOptions.def"
  toolchain::errs() << "\n";
}

LLVM_DUMP_METHOD void FPOptionsOverride::dump() {
#define FP_OPTION(NAME, TYPE, WIDTH, PREVIOUS)                                 \
  if (has##NAME##Override())                                                   \
    toolchain::errs() << "\n " #NAME " Override is " << get##NAME##Override();
#include "language/Core/Basic/FPOptions.def"
  toolchain::errs() << "\n";
}
