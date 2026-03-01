//===- StringsAndChecksums.cpp --------------------------------------------===//
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

#include "vm/core/DebugInfo/CodeView/StringsAndChecksums.h"
#include "vm/core/DebugInfo/CodeView/CodeView.h"
#include "vm/core/DebugInfo/CodeView/DebugChecksumsSubsection.h"
#include "vm/core/DebugInfo/CodeView/DebugStringTableSubsection.h"
#include "vm/core/DebugInfo/CodeView/DebugSubsectionRecord.h"
#include "vm/core/Support/Error.h"
#include <cassert>

using namespace vm::core;
using namespace vm::core::codeview;

StringsAndChecksumsRef::StringsAndChecksumsRef() = default;

StringsAndChecksumsRef::StringsAndChecksumsRef(
    const DebugStringTableSubsectionRef &Strings)
    : Strings(&Strings) {}

StringsAndChecksumsRef::StringsAndChecksumsRef(
    const DebugStringTableSubsectionRef &Strings,
    const DebugChecksumsSubsectionRef &Checksums)
    : Strings(&Strings), Checksums(&Checksums) {}

void StringsAndChecksumsRef::initializeStrings(
    const DebugSubsectionRecord &SR) {
  assert(SR.kind() == DebugSubsectionKind::StringTable);
  assert(!Strings && "Found a string table even though we already have one!");

  OwnedStrings = std::make_shared<DebugStringTableSubsectionRef>();
  consumeError(OwnedStrings->initialize(SR.getRecordData()));
  Strings = OwnedStrings.get();
}

void StringsAndChecksumsRef::reset() {
  resetStrings();
  resetChecksums();
}

void StringsAndChecksumsRef::resetStrings() {
  OwnedStrings.reset();
  Strings = nullptr;
}

void StringsAndChecksumsRef::resetChecksums() {
  OwnedChecksums.reset();
  Checksums = nullptr;
}

void StringsAndChecksumsRef::setStrings(
    const DebugStringTableSubsectionRef &StringsRef) {
  OwnedStrings = std::make_shared<DebugStringTableSubsectionRef>();
  *OwnedStrings = StringsRef;
  Strings = OwnedStrings.get();
}

void StringsAndChecksumsRef::setChecksums(
    const DebugChecksumsSubsectionRef &CS) {
  OwnedChecksums = std::make_shared<DebugChecksumsSubsectionRef>();
  *OwnedChecksums = CS;
  Checksums = OwnedChecksums.get();
}

void StringsAndChecksumsRef::initializeChecksums(
    const DebugSubsectionRecord &FCR) {
  assert(FCR.kind() == DebugSubsectionKind::FileChecksums);
  if (Checksums)
    return;

  OwnedChecksums = std::make_shared<DebugChecksumsSubsectionRef>();
  consumeError(OwnedChecksums->initialize(FCR.getRecordData()));
  Checksums = OwnedChecksums.get();
}
