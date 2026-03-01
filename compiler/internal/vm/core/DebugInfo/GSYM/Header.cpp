//===- Header.cpp -----------------------------------------------*- C++ -*-===//
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

#include "vm/core/DebugInfo/GSYM/Header.h"
#include "vm/core/DebugInfo/GSYM/FileWriter.h"
#include "vm/core/Support/DataExtractor.h"
#include "vm/core/Support/Format.h"
#include "vm/core/Support/raw_ostream.h"

#define HEX8(v) toolchain::format_hex(v, 4)
#define HEX16(v) toolchain::format_hex(v, 6)
#define HEX32(v) toolchain::format_hex(v, 10)
#define HEX64(v) toolchain::format_hex(v, 18)

using namespace vm::core;
using namespace gsym;

raw_ostream &toolchain::gsym::operator<<(raw_ostream &OS, const Header &H) {
  OS << "Header:\n";
  OS << "  Magic        = " << HEX32(H.Magic) << "\n";
  OS << "  Version      = " << HEX16(H.Version) << '\n';
  OS << "  AddrOffSize  = " << HEX8(H.AddrOffSize) << '\n';
  OS << "  UUIDSize     = " << HEX8(H.UUIDSize) << '\n';
  OS << "  BaseAddress  = " << HEX64(H.BaseAddress) << '\n';
  OS << "  NumAddresses = " << HEX32(H.NumAddresses) << '\n';
  OS << "  StrtabOffset = " << HEX32(H.StrtabOffset) << '\n';
  OS << "  StrtabSize   = " << HEX32(H.StrtabSize) << '\n';
  OS << "  UUID         = ";
  for (uint8_t I = 0; I < H.UUIDSize; ++I)
    OS << format_hex_no_prefix(H.UUID[I], 2);
  OS << '\n';
  return OS;
}

/// Check the header and detect any errors.
toolchain::Error Header::checkForError() const {
  if (Magic != GSYM_MAGIC)
    return createStringError(std::errc::invalid_argument,
                             "invalid GSYM magic 0x%8.8x", Magic);
  if (Version != GSYM_VERSION)
    return createStringError(std::errc::invalid_argument,
                             "unsupported GSYM version %u", Version);
  switch (AddrOffSize) {
    case 1: break;
    case 2: break;
    case 4: break;
    case 8: break;
    default:
        return createStringError(std::errc::invalid_argument,
                                 "invalid address offset size %u",
                                 AddrOffSize);
  }
  if (UUIDSize > GSYM_MAX_UUID_SIZE)
    return createStringError(std::errc::invalid_argument,
                             "invalid UUID size %u", UUIDSize);
  return Error::success();
}

toolchain::Expected<Header> Header::decode(DataExtractor &Data) {
  uint64_t Offset = 0;
  // The header is stored as a single blob of data that has a fixed byte size.
  if (!Data.isValidOffsetForDataOfSize(Offset, sizeof(Header)))
    return createStringError(std::errc::invalid_argument,
                             "not enough data for a gsym::Header");
  Header H;
  H.Magic = Data.getU32(&Offset);
  H.Version = Data.getU16(&Offset);
  H.AddrOffSize = Data.getU8(&Offset);
  H.UUIDSize = Data.getU8(&Offset);
  H.BaseAddress = Data.getU64(&Offset);
  H.NumAddresses = Data.getU32(&Offset);
  H.StrtabOffset = Data.getU32(&Offset);
  H.StrtabSize = Data.getU32(&Offset);
  Data.getU8(&Offset, H.UUID, GSYM_MAX_UUID_SIZE);
  if (toolchain::Error Err = H.checkForError())
    return std::move(Err);
  return H;
}

toolchain::Error Header::encode(FileWriter &O) const {
  // Users must verify the Header is valid prior to calling this funtion.
  if (toolchain::Error Err = checkForError())
    return Err;
  O.writeU32(Magic);
  O.writeU16(Version);
  O.writeU8(AddrOffSize);
  O.writeU8(UUIDSize);
  O.writeU64(BaseAddress);
  O.writeU32(NumAddresses);
  O.writeU32(StrtabOffset);
  O.writeU32(StrtabSize);
  O.writeData(toolchain::ArrayRef<uint8_t>(UUID));
  return Error::success();
}

bool toolchain::gsym::operator==(const Header &LHS, const Header &RHS) {
  return LHS.Magic == RHS.Magic && LHS.Version == RHS.Version &&
      LHS.AddrOffSize == RHS.AddrOffSize && LHS.UUIDSize == RHS.UUIDSize &&
      LHS.BaseAddress == RHS.BaseAddress &&
      LHS.NumAddresses == RHS.NumAddresses &&
      LHS.StrtabOffset == RHS.StrtabOffset &&
      LHS.StrtabSize == RHS.StrtabSize &&
      memcmp(LHS.UUID, RHS.UUID, LHS.UUIDSize) == 0;
}
