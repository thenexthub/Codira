//===-- Mangler.cpp - Self-contained c/asm toolchain name mangler --------------===//
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
//
// Unified name mangler for assembly backends.
//
//===----------------------------------------------------------------------===//

#include "vm/core/IR/Mangler.h"
#include "vm/core/ADT/SmallString.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/Demangle/Demangle.h"
#include "vm/core/IR/DataLayout.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/TargetParser/Triple.h"

using namespace vm::core;

namespace {
enum ManglerPrefixTy {
  Default,      ///< Emit default string before each symbol.
  Private,      ///< Emit "private" prefix before each symbol.
  LinkerPrivate ///< Emit "linker private" prefix before each symbol.
};
}

static void getNameWithPrefixImpl(raw_ostream &OS, const Twine &GVName,
                                  ManglerPrefixTy PrefixTy,
                                  const DataLayout &DL, char Prefix) {
  SmallString<256> TmpData;
  StringRef Name = GVName.toStringRef(TmpData);
  assert(!Name.empty() && "getNameWithPrefix requires non-empty name");

  // No need to do anything special if the global has the special "do not
  // mangle" flag in the name.
  if (Name[0] == '\1') {
    OS << Name.substr(1);
    return;
  }

  if (DL.doNotMangleLeadingQuestionMark() && Name[0] == '?')
    Prefix = '\0';

  if (PrefixTy == Private)
    OS << DL.getPrivateGlobalPrefix();
  else if (PrefixTy == LinkerPrivate)
    OS << DL.getLinkerPrivateGlobalPrefix();

  if (Prefix != '\0')
    OS << Prefix;

  // If this is a simple string that doesn't need escaping, just append it.
  OS << Name;
}

static void getNameWithPrefixImpl(raw_ostream &OS, const Twine &GVName,
                                  const DataLayout &DL,
                                  ManglerPrefixTy PrefixTy) {
  char Prefix = DL.getGlobalPrefix();
  return getNameWithPrefixImpl(OS, GVName, PrefixTy, DL, Prefix);
}

void Mangler::getNameWithPrefix(raw_ostream &OS, const Twine &GVName,
                                const DataLayout &DL) {
  return getNameWithPrefixImpl(OS, GVName, DL, Default);
}

void Mangler::getNameWithPrefix(SmallVectorImpl<char> &OutName,
                                const Twine &GVName, const DataLayout &DL) {
  raw_svector_ostream OS(OutName);
  char Prefix = DL.getGlobalPrefix();
  return getNameWithPrefixImpl(OS, GVName, Default, DL, Prefix);
}

static bool hasByteCountSuffix(CallingConv::ID CC) {
  switch (CC) {
  case CallingConv::X86_FastCall:
  case CallingConv::X86_StdCall:
  case CallingConv::X86_VectorCall:
    return true;
  default:
    return false;
  }
}

/// Microsoft fastcall and stdcall functions require a suffix on their name
/// indicating the number of words of arguments they take.
static void addByteCountSuffix(raw_ostream &OS, const Function *F,
                               const DataLayout &DL) {
  // Calculate arguments size total.
  unsigned ArgWords = 0;

  const unsigned PtrSize = DL.getPointerSize();

  for (const Argument &A : F->args()) {
    // For the purposes of the byte count suffix, structs returned by pointer
    // do not count as function arguments.
    if (A.hasStructRetAttr())
      continue;

    // 'Dereference' type in case of byval or inalloca parameter attribute.
    uint64_t AllocSize = A.hasPassPointeeByValueCopyAttr() ?
      A.getPassPointeeByValueCopySize(DL) :
      DL.getTypeAllocSize(A.getType());

    // Size should be aligned to pointer size.
    ArgWords += alignTo(AllocSize, PtrSize);
  }

  OS << '@' << ArgWords;
}

