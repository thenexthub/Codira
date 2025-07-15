//===--- Mangler.cpp - Base class for Codira name mangling -----------------===//
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

#include "language/Basic/Mangler.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/PrettyStackTrace.h"
#include "language/Demangling/Demangler.h"
#include "language/Demangling/ManglingMacros.h"
#include "language/Demangling/Punycode.h"
#include "language/Parse/Lexer.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/Support/CommandLine.h"
#include <algorithm>

using namespace language;
using namespace Mangle;

#ifndef NDEBUG

toolchain::cl::opt<bool> PrintCodiraManglingStats(
    "print-language-mangling-stats", toolchain::cl::init(false),
    toolchain::cl::desc("Print statistics about Codira symbol mangling"));

namespace {

struct SizeStatEntry {
  int sizeDiff;
  std::string Old;
  std::string New;
};

static std::vector<SizeStatEntry> SizeStats;

static int numSmaller = 0;
static int numEqual = 0;
static int numLarger = 0;
static int totalOldSize = 0;
static int totalNewSize = 0;
static int mergedSubsts = 0;
static int numLargeSubsts = 0;

struct OpStatEntry {
  OpStatEntry() : num(0), size(0) { }
  
  int num;
  int size;
};

static toolchain::StringMap<OpStatEntry> OpStats;

} // end anonymous namespace

void Mangler::recordOpStatImpl(StringRef op, size_t OldPos) {
  if (PrintCodiraManglingStats) {
    OpStatEntry &E = OpStats[op];
    ++E.num;
    E.size += Storage.size() - OldPos;
  }
}

#endif // NDEBUG

void Mangle::printManglingStats() {
#ifndef NDEBUG
  if (!PrintCodiraManglingStats)
    return;

  std::sort(SizeStats.begin(), SizeStats.end(),
    [](const SizeStatEntry &LHS, const SizeStatEntry &RHS) {
      return LHS.sizeDiff < RHS.sizeDiff;
    });

  toolchain::outs() << "Mangling size stats:\n"
                  "  num smaller: " << numSmaller << "\n"
                  "  num larger:  " << numLarger << "\n"
                  "  num equal:   " << numEqual << "\n"
                  "  total old size: " << totalOldSize << "\n"
                  "  total new size: " << totalNewSize << "\n"
                  "  new - old size: " << (totalNewSize - totalOldSize) << "\n"
                  "List or larger:\n";
  for (const SizeStatEntry &E : SizeStats) {
    toolchain::outs() << "  delta " << E.sizeDiff << ": " << E.Old << " - " << E.New
                 << '\n';
  }
  
  toolchain::outs() << "Mangling operator stats:\n";

  using MapEntry = toolchain::StringMapEntry<OpStatEntry>;
  std::vector<const MapEntry *> SortedOpStats;
  for (const MapEntry &ME : OpStats) {
    SortedOpStats.push_back(&ME);
  }
  std::sort(SortedOpStats.begin(), SortedOpStats.end(),
    [](const MapEntry *LHS, const MapEntry *RHS) {
      return LHS->getKey() < RHS->getKey();
    });

  for (const MapEntry *E : SortedOpStats) {
    toolchain::outs() << "  " << E->getKey() << ": num = " << E->getValue().num
                 << ", size = " << E->getValue().size << '\n';
  }
  toolchain::outs() << "  merged substitutions: " << mergedSubsts << "\n"
                  "  large substitutions: " << numLargeSubsts << "\n";
#endif
}

void Mangler::beginManglingWithoutPrefix() {
  Storage.clear();
  Substitutions.clear();
  StringSubstitutions.clear();
  NextSubstitutionIndex = 0;
  Words.clear();
  SubstMerging.clear();
}

void Mangler::beginMangling() {
  beginManglingWithoutPrefix();

  switch (Flavor) {
  case ManglingFlavor::Default:
    Buffer << MANGLING_PREFIX_STR;
    break;
  case ManglingFlavor::Embedded:
    Buffer << MANGLING_PREFIX_EMBEDDED_STR;
    break;
  }
}

/// Finish the mangling of the symbol and return the mangled name.
std::string Mangler::finalize() {
  assert(Storage.size() && "Mangling an empty name");
  std::string result = Storage.str().str();
  Storage.clear();

#ifndef NDEBUG
  switch (Flavor) {
  case ManglingFlavor::Default:
    if (StringRef(result).starts_with(MANGLING_PREFIX_STR))
      verify(result, Flavor);
    break;
  case ManglingFlavor::Embedded:
    if (StringRef(result).starts_with(MANGLING_PREFIX_EMBEDDED_STR))
      verify(result, Flavor);
    break;
  }
#endif

  return result;
}

