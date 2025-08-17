//===- SmallVectorMemoryBuffer.h --------------------------------*- C++ -*-===//
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
//
// This file declares a wrapper class to hold the memory into which an
// object will be generated.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_EXECUTIONENGINE_OBJECTMEMORYBUFFER_H
#define LLVM_EXECUTIONENGINE_OBJECTMEMORYBUFFER_H

#include <IndexStoreDB_LLVMSupport/toolchain_ADT_SmallVector.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_MemoryBuffer.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_raw_ostream.h>

namespace toolchain {

/// SmallVector-backed MemoryBuffer instance.
///
/// This class enables efficient construction of MemoryBuffers from SmallVector
/// instances. This is useful for MCJIT and Orc, where object files are streamed
/// into SmallVectors, then inspected using ObjectFile (which takes a
/// MemoryBuffer).
class SmallVectorMemoryBuffer : public MemoryBuffer {
public:
  /// Construct an SmallVectorMemoryBuffer from the given SmallVector
  /// r-value.
  ///
  /// FIXME: It'd be nice for this to be a non-templated constructor taking a
  /// SmallVectorImpl here instead of a templated one taking a SmallVector<N>,
  /// but SmallVector's move-construction/assignment currently only take
  /// SmallVectors. If/when that is fixed we can simplify this constructor and
  /// the following one.
  SmallVectorMemoryBuffer(SmallVectorImpl<char> &&SV)
      : SV(std::move(SV)), BufferName("<in-memory object>") {
    init(this->SV.begin(), this->SV.end(), false);
  }

  /// Construct a named SmallVectorMemoryBuffer from the given
  /// SmallVector r-value and StringRef.
  SmallVectorMemoryBuffer(SmallVectorImpl<char> &&SV, StringRef Name)
      : SV(std::move(SV)), BufferName(Name) {
    init(this->SV.begin(), this->SV.end(), false);
  }

  // Key function.
  ~SmallVectorMemoryBuffer() override;

  StringRef getBufferIdentifier() const override { return BufferName; }

  BufferKind getBufferKind() const override { return MemoryBuffer_Malloc; }

private:
  SmallVector<char, 0> SV;
  std::string BufferName;
};

} // namespace toolchain

#endif
