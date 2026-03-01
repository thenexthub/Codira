//===- Remark.cpp ---------------------------------------------------------===//
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
// Implementation of the Remark type and the C API.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Remarks/Remark.h"
#include "vm/core/ADT/APInt.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/ADT/STLExtras.h"
#include <optional>

using namespace vm::core;
using namespace vm::core::remarks;

std::string Remark::getArgsAsMsg() const {
  std::string Str;
  raw_string_ostream OS(Str);
  for (const Argument &Arg : Args)
    OS << Arg.Val;
  return Str;
}

Argument *Remark::getArgByKey(StringRef Key) {
  auto *It = find_if(Args, [&](auto &Arg) { return Arg.Key == Key; });
  if (It == Args.end())
    return nullptr;
  return &*It;
}

void RemarkLocation::print(raw_ostream &OS) const {
  OS << "{ "
     << "File: " << SourceFilePath << ", Line: " << SourceLine
     << " Column:" << SourceColumn << " }\n";
}

void Argument::print(raw_ostream &OS) const {
  OS << Key << ": " << Val << "\n";
}

void Remark::print(raw_ostream &OS) const {
  OS << "Name: ";
  OS << RemarkName << "\n";
  OS << "Type: " << typeToStr(RemarkType) << "\n";
  OS << "FunctionName: " << FunctionName << "\n";
  OS << "PassName: " << PassName << "\n";
  if (Loc)
    OS << "Loc: " << Loc.value();
  if (Hotness)
    OS << "Hotness: " << Hotness;
  if (!Args.empty()) {
    OS << "Args:\n";
    for (auto Arg : Args)
      OS << "\t" << Arg;
  }
}

// Create wrappers for C Binding types (see CBindingWrapping.h).
DEFINE_SIMPLE_CONVERSION_FUNCTIONS(StringRef, LLVMRemarkStringRef)

extern "C" const char *LLVMRemarkStringGetData(LLVMRemarkStringRef String) {
  return unwrap(String)->data();
}

extern "C" uint32_t LLVMRemarkStringGetLen(LLVMRemarkStringRef String) {
  return unwrap(String)->size();
}

extern "C" LLVMRemarkStringRef
LLVMRemarkDebugLocGetSourceFilePath(LLVMRemarkDebugLocRef DL) {
  return wrap(&unwrap(DL)->SourceFilePath);
}

extern "C" uint32_t LLVMRemarkDebugLocGetSourceLine(LLVMRemarkDebugLocRef DL) {
  return unwrap(DL)->SourceLine;
}

extern "C" uint32_t
LLVMRemarkDebugLocGetSourceColumn(LLVMRemarkDebugLocRef DL) {
  return unwrap(DL)->SourceColumn;
}

extern "C" LLVMRemarkStringRef LLVMRemarkArgGetKey(LLVMRemarkArgRef Arg) {
  return wrap(&unwrap(Arg)->Key);
}

extern "C" LLVMRemarkStringRef LLVMRemarkArgGetValue(LLVMRemarkArgRef Arg) {
  return wrap(&unwrap(Arg)->Val);
}

extern "C" LLVMRemarkDebugLocRef
LLVMRemarkArgGetDebugLoc(LLVMRemarkArgRef Arg) {
  if (const std::optional<RemarkLocation> &Loc = unwrap(Arg)->Loc)
    return wrap(&*Loc);
  return nullptr;
}

extern "C" void LLVMRemarkEntryDispose(LLVMRemarkEntryRef Remark) {
  delete unwrap(Remark);
}

extern "C" LLVMRemarkType LLVMRemarkEntryGetType(LLVMRemarkEntryRef Remark) {
  // Assume here that the enums can be converted both ways.
  return static_cast<LLVMRemarkType>(unwrap(Remark)->RemarkType);
}

extern "C" LLVMRemarkStringRef
LLVMRemarkEntryGetPassName(LLVMRemarkEntryRef Remark) {
  return wrap(&unwrap(Remark)->PassName);
}

extern "C" LLVMRemarkStringRef
LLVMRemarkEntryGetRemarkName(LLVMRemarkEntryRef Remark) {
  return wrap(&unwrap(Remark)->RemarkName);
}

extern "C" LLVMRemarkStringRef
LLVMRemarkEntryGetFunctionName(LLVMRemarkEntryRef Remark) {
  return wrap(&unwrap(Remark)->FunctionName);
}

extern "C" LLVMRemarkDebugLocRef
LLVMRemarkEntryGetDebugLoc(LLVMRemarkEntryRef Remark) {
  if (const std::optional<RemarkLocation> &Loc = unwrap(Remark)->Loc)
    return wrap(&*Loc);
  return nullptr;
}

extern "C" uint64_t LLVMRemarkEntryGetHotness(LLVMRemarkEntryRef Remark) {
  if (const std::optional<uint64_t> &Hotness = unwrap(Remark)->Hotness)
    return *Hotness;
  return 0;
}

extern "C" uint32_t LLVMRemarkEntryGetNumArgs(LLVMRemarkEntryRef Remark) {
  return unwrap(Remark)->Args.size();
}

extern "C" LLVMRemarkArgRef
LLVMRemarkEntryGetFirstArg(LLVMRemarkEntryRef Remark) {
  ArrayRef<Argument> Args = unwrap(Remark)->Args;
  // No arguments to iterate on.
  if (Args.empty())
    return nullptr;
  return reinterpret_cast<LLVMRemarkArgRef>(
      const_cast<Argument *>(Args.begin()));
}

extern "C" LLVMRemarkArgRef
LLVMRemarkEntryGetNextArg(LLVMRemarkArgRef ArgIt, LLVMRemarkEntryRef Remark) {
  // No more arguments to iterate on.
  if (ArgIt == nullptr)
    return nullptr;

  auto It = (ArrayRef<Argument>::const_iterator)ArgIt;
  auto Next = std::next(It);
  if (Next == unwrap(Remark)->Args.end())
    return nullptr;

  return reinterpret_cast<LLVMRemarkArgRef>(const_cast<Argument *>(Next));
}
