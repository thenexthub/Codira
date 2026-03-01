//===------ InProcessMemoryAccess.cpp - Direct, in-process mem access -----===//
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

#include "vm/core/ExecutionEngine/Orc/InProcessMemoryAccess.h"

#define DEBUG_TYPE "orc"

namespace vm::core::orc {

MemoryAccess::~MemoryAccess() = default;

void InProcessMemoryAccess::writeUInt8sAsync(ArrayRef<tpctypes::UInt8Write> Ws,
                                             WriteResultFn OnWriteComplete) {
  for (auto &W : Ws)
    *W.Addr.toPtr<uint8_t *>() = W.Value;
  OnWriteComplete(Error::success());
}

void InProcessMemoryAccess::writeUInt16sAsync(
    ArrayRef<tpctypes::UInt16Write> Ws, WriteResultFn OnWriteComplete) {
  for (auto &W : Ws)
    *W.Addr.toPtr<uint16_t *>() = W.Value;
  OnWriteComplete(Error::success());
}

void InProcessMemoryAccess::writeUInt32sAsync(
    ArrayRef<tpctypes::UInt32Write> Ws, WriteResultFn OnWriteComplete) {
  for (auto &W : Ws)
    *W.Addr.toPtr<uint32_t *>() = W.Value;
  OnWriteComplete(Error::success());
}

void InProcessMemoryAccess::writeUInt64sAsync(
    ArrayRef<tpctypes::UInt64Write> Ws, WriteResultFn OnWriteComplete) {
  for (auto &W : Ws)
    *W.Addr.toPtr<uint64_t *>() = W.Value;
  OnWriteComplete(Error::success());
}

void InProcessMemoryAccess::writePointersAsync(
    ArrayRef<tpctypes::PointerWrite> Ws, WriteResultFn OnWriteComplete) {
  if (IsArch64Bit) {
    for (auto &W : Ws)
      *W.Addr.toPtr<uint64_t *>() = W.Value.getValue();
  } else {
    for (auto &W : Ws)
      *W.Addr.toPtr<uint32_t *>() = static_cast<uint32_t>(W.Value.getValue());
  }

  OnWriteComplete(Error::success());
}

void InProcessMemoryAccess::writeBuffersAsync(
    ArrayRef<tpctypes::BufferWrite> Ws, WriteResultFn OnWriteComplete) {
  for (auto &W : Ws)
    memcpy(W.Addr.toPtr<char *>(), W.Buffer.data(), W.Buffer.size());
  OnWriteComplete(Error::success());
}

void InProcessMemoryAccess::readUInt8sAsync(
    ArrayRef<ExecutorAddr> Rs, OnReadUIntsCompleteFn<uint8_t> OnComplete) {
  ReadUIntsResult<uint8_t> Result;
  Result.reserve(Rs.size());
  for (auto &R : Rs)
    Result.push_back(*R.toPtr<uint8_t *>());
  OnComplete(std::move(Result));
}

void InProcessMemoryAccess::readUInt16sAsync(
    ArrayRef<ExecutorAddr> Rs, OnReadUIntsCompleteFn<uint16_t> OnComplete) {
  ReadUIntsResult<uint16_t> Result;
  Result.reserve(Rs.size());
  for (auto &R : Rs)
    Result.push_back(*R.toPtr<uint16_t *>());
  OnComplete(std::move(Result));
}

void InProcessMemoryAccess::readUInt32sAsync(
    ArrayRef<ExecutorAddr> Rs, OnReadUIntsCompleteFn<uint32_t> OnComplete) {
  ReadUIntsResult<uint32_t> Result;
  Result.reserve(Rs.size());
  for (auto &R : Rs)
    Result.push_back(*R.toPtr<uint32_t *>());
  OnComplete(std::move(Result));
}

void InProcessMemoryAccess::readUInt64sAsync(
    ArrayRef<ExecutorAddr> Rs, OnReadUIntsCompleteFn<uint64_t> OnComplete) {
  ReadUIntsResult<uint64_t> Result;
  Result.reserve(Rs.size());
  for (auto &R : Rs)
    Result.push_back(*R.toPtr<uint64_t *>());
  OnComplete(std::move(Result));
}

void InProcessMemoryAccess::readPointersAsync(
    ArrayRef<ExecutorAddr> Rs, OnReadPointersCompleteFn OnComplete) {
  ReadPointersResult Result;
  Result.reserve(Rs.size());
  for (auto &R : Rs)
    Result.push_back(ExecutorAddr::fromPtr(*R.toPtr<void **>()));
  OnComplete(std::move(Result));
}

void InProcessMemoryAccess::readBuffersAsync(
    ArrayRef<ExecutorAddrRange> Rs, OnReadBuffersCompleteFn OnComplete) {
  ReadBuffersResult Result;
  Result.reserve(Rs.size());
  for (auto &R : Rs) {
    Result.push_back({});
    Result.back().resize(R.size());
    memcpy(Result.back().data(), R.Start.toPtr<char *>(), R.size());
  }
  OnComplete(std::move(Result));
}

void InProcessMemoryAccess::readStringsAsync(
    ArrayRef<ExecutorAddr> Rs, OnReadStringsCompleteFn OnComplete) {
  ReadStringsResult Result;
  Result.reserve(Rs.size());
  for (auto &R : Rs) {
    Result.push_back({});
    for (auto *P = R.toPtr<char *>(); *P; ++P)
      Result.back().push_back(*P);
  }
  OnComplete(std::move(Result));
}

} // end namespace vm::core::orc
