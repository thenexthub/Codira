//===--- CompactArray.cpp -------------------------------------------------===//
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

#include "sourcekitd/CompactArray.h"
#include "SourceKit/Core/Toolchain.h"
#include "SourceKit/Support/UIdent.h"
#include "toolchain/Support/MemoryBuffer.h"
#include <limits.h>

using namespace SourceKit;
using namespace sourcekitd;

template <typename T>
static void addScalar(T Val, SmallVectorImpl<uint8_t> &Buf) {
  const uint8_t *ValPtr = reinterpret_cast<const uint8_t *>(&Val);
  Buf.append(ValPtr, ValPtr + sizeof(Val));
}

CompactArrayBuilderImpl::CompactArrayBuilderImpl() {
  // Offset 0 is used for empty strings.
  StringBuffer += '\0';
}

void CompactArrayBuilderImpl::addImpl(uint8_t Val) {
  addScalar(Val, EntriesBuffer);
}

void CompactArrayBuilderImpl::addImpl(unsigned Val) {
  addScalar(Val, EntriesBuffer);
}

void CompactArrayBuilderImpl::addImpl(StringRef Val) {
  addScalar(getOffsetForString(Val), EntriesBuffer);
}

void CompactArrayBuilderImpl::addImpl(UIdent Val) {
  if (Val.isValid()) {
    sourcekitd_uid_t uid = SKDUIDFromUIdent(Val);
    addScalar(uid, EntriesBuffer);
  } else {
    addScalar(sourcekitd_uid_t(nullptr), EntriesBuffer);
  }
}

void CompactArrayBuilderImpl::addImpl(std::optional<toolchain::StringRef> Val) {
  if (Val.has_value()) {
    addImpl(Val.value());
  } else {
    addImpl(unsigned(-1));
  }
}

unsigned CompactArrayBuilderImpl::getOffsetForString(StringRef Str) {
  if (Str.empty())
    return 0;

  unsigned Offset = StringBuffer.size();
  StringBuffer += Str;
  StringBuffer += '\0';
  return Offset;
}

std::unique_ptr<toolchain::MemoryBuffer>
CompactArrayBuilderImpl::createBuffer(CustomBufferKind Kind) const {
  auto bodySize = sizeInBytes();
  std::unique_ptr<toolchain::WritableMemoryBuffer> Buf;
  Buf = toolchain::WritableMemoryBuffer::getNewUninitMemBuffer(
      sizeof(uint64_t) + bodySize);
  *reinterpret_cast<uint64_t*>(Buf->getBufferStart()) = (uint64_t)Kind;
  copyInto(Buf->getBufferStart() + sizeof(uint64_t), bodySize);
  return std::move(Buf);
}

void CompactArrayBuilderImpl::appendTo(SmallVectorImpl<char> &Buf) const {
  size_t OrigSize = Buf.size();
  size_t NewSize = OrigSize + sizeInBytes();
  Buf.resize(NewSize);
  copyInto(Buf.data() + OrigSize, NewSize - OrigSize);
}

void CompactArrayBuilderImpl::copyInto(char *BufPtr, size_t Length) const {
  uint64_t EntriesBufSize = EntriesBuffer.size();
  assert(Length >= sizeInBytes());
  memcpy(BufPtr, &EntriesBufSize, sizeof(EntriesBufSize));
  BufPtr += sizeof(EntriesBufSize);
  memcpy(BufPtr, EntriesBuffer.data(), EntriesBufSize);
  BufPtr += EntriesBufSize;
  memcpy(BufPtr, StringBuffer.data(), StringBuffer.size());
}

unsigned CompactArrayBuilderImpl::copyInto(char *BufPtr) const {
  size_t Length = sizeInBytes();
  copyInto(BufPtr, Length);
  return Length;
}

bool CompactArrayBuilderImpl::empty() const {
  return EntriesBuffer.empty();
}

size_t CompactArrayBuilderImpl::sizeInBytes() const {
  return sizeof(uint64_t) + EntriesBuffer.size() + StringBuffer.size();
}

template <typename T>
static void readScalar(const uint8_t *Buf, T &Val) {
  memcpy(&Val, Buf, sizeof(Val));
}

void CompactArrayReaderImpl::readImpl(size_t Offset, uint8_t &Val) {
  readScalar(getEntriesBufStart() + Offset, Val);
}

void CompactArrayReaderImpl::readImpl(size_t Offset, unsigned &Val) {
  readScalar(getEntriesBufStart() + Offset, Val);
}

void CompactArrayReaderImpl::readImpl(size_t Offset, const char * &Val) {
  unsigned StrOffset;
  readScalar(getEntriesBufStart() + Offset, StrOffset);
  if (StrOffset == unsigned(-1)) {
    Val = nullptr;
    return;
  }
  Val = getStringBufStart() + StrOffset;
}

void CompactArrayReaderImpl::readImpl(size_t Offset, sourcekitd_uid_t &Val) {
  readScalar(getEntriesBufStart() + Offset, Val);
}
