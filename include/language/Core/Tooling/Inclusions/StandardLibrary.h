//===--- StandardLibrary.h --------------------------------------*- C++ -*-===//
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
///
/// \file
/// Provides an interface for querying information about C and C++ Standard
/// Library headers and symbols.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_INCLUSIONS_STANDARDLIBRARY_H
#define LANGUAGE_CORE_TOOLING_INCLUSIONS_STANDARDLIBRARY_H

#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/raw_ostream.h"
#include <optional>
#include <string>
#include <vector>

namespace language::Core {
class Decl;
class NamespaceDecl;
class DeclContext;
namespace tooling {
namespace stdlib {

class Symbol;
enum class Lang { C = 0, CXX, LastValue = CXX };

// A standard library header, such as <iostream>
// Lightweight class, in fact just an index into a table.
// C++ and C Library compatibility headers are considered different: e.g.
// "<cstdio>" and "<stdio.h>" (and their symbols) are treated differently.
class Header {
public:
  static std::vector<Header> all(Lang L = Lang::CXX);
  // Name should contain the angle brackets, e.g. "<vector>".
  static std::optional<Header> named(toolchain::StringRef Name,
                                     Lang Language = Lang::CXX);

  friend toolchain::raw_ostream &operator<<(toolchain::raw_ostream &OS, const Header &H) {
    return OS << H.name();
  }
  toolchain::StringRef name() const;

private:
  Header(unsigned ID, Lang Language) : ID(ID), Language(Language) {}
  unsigned ID;
  Lang Language;

  friend Symbol;
  friend toolchain::DenseMapInfo<Header>;
  friend bool operator==(const Header &L, const Header &R) {
    return L.ID == R.ID;
  }
};

// A top-level standard library symbol, such as std::vector
// Lightweight class, in fact just an index into a table.
// C++ and C Standard Library symbols are considered distinct: e.g. std::printf
// and ::printf are not treated as the same symbol.
// The symbols do not contain macros right now, we don't have a reliable index
// for them.
class Symbol {
public:
  static std::vector<Symbol> all(Lang L = Lang::CXX);
  /// \p Scope should have the trailing "::", for example:
  /// named("std::chrono::", "system_clock")
  static std::optional<Symbol>
  named(toolchain::StringRef Scope, toolchain::StringRef Name, Lang Language = Lang::CXX);

  friend toolchain::raw_ostream &operator<<(toolchain::raw_ostream &OS, const Symbol &S) {
    return OS << S.qualifiedName();
  }
  toolchain::StringRef scope() const;
  toolchain::StringRef name() const;
  toolchain::StringRef qualifiedName() const;
  // The preferred header for this symbol (e.g. the suggested insertion).
  std::optional<Header> header() const;
  // Some symbols may be provided by multiple headers.
  toolchain::SmallVector<Header> headers() const;

private:
  Symbol(unsigned ID, Lang Language) : ID(ID), Language(Language) {}
  unsigned ID;
  Lang Language;

  friend class Recognizer;
  friend toolchain::DenseMapInfo<Symbol>;
  friend bool operator==(const Symbol &L, const Symbol &R) {
    return L.ID == R.ID;
  }
};

// A functor to find the stdlib::Symbol associated with a decl.
//
// For non-top-level decls (std::vector<int>::iterator), returns the top-level
// symbol (std::vector).
class Recognizer {
public:
  Recognizer();
  std::optional<Symbol> operator()(const Decl *D);

private:
  using NSSymbolMap = toolchain::DenseMap<toolchain::StringRef, unsigned>;
  NSSymbolMap *namespaceSymbols(const DeclContext *DC, Lang L);
  toolchain::DenseMap<const DeclContext *, NSSymbolMap *> NamespaceCache;
};

} // namespace stdlib
} // namespace tooling
} // namespace language::Core

namespace toolchain {

template <> struct DenseMapInfo<language::Core::tooling::stdlib::Header> {
  static inline language::Core::tooling::stdlib::Header getEmptyKey() {
    return language::Core::tooling::stdlib::Header(-1,
                                          language::Core::tooling::stdlib::Lang::CXX);
  }
  static inline language::Core::tooling::stdlib::Header getTombstoneKey() {
    return language::Core::tooling::stdlib::Header(-2,
                                          language::Core::tooling::stdlib::Lang::CXX);
  }
  static unsigned getHashValue(const language::Core::tooling::stdlib::Header &H) {
    return hash_value(H.ID);
  }
  static bool isEqual(const language::Core::tooling::stdlib::Header &LHS,
                      const language::Core::tooling::stdlib::Header &RHS) {
    return LHS == RHS;
  }
};

template <> struct DenseMapInfo<language::Core::tooling::stdlib::Symbol> {
  static inline language::Core::tooling::stdlib::Symbol getEmptyKey() {
    return language::Core::tooling::stdlib::Symbol(-1,
                                          language::Core::tooling::stdlib::Lang::CXX);
  }
  static inline language::Core::tooling::stdlib::Symbol getTombstoneKey() {
    return language::Core::tooling::stdlib::Symbol(-2,
                                          language::Core::tooling::stdlib::Lang::CXX);
  }
  static unsigned getHashValue(const language::Core::tooling::stdlib::Symbol &S) {
    return hash_value(S.ID);
  }
  static bool isEqual(const language::Core::tooling::stdlib::Symbol &LHS,
                      const language::Core::tooling::stdlib::Symbol &RHS) {
    return LHS == RHS;
  }
};
} // namespace toolchain

#endif // LANGUAGE_CORE_TOOLING_INCLUSIONS_STANDARDLIBRARY_H
