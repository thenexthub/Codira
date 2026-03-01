//===- MinGW.h --------------------------------------------------*- C++ -*-===//
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

#ifndef LLD_COFF_MINGW_H
#define LLD_COFF_MINGW_H

#include "Config.h"
#include "Symbols.h"
#include "lld/Common/LLVM.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Option/ArgList.h"
#include <vector>

namespace lld::coff {
class COFFLinkerContext;

// Logic for deciding what symbols to export, when exporting all
// symbols for MinGW.
class AutoExporter {
public:
  AutoExporter(SymbolTable &symtab,
               const llvm::DenseSet<StringRef> &manualExcludeSymbols);

  void addWholeArchive(StringRef path);
  void addExcludedSymbol(StringRef symbol);

  llvm::StringSet<> excludeSymbols;
  llvm::StringSet<> excludeSymbolPrefixes;
  llvm::StringSet<> excludeSymbolSuffixes;
  llvm::StringSet<> excludeLibs;
  llvm::StringSet<> excludeObjects;

  const llvm::DenseSet<StringRef> &manualExcludeSymbols;

  bool shouldExport(Defined *sym) const;

private:
  SymbolTable &symtab;
};

void writeDefFile(COFFLinkerContext &, StringRef name,
                  const std::vector<Export> &exports);

// The -wrap option is a feature to rename symbols so that you can write
// wrappers for existing functions. If you pass `-wrap:foo`, all
// occurrences of symbol `foo` are resolved to `__wrap_foo` (so, you are
// expected to write `__wrap_foo` function as a wrapper). The original
// symbol becomes accessible as `__real_foo`, so you can call that from your
// wrapper.
//
void addWrappedSymbols(SymbolTable &symtab, llvm::opt::InputArgList &args);

void wrapSymbols(SymbolTable &symtab);

} // namespace lld::coff

#endif
