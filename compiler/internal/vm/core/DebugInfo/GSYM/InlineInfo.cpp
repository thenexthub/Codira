//===- InlineInfo.cpp -------------------------------------------*- C++ -*-===//
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

#include "vm/core/DebugInfo/GSYM/InlineInfo.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/DebugInfo/GSYM/FileEntry.h"
#include "vm/core/DebugInfo/GSYM/FileWriter.h"
#include "vm/core/DebugInfo/GSYM/GsymReader.h"
#include "vm/core/Support/DataExtractor.h"
#include "vm/core/Support/InterleavedRange.h"
#include <inttypes.h>

using namespace vm::core;
using namespace gsym;


raw_ostream &toolchain::gsym::operator<<(raw_ostream &OS, const InlineInfo &II) {
  if (!II.isValid())
    return OS;
  OS << interleaved(II.Ranges, " ");
  OS << " Name = " << HEX32(II.Name) << ", CallFile = " << II.CallFile
     << ", CallLine = " << II.CallFile << '\n';
  for (const auto &Child : II.Children)
    OS << Child;
  return OS;
}

static bool getInlineStackHelper(const InlineInfo &II, uint64_t Addr,
    std::vector<const InlineInfo *> &InlineStack) {
  if (II.Ranges.contains(Addr)) {
    // If this is the top level that represents the concrete function,
    // there will be no name and we shoud clear the inline stack. Otherwise
    // we have found an inline call stack that we need to insert.
    if (II.Name != 0)
      InlineStack.insert(InlineStack.begin(), &II);
    for (const auto &Child : II.Children) {
      if (::getInlineStackHelper(Child, Addr, InlineStack))
        break;
    }
    return !InlineStack.empty();
  }
  return false;
}

std::optional<InlineInfo::InlineArray>
InlineInfo::getInlineStack(uint64_t Addr) const {
  InlineArray Result;
  if (getInlineStackHelper(*this, Addr, Result))
    return Result;
  return std::nullopt;
}

/// Skip an InlineInfo object in the specified data at the specified offset.
///
/// Used during the InlineInfo::lookup() call to quickly skip child InlineInfo
/// objects where the addres ranges isn't contained in the InlineInfo object
/// or its children. This avoids allocations by not appending child InlineInfo
/// objects to the InlineInfo::Children array.
///
/// \param Data The binary stream to read the data from.
///
/// \param Offset The byte offset within \a Data.
///
/// \param SkippedRanges If true, address ranges have already been skipped.

static bool skip(DataExtractor &Data, uint64_t &Offset, bool SkippedRanges) {
  if (!SkippedRanges) {
    if (skipRanges(Data, Offset) == 0)
      return false;
  }
  bool HasChildren = Data.getU8(&Offset) != 0;
  Data.getU32(&Offset); // Skip Inline.Name.
  Data.getULEB128(&Offset); // Skip Inline.CallFile.
  Data.getULEB128(&Offset); // Skip Inline.CallLine.
  if (HasChildren) {
    while (skip(Data, Offset, false /* SkippedRanges */))
      /* Do nothing */;
  }
  // We skipped a valid InlineInfo.
  return true;
}

/// A Lookup helper functions.
///
/// Used during the InlineInfo::lookup() call to quickly only parse an
/// InlineInfo object if the address falls within this object. This avoids
/// allocations by not appending child InlineInfo objects to the
/// InlineInfo::Children array and also skips any InlineInfo objects that do
/// not contain the address we are looking up.
///
/// \param Data The binary stream to read the data from.
///
/// \param Offset The byte offset within \a Data.
///
/// \param BaseAddr The address that the relative address range offsets are
///                 relative to.

static bool lookup(const GsymReader &GR, DataExtractor &Data, uint64_t &Offset,
                   uint64_t BaseAddr, uint64_t Addr, SourceLocations &SrcLocs,
                   toolchain::Error &Err) {
  InlineInfo Inline;
  decodeRanges(Inline.Ranges, Data, BaseAddr, Offset);
  if (Inline.Ranges.empty())
    return true;
  // Check if the address is contained within the inline information, and if
  // not, quickly skip this InlineInfo object and all its children.
  if (!Inline.Ranges.contains(Addr)) {
    skip(Data, Offset, true /* SkippedRanges */);
    return false;
  }

  // The address range is contained within this InlineInfo, add the source
  // location for this InlineInfo and any children that contain the address.
  bool HasChildren = Data.getU8(&Offset) != 0;
  Inline.Name = Data.getU32(&Offset);
  Inline.CallFile = (uint32_t)Data.getULEB128(&Offset);
  Inline.CallLine = (uint32_t)Data.getULEB128(&Offset);
  if (HasChildren) {
    // Child address ranges are encoded relative to the first address in the
    // parent InlineInfo object.
    const auto ChildBaseAddr = Inline.Ranges[0].start();
    bool Done = false;
    while (!Done)
      Done = lookup(GR, Data, Offset, ChildBaseAddr, Addr, SrcLocs, Err);
  }

  std::optional<FileEntry> CallFile = GR.getFile(Inline.CallFile);
  if (!CallFile) {
    Err = createStringError(std::errc::invalid_argument,
                            "failed to extract file[%" PRIu32 "]",
                            Inline.CallFile);
    return false;
  }

  if (CallFile->Dir || CallFile->Base) {
    SourceLocation SrcLoc;
    SrcLoc.Name = SrcLocs.back().Name;
    SrcLoc.Offset = SrcLocs.back().Offset;
    SrcLoc.Dir = GR.getString(CallFile->Dir);
    SrcLoc.Base = GR.getString(CallFile->Base);
    SrcLoc.Line = Inline.CallLine;
    SrcLocs.back().Name = GR.getString(Inline.Name);
    SrcLocs.back().Offset = Addr - Inline.Ranges[0].start();
    SrcLocs.push_back(SrcLoc);
  }
  return true;
}

