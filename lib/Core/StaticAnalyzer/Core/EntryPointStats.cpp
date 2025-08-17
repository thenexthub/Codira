//===- EntryPointStats.cpp --------------------------------------*- C++ -*-===//
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

#include "language/Core/StaticAnalyzer/Core/PathSensitive/EntryPointStats.h"
#include "language/Core/AST/DeclBase.h"
#include "language/Core/Analysis/AnalysisDeclContext.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/ManagedStatic.h"
#include "toolchain/Support/raw_ostream.h"
#include <iterator>

using namespace language::Core;
using namespace ento;

namespace {
struct Registry {
  std::vector<BoolEPStat *> BoolStats;
  std::vector<CounterEPStat *> CounterStats;
  std::vector<UnsignedMaxEPStat *> UnsignedMaxStats;
  std::vector<UnsignedEPStat *> UnsignedStats;

  bool IsLocked = false;

  struct Snapshot {
    const Decl *EntryPoint;
    std::vector<bool> BoolStatValues;
    std::vector<unsigned> UnsignedStatValues;

    void dumpAsCSV(toolchain::raw_ostream &OS) const;
  };

  std::vector<Snapshot> Snapshots;
};
} // namespace

static toolchain::ManagedStatic<Registry> StatsRegistry;

namespace {
template <typename Callback> void enumerateStatVectors(const Callback &Fn) {
  Fn(StatsRegistry->BoolStats);
  Fn(StatsRegistry->CounterStats);
  Fn(StatsRegistry->UnsignedMaxStats);
  Fn(StatsRegistry->UnsignedStats);
}
} // namespace

static void checkStatName(const EntryPointStat *M) {
#ifdef NDEBUG
  return;
#endif // NDEBUG
  constexpr std::array AllowedSpecialChars = {
      '+', '-', '_', '=', ':', '(',  ')', '@', '!', '~',
      '$', '%', '^', '&', '*', '\'', ';', '<', '>', '/'};
  for (unsigned char C : M->name()) {
    if (!std::isalnum(C) && !toolchain::is_contained(AllowedSpecialChars, C)) {
      toolchain::errs() << "Stat name \"" << M->name() << "\" contains character '"
                   << C << "' (" << static_cast<int>(C)
                   << ") that is not allowed.";
      assert(false && "The Stat name contains unallowed character");
    }
  }
}

void EntryPointStat::lockRegistry() {
  auto CmpByNames = [](const EntryPointStat *L, const EntryPointStat *R) {
    return L->name() < R->name();
  };
  enumerateStatVectors(
      [CmpByNames](auto &Stats) { toolchain::sort(Stats, CmpByNames); });
  enumerateStatVectors(
      [](const auto &Stats) { toolchain::for_each(Stats, checkStatName); });
  StatsRegistry->IsLocked = true;
}

[[maybe_unused]] static bool isRegistered(toolchain::StringLiteral Name) {
  auto ByName = [Name](const EntryPointStat *M) { return M->name() == Name; };
  bool Result = false;
  enumerateStatVectors([ByName, &Result](const auto &Stats) {
    Result = Result || toolchain::any_of(Stats, ByName);
  });
  return Result;
}

BoolEPStat::BoolEPStat(toolchain::StringLiteral Name) : EntryPointStat(Name) {
  assert(!StatsRegistry->IsLocked);
  assert(!isRegistered(Name));
  StatsRegistry->BoolStats.push_back(this);
}

CounterEPStat::CounterEPStat(toolchain::StringLiteral Name) : EntryPointStat(Name) {
  assert(!StatsRegistry->IsLocked);
  assert(!isRegistered(Name));
  StatsRegistry->CounterStats.push_back(this);
}

UnsignedMaxEPStat::UnsignedMaxEPStat(toolchain::StringLiteral Name)
    : EntryPointStat(Name) {
  assert(!StatsRegistry->IsLocked);
  assert(!isRegistered(Name));
  StatsRegistry->UnsignedMaxStats.push_back(this);
}

UnsignedEPStat::UnsignedEPStat(toolchain::StringLiteral Name)
    : EntryPointStat(Name) {
  assert(!StatsRegistry->IsLocked);
  assert(!isRegistered(Name));
  StatsRegistry->UnsignedStats.push_back(this);
}

static std::vector<unsigned> consumeUnsignedStats() {
  std::vector<unsigned> Result;
  Result.reserve(StatsRegistry->CounterStats.size() +
                 StatsRegistry->UnsignedMaxStats.size() +
                 StatsRegistry->UnsignedStats.size());
  for (auto *M : StatsRegistry->CounterStats) {
    Result.push_back(M->value());
    M->reset();
  }
  for (auto *M : StatsRegistry->UnsignedMaxStats) {
    Result.push_back(M->value());
    M->reset();
  }
  for (auto *M : StatsRegistry->UnsignedStats) {
    Result.push_back(M->value());
    M->reset();
  }
  return Result;
}

static std::vector<toolchain::StringLiteral> getStatNames() {
  std::vector<toolchain::StringLiteral> Ret;
  auto GetName = [](const EntryPointStat *M) { return M->name(); };
  enumerateStatVectors([GetName, &Ret](const auto &Stats) {
    transform(Stats, std::back_inserter(Ret), GetName);
  });
  return Ret;
}

void Registry::Snapshot::dumpAsCSV(toolchain::raw_ostream &OS) const {
  OS << '"';
  toolchain::printEscapedString(
      language::Core::AnalysisDeclContext::getFunctionName(EntryPoint), OS);
  OS << "\", ";
  auto PrintAsBool = [&OS](bool B) { OS << (B ? "true" : "false"); };
  toolchain::interleaveComma(BoolStatValues, OS, PrintAsBool);
  OS << ((BoolStatValues.empty() || UnsignedStatValues.empty()) ? "" : ", ");
  toolchain::interleaveComma(UnsignedStatValues, OS);
}

static std::vector<bool> consumeBoolStats() {
  std::vector<bool> Result;
  Result.reserve(StatsRegistry->BoolStats.size());
  for (auto *M : StatsRegistry->BoolStats) {
    Result.push_back(M->value());
    M->reset();
  }
  return Result;
}

void EntryPointStat::takeSnapshot(const Decl *EntryPoint) {
  auto BoolValues = consumeBoolStats();
  auto UnsignedValues = consumeUnsignedStats();
  StatsRegistry->Snapshots.push_back(
      {EntryPoint, std::move(BoolValues), std::move(UnsignedValues)});
}

void EntryPointStat::dumpStatsAsCSV(toolchain::StringRef FileName) {
  std::error_code EC;
  toolchain::raw_fd_ostream File(FileName, EC, toolchain::sys::fs::OF_Text);
  if (EC)
    return;
  dumpStatsAsCSV(File);
}

void EntryPointStat::dumpStatsAsCSV(toolchain::raw_ostream &OS) {
  OS << "EntryPoint, ";
  toolchain::interleaveComma(getStatNames(), OS);
  OS << "\n";

  std::vector<std::string> Rows;
  Rows.reserve(StatsRegistry->Snapshots.size());
  for (const auto &Snapshot : StatsRegistry->Snapshots) {
    std::string Row;
    toolchain::raw_string_ostream RowOs(Row);
    Snapshot.dumpAsCSV(RowOs);
    RowOs << "\n";
    Rows.push_back(RowOs.str());
  }
  toolchain::sort(Rows);
  for (const auto &Row : Rows) {
    OS << Row;
  }
}
