//===--- BasicBridging.cpp - Utilities for language bridging -----------------===//
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

#include "language/Basic/BasicBridging.h"
#include "language/Basic/Assertions.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/VersionTuple.h"
#include "toolchain/Support/raw_ostream.h"

#ifdef PURE_BRIDGING_MODE
// In PURE_BRIDGING_MODE, bridging functions are not inlined and therefore
// inluded in the cpp file.
#include "language/Basic/BasicBridgingImpl.h"
#endif

using namespace language;

void assertFail(const char * _Nonnull msg, const char * _Nonnull file,
                CodiraUInt line, const char * _Nonnull function) {
  ASSERT_failure(msg, file, line, function);
}

//===----------------------------------------------------------------------===//
// MARK: BridgedOStream
//===----------------------------------------------------------------------===//

void BridgedOStream::write(BridgedStringRef string) const {
  *os << string.unbridged();
}

void BridgedOStream::newLine() const {
  os->write('\n');
}

void BridgedOStream::flush() const {
  os->flush();
}

BridgedOStream Bridged_dbgs() {
  return BridgedOStream(&toolchain::dbgs());
}

//===----------------------------------------------------------------------===//
// MARK: BridgedStringRef
//===----------------------------------------------------------------------===//

void BridgedStringRef::write(BridgedOStream os) const {
  os.unbridged()->write(Data, Length);
}

//===----------------------------------------------------------------------===//
// MARK: BridgedOwnedString
//===----------------------------------------------------------------------===//

BridgedOwnedString::BridgedOwnedString(toolchain::StringRef stringToCopy)
    : Data(nullptr), Length(stringToCopy.size()) {
  if (Length != 0) {
    Data = new char[Length];
    std::memcpy(Data, stringToCopy.data(), Length);
  }
}

void BridgedOwnedString::destroy() const {
  if (Data)
    delete[] Data;
}

//===----------------------------------------------------------------------===//
// MARK: BridgedData
//===----------------------------------------------------------------------===//

void BridgedData::free() const {
  if (BaseAddress == nullptr)
    return;
  ::free(const_cast<char *>(BaseAddress));
}

//===----------------------------------------------------------------------===//
// MARK: BridgedCharSourceRangeVector
//===----------------------------------------------------------------------===//

BridgedCharSourceRangeVector::BridgedCharSourceRangeVector()
    : vector(new std::vector<CharSourceRange>()) {}

void BridgedCharSourceRangeVector::push_back(BridgedCharSourceRange range) {
  static_cast<std::vector<CharSourceRange> *>(vector)->push_back(
      range.unbridged());
}

//===----------------------------------------------------------------------===//
// MARK: BridgedVersionTuple
//===----------------------------------------------------------------------===//

BridgedVersionTuple::BridgedVersionTuple(toolchain::VersionTuple version) {
  if (version.getBuild())
    *this = BridgedVersionTuple(version.getMajor(), *version.getMinor(),
                                *version.getSubminor(), *version.getBuild());
  else if (version.getSubminor())
    *this = BridgedVersionTuple(version.getMajor(), *version.getMinor(),
                                *version.getSubminor());
  else if (version.getMinor())
    *this = BridgedVersionTuple(version.getMajor(), *version.getMinor());
  else
    *this = BridgedVersionTuple(version.getMajor());
}

toolchain::VersionTuple BridgedVersionTuple::unbridged() const {
  if (HasBuild)
    return toolchain::VersionTuple(Major, Minor, Subminor, Build);
  if (HasSubminor)
    return toolchain::VersionTuple(Major, Minor, Subminor);
  if (HasMinor)
    return toolchain::VersionTuple(Major, Minor);
  return toolchain::VersionTuple(Major);
}