toolchain::Error InlineInfo::lookup(const GsymReader &GR, DataExtractor &Data,
                               uint64_t BaseAddr, uint64_t Addr,
                               SourceLocations &SrcLocs) {
  // Call our recursive helper function starting at offset zero.
  uint64_t Offset = 0;
  toolchain::Error Err = Error::success();
  ::lookup(GR, Data, Offset, BaseAddr, Addr, SrcLocs, Err);
  return Err;
}

/// Decode an InlineInfo in Data at the specified offset.
///
/// A local helper function to decode InlineInfo objects. This function is
/// called recursively when parsing child InlineInfo objects.
///
/// \param Data The data extractor to decode from.
/// \param Offset The offset within \a Data to decode from.
/// \param BaseAddr The base address to use when decoding address ranges.
/// \returns An InlineInfo or an error describing the issue that was
/// encountered during decoding.
static toolchain::Expected<InlineInfo> decode(DataExtractor &Data, uint64_t &Offset,
                                         uint64_t BaseAddr) {
  InlineInfo Inline;
  if (!Data.isValidOffset(Offset))
    return createStringError(std::errc::io_error,
        "0x%8.8" PRIx64 ": missing InlineInfo address ranges data", Offset);
  decodeRanges(Inline.Ranges, Data, BaseAddr, Offset);
  if (Inline.Ranges.empty())
    return Inline;
  if (!Data.isValidOffsetForDataOfSize(Offset, 1))
    return createStringError(std::errc::io_error,
        "0x%8.8" PRIx64 ": missing InlineInfo uint8_t indicating children",
        Offset);
  bool HasChildren = Data.getU8(&Offset) != 0;
  if (!Data.isValidOffsetForDataOfSize(Offset, 4))
    return createStringError(std::errc::io_error,
        "0x%8.8" PRIx64 ": missing InlineInfo uint32_t for name", Offset);
  Inline.Name = Data.getU32(&Offset);
  if (!Data.isValidOffset(Offset))
    return createStringError(std::errc::io_error,
        "0x%8.8" PRIx64 ": missing ULEB128 for InlineInfo call file", Offset);
  Inline.CallFile = (uint32_t)Data.getULEB128(&Offset);
  if (!Data.isValidOffset(Offset))
    return createStringError(std::errc::io_error,
        "0x%8.8" PRIx64 ": missing ULEB128 for InlineInfo call line", Offset);
  Inline.CallLine = (uint32_t)Data.getULEB128(&Offset);
  if (HasChildren) {
    // Child address ranges are encoded relative to the first address in the
    // parent InlineInfo object.
    const auto ChildBaseAddr = Inline.Ranges[0].start();
    while (true) {
      toolchain::Expected<InlineInfo> Child = decode(Data, Offset, ChildBaseAddr);
      if (!Child)
        return Child.takeError();
      // InlineInfo with empty Ranges termintes a child sibling chain.
      if (Child.get().Ranges.empty())
        break;
      Inline.Children.emplace_back(std::move(*Child));
    }
  }
  return Inline;
}

toolchain::Expected<InlineInfo> InlineInfo::decode(DataExtractor &Data,
                                              uint64_t BaseAddr) {
  uint64_t Offset = 0;
  return ::decode(Data, Offset, BaseAddr);
}

toolchain::Error InlineInfo::encode(FileWriter &O, uint64_t BaseAddr) const {
  // Users must verify the InlineInfo is valid prior to calling this funtion.
  // We don't want to emit any InlineInfo objects if they are not valid since
  // it will waste space in the GSYM file.
  if (!isValid())
    return createStringError(std::errc::invalid_argument,
                             "attempted to encode invalid InlineInfo object");
  encodeRanges(Ranges, O, BaseAddr);
  bool HasChildren = !Children.empty();
  O.writeU8(HasChildren);
  O.writeU32(Name);
  O.writeULEB(CallFile);
  O.writeULEB(CallLine);
  if (HasChildren) {
    // Child address ranges are encoded as relative to the first
    // address in the Ranges for this object. This keeps the offsets
    // small and allows for efficient encoding using ULEB offsets.
    const uint64_t ChildBaseAddr = Ranges[0].start();
    for (const auto &Child : Children) {
      // Make sure all child address ranges are contained in the parent address
      // ranges.
      for (const auto &ChildRange: Child.Ranges) {
        if (!Ranges.contains(ChildRange))
          return createStringError(std::errc::invalid_argument,
                                   "child range not contained in parent");
      }
      toolchain::Error Err = Child.encode(O, ChildBaseAddr);
      if (Err)
        return Err;
    }

    // Terminate child sibling chain by emitting a zero. This zero will cause
    // the decodeAll() function above to return false and stop the decoding
    // of child InlineInfo objects that are siblings.
    O.writeULEB(0);
  }
  return Error::success();
}

static uint64_t GetTotalNumChildren(const InlineInfo &II) {
  uint64_t NumChildren = II.Children.size();
  for (const auto &Child : II.Children)
    NumChildren += GetTotalNumChildren(Child);
  return NumChildren;
}

bool InlineInfo::operator<(const InlineInfo &RHS) const {
  return GetTotalNumChildren(*this) < GetTotalNumChildren(RHS);
}