/// Finish the mangling of the symbol and write the mangled name into
/// \p stream.
void Mangler::finalize(toolchain::raw_ostream &stream) {
  std::string result = finalize();
  stream.write(result.data(), result.size());
}

TOOLCHAIN_ATTRIBUTE_UNUSED
static bool treeContains(Demangle::NodePointer Nd, Demangle::Node::Kind Kind) {
  if (Nd->getKind() == Kind)
    return true;

  for (auto Child : *Nd) {
    if (treeContains(Child, Kind))
      return true;
  }
  return false;
}

void Mangler::verify(StringRef nameStr, ManglingFlavor Flavor) {
  SmallString<128> buffer;
  if (!nameStr.starts_with(MANGLING_PREFIX_STR) &&
      !nameStr.starts_with(MANGLING_PREFIX_EMBEDDED_STR) &&
      !nameStr.starts_with("_Tt") &&
      !nameStr.starts_with("_S")) {
    // This list is the set of prefixes recognized by Demangler::demangleSymbol.
    // It should be kept in sync.
    assert(StringRef(MANGLING_PREFIX_STR) != "_S" && "redundant check");

    switch (Flavor) {
    case ManglingFlavor::Default:
      buffer += MANGLING_PREFIX_STR;
      break;
    case ManglingFlavor::Embedded:
      buffer += MANGLING_PREFIX_EMBEDDED_STR;
      break;
    }

    buffer += nameStr;
    nameStr = buffer.str();
  }

  Demangler Dem;
  NodePointer Root = Dem.demangleSymbol(nameStr);
  if (!Root || treeContains(Root, Node::Kind::Suffix)) {
    ABORT([&](auto &out) {
      out << "Can't demangle: " << nameStr;
    });
  }
  auto mangling = mangleNode(Root, Flavor);
  if (!mangling.isSuccess()) {
    ABORT([&](auto &out) {
      out << "Can't remangle: " << nameStr;
    });
  }
  std::string Remangled = mangling.result();
  if (Remangled == nameStr)
    return;

  ABORT([&](auto &out) {
    out << "Remangling failed:\n";
    out << "original     = " << nameStr << "\n";
    out << "remangled    = " << Remangled;
  });
}

void Mangler::appendIdentifier(StringRef ident, bool allowRawIdentifiers) {
  auto Iter = StringSubstitutions.find(ident);
  if (Iter != StringSubstitutions.end())
    return mangleSubstitution(Iter->second);

  size_t OldPos = Storage.size();
  addSubstitution(ident);

  if (allowRawIdentifiers && Lexer::identifierMustAlwaysBeEscaped(ident)) {
    toolchain::SmallString<256> escaped;
    appendRawIdentifierForRuntime(ident, escaped);
    mangleIdentifier(*this, escaped);
  } else {
    mangleIdentifier(*this, ident);
  }

  recordOpStat("<identifier>", OldPos);
}

void Mangler::appendRawIdentifierForRuntime(
    StringRef ident, toolchain::SmallVectorImpl<char> &buffer) {
  // SE-0451: Raw identifiers retain their backticks as part of their
  // mangling. Additionally, the runtime has historically used spaces
  // (U+0020) as delimiters in some metadata (e.g., tuple element labels
  // and protocol associated type names). If one of these names is a raw
  // identifier, it may also contain U+0020. To address this in a
  // backwards-compatible fashion, we replace any occurrences of U+0020 in
  // the name with U+00A0 (UTF-8: 0xC2 0xA0), since U+00A0 is not permitted
  // in raw identifiers.
  buffer.push_back('`');
  for (auto ch : ident) {
    if (ch == ' ') {
      buffer.push_back(0xc2);
      buffer.push_back(0xa0);
    } else {
      buffer.push_back(ch);
    }
  }
  buffer.push_back('`');
}

void Mangler::dump() const {
  // FIXME: const_casting because toolchain::raw_svector_ostream::str() is
  // incorrectly not marked const.
  toolchain::errs() << const_cast<Mangler*>(this)->Buffer.str() << '\n';
}

bool Mangler::tryMangleSubstitution(const void *ptr) {
  auto ir = Substitutions.find(ptr);
  if (ir == Substitutions.end())
    return false;

  mangleSubstitution(ir->second);
  return true;
}

void Mangler::mangleSubstitution(unsigned Idx) {
  if (Idx >= 26) {
#ifndef NDEBUG
    ++numLargeSubsts;
#endif
    return appendOperator("A", Index(Idx - 26));
  }

  char SubstChar = Idx + 'A';
  StringRef Subst(&SubstChar, 1);
  if (SubstMerging.tryMergeSubst(*this, Subst, /*isStandardSubst*/ false)) {
#ifndef NDEBUG
    ++mergedSubsts;
#endif
  } else {
    appendOperator("A", Subst);
  }
}

