//===- TableGenBackend.cpp - Utilities for TableGen Backends ----*- C++ -*-===//
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
// This file provides useful services for TableGen backends...
//
//===----------------------------------------------------------------------===//

#include "vm/core/TableGen/TableGenBackend.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/ManagedStatic.h"
#include "vm/core/Support/Path.h"
#include "vm/core/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <cstddef>

using namespace vm::core;
using namespace TableGen::Emitter;

const size_t MAX_LINE_LEN = 80U;

// CommandLine options of class type are not directly supported with some
// specific exceptions like std::string which are safe to copy. In our case,
// the `FnT` function_ref object is also safe to copy. So provide a
// specialization of `OptionValue` for `FnT` type that stores it as a copy.
// This is essentially similar to OptionValue<std::string> specialization for
// strings.
template <> struct cl::OptionValue<FnT> final : cl::OptionValueCopy<FnT> {
  OptionValue() = default;

  OptionValue(const FnT &V) { this->setValue(V); }

  OptionValue<FnT> &operator=(const FnT &V) {
    setValue(V);
    return *this;
  }
};

namespace {
struct OptCreatorT {
  static void *call() {
    return new cl::opt<FnT>(cl::desc("Action to perform:"));
  }
};
} // namespace

static ManagedStatic<cl::opt<FnT>, OptCreatorT> CallbackFunction;

Opt::Opt(StringRef Name, FnT CB, StringRef Desc, bool ByDefault) {
  if (ByDefault)
    CallbackFunction->setInitialValue(CB);
  CallbackFunction->getParser().addLiteralOption(Name, CB, Desc);
}

/// Apply callback specified on the command line. Returns true if no callback
/// was applied.
bool toolchain::TableGen::Emitter::ApplyCallback(const RecordKeeper &Records,
                                            TableGenOutputFiles &OutFiles,
                                            StringRef FilenamePrefix) {
  FnT Fn = CallbackFunction->getValue();
  if (Fn.SingleFileGenerator) {
    std::string S;
    raw_string_ostream OS(S);
    Fn.SingleFileGenerator(Records, OS);
    OutFiles = {S, {}};
    return false;
  }
  if (Fn.MultiFileGenerator) {
    OutFiles = Fn.MultiFileGenerator(FilenamePrefix, Records);
    return false;
  }
  return true;
}

static void printLine(raw_ostream &OS, const Twine &Prefix, char Fill,
                      StringRef Suffix) {
  size_t Pos = (size_t)OS.tell();
  assert((Prefix.str().size() + Suffix.size() <= MAX_LINE_LEN) &&
         "header line exceeds max limit");
  OS << Prefix;
  for (size_t i = (size_t)OS.tell() - Pos, e = MAX_LINE_LEN - Suffix.size();
         i < e; ++i)
    OS << Fill;
  OS << Suffix << '\n';
}

void toolchain::emitSourceFileHeader(StringRef Desc, raw_ostream &OS,
                                const RecordKeeper &Record) {
  printLine(OS, "/*===- TableGen'erated file ", '-', "*- C++ -*-===*\\");
  StringRef Prefix("|* ");
  StringRef Suffix(" *|");
  printLine(OS, Prefix, ' ', Suffix);
  size_t PSLen = Prefix.size() + Suffix.size();
  assert(PSLen < MAX_LINE_LEN);
  size_t Pos = 0U;
  do {
    size_t Length = std::min(Desc.size() - Pos, MAX_LINE_LEN - PSLen);
    printLine(OS, Prefix + Desc.substr(Pos, Length), ' ', Suffix);
    Pos += Length;
  } while (Pos < Desc.size());
  printLine(OS, Prefix, ' ', Suffix);
  printLine(OS, Prefix + "Automatically generated file, do not edit!", ' ',
            Suffix);

  // Print the filename of source file.
  if (!Record.getInputFilename().empty())
    printLine(
        OS, Prefix + "From: " + sys::path::filename(Record.getInputFilename()),
        ' ', Suffix);
  printLine(OS, Prefix, ' ', Suffix);
  printLine(OS, "\\*===", '-', "===*/");
  OS << '\n';
}