void Mangler::getNameWithPrefix(raw_ostream &OS, const GlobalValue *GV,
                                bool CannotUsePrivateLabel) const {
  ManglerPrefixTy PrefixTy = Default;
  assert(GV != nullptr && "Invalid Global Value");
  if (GV->hasPrivateLinkage()) {
    if (CannotUsePrivateLabel)
      PrefixTy = LinkerPrivate;
    else
      PrefixTy = Private;
  }

  const DataLayout &DL = GV->getDataLayout();
  if (!GV->hasName()) {
    // Get the ID for the global, assigning a new one if we haven't got one
    // already.
    unsigned &ID = AnonGlobalIDs[GV];
    if (ID == 0)
      ID = AnonGlobalIDs.size();

    // Must mangle the global into a unique ID.
    getNameWithPrefixImpl(OS, "__unnamed_" + Twine(ID), DL, PrefixTy);
    return;
  }

  StringRef Name = GV->getName();
  char Prefix = DL.getGlobalPrefix();

  // Mangle functions with Microsoft calling conventions specially.  Only do
  // this mangling for x86_64 vectorcall and 32-bit x86.
  const Function *MSFunc = dyn_cast_or_null<Function>(GV->getAliaseeObject());

  // Don't add byte count suffixes when '\01' or '?' are in the first
  // character.
  if (Name.starts_with("\01") ||
      (DL.doNotMangleLeadingQuestionMark() && Name.starts_with("?")))
    MSFunc = nullptr;

  CallingConv::ID CC =
      MSFunc ? MSFunc->getCallingConv() : (unsigned)CallingConv::C;
  if (!DL.hasMicrosoftFastStdCallMangling() &&
      CC != CallingConv::X86_VectorCall)
    MSFunc = nullptr;
  if (MSFunc) {
    if (CC == CallingConv::X86_FastCall)
      Prefix = '@'; // fastcall functions have an @ prefix instead of _.
    else if (CC == CallingConv::X86_VectorCall)
      Prefix = '\0'; // vectorcall functions have no prefix.
  }

  getNameWithPrefixImpl(OS, Name, PrefixTy, DL, Prefix);

  if (!MSFunc)
    return;

  // If we are supposed to add a microsoft-style suffix for stdcall, fastcall,
  // or vectorcall, add it.  These functions have a suffix of @N where N is the
  // cumulative byte size of all of the parameters to the function in decimal.
  if (CC == CallingConv::X86_VectorCall)
    OS << '@'; // vectorcall functions use a double @ suffix.
  FunctionType *FT = MSFunc->getFunctionType();
  if (hasByteCountSuffix(CC) &&
      // "Pure" variadic functions do not receive @0 suffix.
      (!FT->isVarArg() || FT->getNumParams() == 0 ||
       (FT->getNumParams() == 1 && MSFunc->hasStructRetAttr())))
    addByteCountSuffix(OS, MSFunc, DL);
}

void Mangler::getNameWithPrefix(SmallVectorImpl<char> &OutName,
                                const GlobalValue *GV,
                                bool CannotUsePrivateLabel) const {
  raw_svector_ostream OS(OutName);
  getNameWithPrefix(OS, GV, CannotUsePrivateLabel);
}

// Check if the name needs quotes to be safe for the linker to interpret.
static bool canBeUnquotedInDirective(char C) {
  return isAlnum(C) || C == '_' || C == '@' || C == '#';
}

static bool canBeUnquotedInDirective(StringRef Name) {
  if (Name.empty())
    return false;

  // If any of the characters in the string is an unacceptable character, force
  // quotes.
  for (char C : Name) {
    if (!canBeUnquotedInDirective(C))
      return false;
  }

  return true;
}

