//===--- CMemoryReader.h - MemoryReader wrapper for C impls -----*- C++ -*-===//
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
//
//  This file declares an implementation of MemoryReader that wraps the
//  C interface provided by SwiftRemoteMirror.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_REMOTE_CMEMORYREADER_H
#define SWIFT_REMOTE_CMEMORYREADER_H

#include "language/SwiftRemoteMirror/MemoryReaderInterface.h"
#include "language/Remote/MemoryReader.h"

struct MemoryReaderImpl {
  uint8_t PointerSize;

  // Opaque pointer passed to all the callback functions.
  void *reader_context;

  QueryDataLayoutFunction queryDataLayout;
  FreeBytesFunction free;
  ReadBytesFunction readBytes;
  GetStringLengthFunction getStringLength;
  GetSymbolAddressFunction getSymbolAddress;
};

namespace language {
namespace remote {

/// An implementation of MemoryReader which wraps the C interface offered
/// by SwiftRemoteMirror.
class CMemoryReader final : public MemoryReader {
  MemoryReaderImpl Impl;

  uint64_t ptrauthMask;

  uint64_t getPtrauthMask() {
    if (ptrauthMask == 0) {
      int success;
      if (Impl.PointerSize == 4) {
        uint32_t ptrauthMask32 = 0;
        success = queryDataLayout(DataLayoutQueryType::DLQ_GetPtrAuthMask,
                                  nullptr, &ptrauthMask32);
        ptrauthMask = ptrauthMask32;
      } else if (Impl.PointerSize == 8) {
        success = queryDataLayout(DataLayoutQueryType::DLQ_GetPtrAuthMask,
                                  nullptr, &ptrauthMask);
      } else {
        success = 0;
      }

      if (!success)
        ptrauthMask = ~0ull;
    }
    return ptrauthMask;
  }

  // Check to see if an address has bits outside the ptrauth mask. This suggests
  // that we're likely failing to strip a signed pointer when reading from it.
  bool hasSignatureBits(RemoteAddress address) {
    return false;
    uint64_t addressData = address.getAddressData();
    return addressData != (addressData & getPtrauthMask());
  }

public:
  CMemoryReader(MemoryReaderImpl Impl) : Impl(Impl), ptrauthMask(0) {
    assert(this->Impl.queryDataLayout && "No queryDataLayout implementation");
    assert(this->Impl.getStringLength && "No stringLength implementation");
    assert(this->Impl.readBytes && "No readBytes implementation");
  }

  bool queryDataLayout(DataLayoutQueryType type, void *inBuffer,
                       void *outBuffer) override {
    return Impl.queryDataLayout(Impl.reader_context, type, inBuffer,
                                outBuffer) != 0;
  }

  RemoteAddress getSymbolAddress(const std::string &name) override {
    auto addressData = Impl.getSymbolAddress(Impl.reader_context,
                                             name.c_str(), name.size());
    return RemoteAddress(addressData);
  }

  uint64_t getStringLength(RemoteAddress address) {
    assert(!hasSignatureBits(address));
    return Impl.getStringLength(Impl.reader_context,
                                address.getAddressData());
  }

  bool readString(RemoteAddress address, std::string &dest) override {
    assert(!hasSignatureBits(address));
    auto length = getStringLength(address);
    if (length == 0) {
      // A length of zero unfortunately might mean either that there's a zero
      // length string at the location we're trying to read, or that reading
      // failed. We can do a one-byte read to tell them apart.
      auto buf = readBytes(address, 1);
      return buf && ((const char*)buf.get())[0] == 0;
    }

    auto Buf = readBytes(address, length);
    if (!Buf)
      return false;
    
    dest = std::string(reinterpret_cast<const char *>(Buf.get()), length);
    return true;
  }

  ReadBytesResult readBytes(RemoteAddress address, uint64_t size) override {
    assert(!hasSignatureBits(address));
    void *FreeContext;
    auto Ptr = Impl.readBytes(Impl.reader_context, address.getAddressData(),
                              size, &FreeContext);

    auto Free = Impl.free;
    if (Free == nullptr)
      return ReadBytesResult(Ptr, [](const void *) {});

    auto ReaderContext = Impl.reader_context;
    auto freeLambda = [=](const void *Ptr) {
      Free(ReaderContext, Ptr, FreeContext);
    };
    return ReadBytesResult(Ptr, freeLambda);
  }
};

}
}

#endif
