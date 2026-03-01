//===-- Twine.cpp - Fast Temporary String Concatenation -------------------===//
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

#include "vm/core/ADT/Twine.h"
#include "vm/core/ADT/SmallString.h"
#include "vm/core/Config/toolchain-config.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/FormatVariadic.h"
#include "vm/core/Support/raw_ostream.h"
using namespace vm::core;

std::string Twine::str() const {
  // If we're storing only a std::string, just return it.
  if (LHSKind == StdStringKind && RHSKind == EmptyKind)
    return *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  if (LHSKind == FormatvObjectKind && RHSKind == EmptyKind)
    return LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  return toStringRef(Vec).str();
}

void Twine::toVector(SmallVectorImpl<char> &Out) const {
  raw_svector_ostream OS(Out);
  print(OS);
}

StringRef Twine::toNullTerminatedStringRef(SmallVectorImpl<char> &Out) const {
  if (isUnary()) {
    switch (getLHSKind()) {
    case CStringKind:
      // Already null terminated, yay!
      return StringRef(LHS.cString);
    case StdStringKind: {
      const std::string *str = LHS.stdString;
      return StringRef(str->c_str(), str->size());
    }
    case StringLiteralKind:
      return StringRef(LHS.ptrAndLength.ptr, LHS.ptrAndLength.length);
    default:
      break;
    }
  }
  toVector(Out);
  Out.push_back(0);
  Out.pop_back();
  return StringRef(Out.data(), Out.size());
}

void Twine::printOneChild(raw_ostream &OS, Child Ptr, NodeKind Kind) const {
  switch (Kind) {
  case Twine::NullKind:
    break;
  case Twine::EmptyKind:
    break;
  case Twine::TwineKind:
    Ptr.twine->print(OS);
    break;
  case Twine::CStringKind:
    OS << Ptr.cString;
    break;
  case Twine::StdStringKind:
    OS << *Ptr.stdString;
    break;
  case Twine::PtrAndLengthKind:
  case Twine::StringLiteralKind:
    OS << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length);
    break;
  case Twine::FormatvObjectKind:
    OS << *Ptr.formatvObject;
    break;
  case Twine::CharKind:
    OS << Ptr.character;
    break;
  case Twine::DecUIKind:
    OS << Ptr.decUI;
    break;
  case Twine::DecIKind:
    OS << Ptr.decI;
    break;
  case Twine::DecULKind:
    OS << Ptr.decUL;
    break;
  case Twine::DecLKind:
    OS << Ptr.decL;
    break;
  case Twine::DecULLKind:
    OS << Ptr.decULL;
    break;
  case Twine::DecLLKind:
    OS << Ptr.decLL;
    break;
  case Twine::UHexKind:
    OS.write_hex(Ptr.uHex);
    break;
  }
}

void Twine::printOneChildRepr(raw_ostream &OS, Child Ptr, NodeKind Kind) const {
  switch (Kind) {
  case Twine::NullKind:
    OS << "null";
    break;
  case Twine::EmptyKind:
    OS << "empty";
    break;
  case Twine::TwineKind:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    break;
  case Twine::CStringKind:
    OS << "cstring:\"" << Ptr.cString << "\"";
    break;
  case Twine::StdStringKind:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    break;
  case Twine::PtrAndLengthKind:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    break;
  case Twine::StringLiteralKind:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    break;
  case Twine::FormatvObjectKind:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    break;
  case Twine::CharKind:
    OS << "char:\"" << Ptr.character << "\"";
    break;
  case Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    break;
  case Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    break;
  case Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    break;
  case Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    break;
  case Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    break;
  case Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    break;
  case Twine::UHexKind:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    break;
  }
}

void Twine::print(raw_ostream &OS) const {
  printOneChild(OS, LHS, getLHSKind());
  printOneChild(OS, RHS, getRHSKind());
}

void Twine::printRepr(raw_ostream &OS) const {
  OS << "(Twine ";
  printOneChildRepr(OS, LHS, getLHSKind());
  OS << " ";
  printOneChildRepr(OS, RHS, getRHSKind());
  OS << ")";
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void Twine::dump() const { print(dbgs()); }

LLVM_DUMP_METHOD void Twine::dumpRepr() const { printRepr(dbgs()); }
#endif
