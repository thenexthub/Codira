// EntryPointStats.h - Tracking statistics per entry point ------*- C++ -*-===//
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

#ifndef CLANG_INCLUDE_CLANG_STATICANALYZER_CORE_PATHSENSITIVE_ENTRYPOINTSTATS_H
#define CLANG_INCLUDE_CLANG_STATICANALYZER_CORE_PATHSENSITIVE_ENTRYPOINTSTATS_H

#include "toolchain/ADT/Statistic.h"
#include "toolchain/ADT/StringRef.h"

namespace toolchain {
class raw_ostream;
} // namespace toolchain

namespace language::Core {
class Decl;

namespace ento {

class EntryPointStat {
public:
  toolchain::StringLiteral name() const { return Name; }

  static void lockRegistry();

  static void takeSnapshot(const Decl *EntryPoint);
  static void dumpStatsAsCSV(toolchain::raw_ostream &OS);
  static void dumpStatsAsCSV(toolchain::StringRef FileName);

protected:
  explicit EntryPointStat(toolchain::StringLiteral Name) : Name{Name} {}
  EntryPointStat(const EntryPointStat &) = delete;
  EntryPointStat(EntryPointStat &&) = delete;
  EntryPointStat &operator=(EntryPointStat &) = delete;
  EntryPointStat &operator=(EntryPointStat &&) = delete;

private:
  toolchain::StringLiteral Name;
};

class BoolEPStat : public EntryPointStat {
  std::optional<bool> Value = {};

public:
  explicit BoolEPStat(toolchain::StringLiteral Name);
  unsigned value() const { return Value && *Value; }
  void set(bool V) {
    assert(!Value.has_value());
    Value = V;
  }
  void reset() { Value = {}; }
};

// used by CounterEntryPointTranslationUnitStat
class CounterEPStat : public EntryPointStat {
  using EntryPointStat::EntryPointStat;
  unsigned Value = {};

public:
  explicit CounterEPStat(toolchain::StringLiteral Name);
  unsigned value() const { return Value; }
  void reset() { Value = {}; }
  CounterEPStat &operator++() {
    ++Value;
    return *this;
  }

  CounterEPStat &operator++(int) {
    // No difference as you can't extract the value
    return ++(*this);
  }

  CounterEPStat &operator+=(unsigned Inc) {
    Value += Inc;
    return *this;
  }
};

// used by UnsignedMaxEtryPointTranslationUnitStatistic
class UnsignedMaxEPStat : public EntryPointStat {
  using EntryPointStat::EntryPointStat;
  unsigned Value = {};

public:
  explicit UnsignedMaxEPStat(toolchain::StringLiteral Name);
  unsigned value() const { return Value; }
  void reset() { Value = {}; }
  void updateMax(unsigned X) { Value = std::max(Value, X); }
};

class UnsignedEPStat : public EntryPointStat {
  using EntryPointStat::EntryPointStat;
  std::optional<unsigned> Value = {};

public:
  explicit UnsignedEPStat(toolchain::StringLiteral Name);
  unsigned value() const { return Value.value_or(0); }
  void reset() { Value.reset(); }
  void set(unsigned V) {
    assert(!Value.has_value());
    Value = V;
  }
};

class CounterEntryPointTranslationUnitStat {
  CounterEPStat M;
  toolchain::TrackingStatistic S;

public:
  CounterEntryPointTranslationUnitStat(const char *DebugType,
                                       toolchain::StringLiteral Name,
                                       toolchain::StringLiteral Desc)
      : M(Name), S(DebugType, Name.data(), Desc.data()) {}
  CounterEntryPointTranslationUnitStat &operator++() {
    ++M;
    ++S;
    return *this;
  }

  CounterEntryPointTranslationUnitStat &operator++(int) {
    // No difference with prefix as the value is not observable.
    return ++(*this);
  }

  CounterEntryPointTranslationUnitStat &operator+=(unsigned Inc) {
    M += Inc;
    S += Inc;
    return *this;
  }
};

class UnsignedMaxEntryPointTranslationUnitStatistic {
  UnsignedMaxEPStat M;
  toolchain::TrackingStatistic S;

public:
  UnsignedMaxEntryPointTranslationUnitStatistic(const char *DebugType,
                                                toolchain::StringLiteral Name,
                                                toolchain::StringLiteral Desc)
      : M(Name), S(DebugType, Name.data(), Desc.data()) {}
  void updateMax(uint64_t Value) {
    M.updateMax(static_cast<unsigned>(Value));
    S.updateMax(Value);
  }
};

#define STAT_COUNTER(VARNAME, DESC)                                            \
  static language::Core::ento::CounterEntryPointTranslationUnitStat VARNAME = {         \
      DEBUG_TYPE, #VARNAME, DESC}

#define STAT_MAX(VARNAME, DESC)                                                \
  static language::Core::ento::UnsignedMaxEntryPointTranslationUnitStatistic VARNAME =  \
      {DEBUG_TYPE, #VARNAME, DESC}

} // namespace ento
} // namespace language::Core

#endif // CLANG_INCLUDE_CLANG_STATICANALYZER_CORE_PATHSENSITIVE_ENTRYPOINTSTATS_H
