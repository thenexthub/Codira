//=-- MemProfSummary.cpp - MemProf summary support ---------------=//
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
// This file contains MemProf summary support.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ProfileData/MemProfSummary.h"

using namespace vm::core;
using namespace vm::core::memprof;

void MemProfSummary::printSummaryYaml(raw_ostream &OS) const {
  // For now emit as YAML comments, since they aren't read on input.
  OS << "---\n";
  OS << "# MemProfSummary:\n";
  OS << "#   Total contexts: " << NumContexts << "\n";
  OS << "#   Total cold contexts: " << NumColdContexts << "\n";
  OS << "#   Total hot contexts: " << NumHotContexts << "\n";
  OS << "#   Maximum cold context total size: " << MaxColdTotalSize << "\n";
  OS << "#   Maximum warm context total size: " << MaxWarmTotalSize << "\n";
  OS << "#   Maximum hot context total size: " << MaxHotTotalSize << "\n";
  if (HasDataAccessProfile) {
    OS << "#   Num hot symbols and string literals: "
       << NumHotSymbolsAndStringLiterals << "\n";
    OS << "#   Num known cold symbols: " << NumKnownColdSymbols << "\n";
    OS << "#   Num known cold string literals: " << NumKnownColdStringLiterals
       << "\n";
  }
}

void MemProfSummary::write(ProfOStream &OS) const {
  // Write the current number of fields first, which helps enable backwards and
  // forwards compatibility (see comment in header).
  OS.write32(memprof::MemProfSummary::getNumSummaryFields());
  auto StartPos = OS.tell();
  (void)StartPos;
  OS.write(NumContexts);
  OS.write(NumColdContexts);
  OS.write(NumHotContexts);
  OS.write(MaxColdTotalSize);
  OS.write(MaxWarmTotalSize);
  OS.write(MaxHotTotalSize);
  // Sanity check that the number of fields was kept in sync with actual fields.
  assert((OS.tell() - StartPos) / 8 == MemProfSummary::getNumSummaryFields());
}

std::unique_ptr<MemProfSummary>
MemProfSummary::deserialize(const unsigned char *&Ptr) {
  auto NumSummaryFields =
      support::endian::readNext<uint32_t, toolchain::endianness::little>(Ptr);
  // The initial version of the summary contains 6 fields. To support backwards
  // compatibility with older profiles, if new summary fields are added (until a
  // version bump) this code will need to check NumSummaryFields against the
  // current value of MemProfSummary::getNumSummaryFields(). If NumSummaryFields
  // is lower then default values will need to be filled in for the newer fields
  // instead of trying to read them from the profile.
  //
  // For now, assert that the profile contains at least as many fields as
  // expected by the code.
  assert(NumSummaryFields >= MemProfSummary::getNumSummaryFields());

  auto MemProfSum = std::make_unique<MemProfSummary>(
      support::endian::read<uint64_t, toolchain::endianness::little>(Ptr),
      support::endian::read<uint64_t, toolchain::endianness::little>(Ptr + 8),
      support::endian::read<uint64_t, toolchain::endianness::little>(Ptr + 16),
      support::endian::read<uint64_t, toolchain::endianness::little>(Ptr + 24),
      support::endian::read<uint64_t, toolchain::endianness::little>(Ptr + 32),
      support::endian::read<uint64_t, toolchain::endianness::little>(Ptr + 40));

  // Enable forwards compatibility by skipping past any additional fields in the
  // profile's summary.
  Ptr += NumSummaryFields * sizeof(uint64_t);

  return MemProfSum;
}

// FIXME: Consider to serialize the data access summary fields, ideally
// batch this together with more substantial profile format change
// and bump version once.
void MemProfSummary::buildDataAccessSummary(
    const DataAccessProfData &DataAccessProfile) {
  HasDataAccessProfile = true;
  NumHotSymbolsAndStringLiterals = DataAccessProfile.getRecords().size();
  NumKnownColdSymbols = DataAccessProfile.getKnownColdSymbols().size();
  NumKnownColdStringLiterals = DataAccessProfile.getKnownColdHashes().size();
}