void toolchain::emitLinkerFlagsForGlobalCOFF(raw_ostream &OS, const GlobalValue *GV,
                                        const Triple &TT, Mangler &Mangler) {
  if (GV->hasDLLExportStorageClass() && !GV->isDeclaration()) {

    if (TT.isWindowsMSVCEnvironment() || TT.isUEFI())
      OS << " /EXPORT:";
    else
      OS << " -export:";

    bool NeedQuotes = GV->hasName() && !canBeUnquotedInDirective(GV->getName());
    if (NeedQuotes)
      OS << "\"";
    if (TT.isWindowsGNUEnvironment() || TT.isWindowsCygwinEnvironment()) {
      std::string Flag;
      raw_string_ostream FlagOS(Flag);
      Mangler.getNameWithPrefix(FlagOS, GV, false);
      if (Flag[0] == GV->getDataLayout().getGlobalPrefix())
        OS << Flag.substr(1);
      else
        OS << Flag;
    } else {
      Mangler.getNameWithPrefix(OS, GV, false);
    }
    if (TT.isWindowsArm64EC()) {
      // Use EXPORTAS for mangled ARM64EC symbols.
      // FIXME: During LTO, we're invoked prior to the EC lowering pass,
      // so symbols are not yet mangled. Emitting the unmangled name
      // typically functions correctly; the linker can resolve the export
      // with the demangled alias.
      if (std::optional<std::string> demangledName =
              getArm64ECDemangledFunctionName(GV->getName()))
        OS << ",EXPORTAS," << *demangledName;
    }
    if (NeedQuotes)
      OS << "\"";

    if (!GV->getValueType()->isFunctionTy()) {
      if (TT.isWindowsMSVCEnvironment() || TT.isUEFI())
        OS << ",DATA";
      else
        OS << ",data";
    }
  }
  if (GV->hasHiddenVisibility() && !GV->isDeclaration() && TT.isOSCygMing()) {

    OS << " -exclude-symbols:";

    bool NeedQuotes = GV->hasName() && !canBeUnquotedInDirective(GV->getName());
    if (NeedQuotes)
      OS << "\"";

    std::string Flag;
    raw_string_ostream FlagOS(Flag);
    Mangler.getNameWithPrefix(FlagOS, GV, false);
    if (Flag[0] == GV->getDataLayout().getGlobalPrefix())
      OS << Flag.substr(1);
    else
      OS << Flag;

    if (NeedQuotes)
      OS << "\"";
  }
}

void toolchain::emitLinkerFlagsForUsedCOFF(raw_ostream &OS, const GlobalValue *GV,
                                      const Triple &T, Mangler &M) {
  if (!T.isWindowsMSVCEnvironment())
    return;

  OS << " /INCLUDE:";
  bool NeedQuotes = GV->hasName() && !canBeUnquotedInDirective(GV->getName());
  if (NeedQuotes)
    OS << "\"";
  M.getNameWithPrefix(OS, GV, false);
  if (NeedQuotes)
    OS << "\"";
}

std::optional<std::string> toolchain::getArm64ECMangledFunctionName(StringRef Name) {
  assert(!Name.empty() &&
         "getArm64ECMangledFunctionName requires non-empty name");

  if (Name[0] != '?') {
    // For non-C++ symbols, prefix the name with "#" unless it's already
    // mangled.
    if (Name[0] == '#')
      return std::nullopt;
    return std::optional<std::string>(("#" + Name).str());
  }

  // If the name contains $$h, then it is already mangled.
  if (Name.contains("$$h"))
    return std::nullopt;

  // Handle MD5 mangled names, which use a slightly different rule from
  // other C++ manglings.
  //
  // A non-Arm64EC function:
  //
  // ??@aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa@
  //
  // An Arm64EC function:
  //
  // ??@aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa@$$h@
  if (Name.starts_with("??@") && Name.ends_with("@"))
    return (Name + "$$h@").str();

  // Ask the demangler where we should insert "$$h".
  auto InsertIdx = getArm64ECInsertionPointInMangledName(Name);
  if (!InsertIdx)
    return std::nullopt;

  return std::optional<std::string>(
      (Name.substr(0, *InsertIdx) + "$$h" + Name.substr(*InsertIdx)).str());
}

std::optional<std::string>
toolchain::getArm64ECDemangledFunctionName(StringRef Name) {
  // For non-C++ names, drop the "#" prefix.
  if (Name[0] == '#')
    return std::optional<std::string>(Name.substr(1));
  if (Name[0] != '?')
    return std::nullopt;

  // MD5 mangled name; see comment in getArm64ECMangledFunctionName.
  if (Name.starts_with("??@") && Name.ends_with("@$$h@"))
    return Name.drop_back(4).str();

  // Drop the ARM64EC "$$h" tag.
  std::pair<StringRef, StringRef> Pair = Name.split("$$h");
  if (Pair.second.empty())
    return std::nullopt;
  return std::optional<std::string>((Pair.first + Pair.second).str());
}
