//===- MapFile.cpp --------------------------------------------------------===//
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
// This file implements the -Map option. It shows lists in order and
// hierarchically the output sections, input sections, input files and
// symbol:
//
//       Addr      Off   Size    Out     In      Symbol
//          - 00000015     10    .text
//          - 0000000e     10            test.o:(.text)
//          - 00000000      5                    local
//          - 00000000      5                    f(int)
//
//===----------------------------------------------------------------------===//

#include "MapFile.h"
#include "InputElement.h"
#include "InputFiles.h"
#include "OutputSections.h"
#include "OutputSegment.h"
#include "Symbols.h"
#include "SyntheticSections.h"
#include "llvm/Support/Parallel.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::object;
using namespace lld;
using namespace lld::wasm;

using SymbolMapTy = DenseMap<const InputChunk *, SmallVector<Symbol *, 4>>;

// Print out the first three columns of a line.
static void writeHeader(raw_ostream &os, int64_t vma, uint64_t lma,
                        uint64_t size) {
  // Not all entries in the map has a virtual memory address (e.g. functions)
  if (vma == -1)
    os << format("       - %8llx %8llx ", lma, size);
  else
    os << format("%8llx %8llx %8llx ", vma, lma, size);
}

// Returns a list of all symbols that we want to print out.
static std::vector<Symbol *> getSymbols() {
  std::vector<Symbol *> v;
  for (InputFile *file : ctx.objectFiles)
    for (Symbol *b : file->getSymbols())
      if (auto *dr = dyn_cast<Symbol>(b))
        if ((!isa<SectionSymbol>(dr)) && dr->isLive() &&
            (dr->getFile() == file))
          v.push_back(dr);
  return v;
}

// Returns a map from sections to their symbols.
static SymbolMapTy getSectionSyms(ArrayRef<Symbol *> syms) {
  SymbolMapTy ret;
  for (Symbol *dr : syms)
    ret[dr->getChunk()].push_back(dr);
  return ret;
}

// Construct a map from symbols to their stringified representations.
// Demangling symbols (which is what toString() does) is slow, so
// we do that in batch using parallel-for.
static DenseMap<Symbol *, std::string>
getSymbolStrings(ArrayRef<Symbol *> syms) {
  std::vector<std::string> str(syms.size());
  parallelFor(0, syms.size(), [&](size_t i) {
    raw_string_ostream os(str[i]);
    auto *chunk = syms[i]->getChunk();
    if (chunk == nullptr)
      return;
    uint64_t fileOffset = chunk->outputSec != nullptr
                              ? chunk->outputSec->getOffset() + chunk->outSecOff
                              : 0;
    uint64_t vma = -1;
    uint64_t size = 0;
    if (auto *DD = dyn_cast<DefinedData>(syms[i])) {
      vma = DD->getVA();
      size = DD->getSize();
      fileOffset += DD->value;
    }
    if (auto *DF = dyn_cast<DefinedFunction>(syms[i])) {
      size = DF->function->getSize();
    }
    writeHeader(os, vma, fileOffset, size);
    os.indent(16) << toString(*syms[i]);
  });

  DenseMap<Symbol *, std::string> ret;
  for (size_t i = 0, e = syms.size(); i < e; ++i)
    ret[syms[i]] = std::move(str[i]);
  return ret;
}

void lld::wasm::writeMapFile(ArrayRef<OutputSection *> outputSections) {
  if (ctx.arg.mapFile.empty())
    return;

  // Open a map file for writing.
  std::error_code ec;
  raw_fd_ostream os(ctx.arg.mapFile, ec, sys::fs::OF_None);
  if (ec) {
    error("cannot open " + ctx.arg.mapFile + ": " + ec.message());
    return;
  }

  // Collect symbol info that we want to print out.
  std::vector<Symbol *> syms = getSymbols();
  SymbolMapTy sectionSyms = getSectionSyms(syms);
  DenseMap<Symbol *, std::string> symStr = getSymbolStrings(syms);

  // Print out the header line.
  os << "    Addr      Off     Size Out     In      Symbol\n";

  for (OutputSection *osec : outputSections) {
    writeHeader(os, -1, osec->getOffset(), osec->getSize());
    os << toString(*osec) << '\n';
    if (auto *code = dyn_cast<CodeSection>(osec)) {
      for (auto *chunk : code->functions) {
        writeHeader(os, -1, chunk->outputSec->getOffset() + chunk->outSecOff,
                    chunk->getSize());
        os.indent(8) << toString(chunk) << '\n';
        for (Symbol *sym : sectionSyms[chunk])
          os << symStr[sym] << '\n';
      }
    } else if (auto *data = dyn_cast<DataSection>(osec)) {
      for (auto *oseg : data->segments) {
        writeHeader(os, oseg->startVA, data->getOffset() + oseg->sectionOffset,
                    oseg->size);
        os << oseg->name << '\n';
        for (auto *chunk : oseg->inputSegments) {
          uint64_t offset =
              chunk->outputSec != nullptr
                  ? chunk->outputSec->getOffset() + chunk->outSecOff
                  : 0;
          writeHeader(os, chunk->getVA(), offset, chunk->getSize());
          os.indent(8) << toString(chunk) << '\n';
          for (Symbol *sym : sectionSyms[chunk])
            os << symStr[sym] << '\n';
        }
      }
    } else if (auto *globals = dyn_cast<GlobalSection>(osec)) {
      for (auto *global : globals->inputGlobals) {
        writeHeader(os, global->getAssignedIndex(), 0, 0);
        os.indent(8) << global->getName() << '\n';
      }
    }
    // TODO: other section/symbol types
  }
}
