//===-- toolchain/CodeGen/ByteStreamer.h - ByteStreamer class --------*- C++ -*-===//
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
// This file contains a class that can take bytes that would normally be
// streamed via the AsmPrinter.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_CODEGEN_ASMPRINTER_BYTESTREAMER_H
#define LLVM_LIB_CODEGEN_ASMPRINTER_BYTESTREAMER_H

#include "DIEHash.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/Support/LEB128.h"
#include <string>

namespace vm::core {
class ByteStreamer {
 protected:
  ~ByteStreamer() = default;
  ByteStreamer(const ByteStreamer&) = default;
  ByteStreamer() = default;

 public:
  // For now we're just handling the calls we need for dwarf emission/hashing.
  virtual void emitInt8(uint8_t Byte, const Twine &Comment = "") = 0;
  virtual void emitSLEB128(uint64_t DWord, const Twine &Comment = "") = 0;
  virtual void emitULEB128(uint64_t DWord, const Twine &Comment = "",
                           unsigned PadTo = 0) = 0;
  virtual unsigned emitDIERef(const DIE &D) = 0;
};

class APByteStreamer final : public ByteStreamer {
private:
  AsmPrinter &AP;

public:
  APByteStreamer(AsmPrinter &Asm) : AP(Asm) {}
  void emitInt8(uint8_t Byte, const Twine &Comment) override {
    AP.OutStreamer->AddComment(Comment);
    AP.emitInt8(Byte);
  }
  void emitSLEB128(uint64_t DWord, const Twine &Comment) override {
    AP.OutStreamer->AddComment(Comment);
    AP.emitSLEB128(DWord);
  }
  void emitULEB128(uint64_t DWord, const Twine &Comment,
                   unsigned PadTo) override {
    AP.OutStreamer->AddComment(Comment);
    AP.emitULEB128(DWord, nullptr, PadTo);
  }
  unsigned emitDIERef(const DIE &D) override {
    uint64_t Offset = D.getOffset();
    static constexpr unsigned ULEB128PadSize = 4;
    assert(Offset < (1ULL << (ULEB128PadSize * 7)) && "Offset wont fit");
    emitULEB128(Offset, "", ULEB128PadSize);
    // Return how many comments to skip in DwarfDebug::emitDebugLocEntry to keep
    // comments aligned with debug loc entries.
    return ULEB128PadSize;
  }
};

class HashingByteStreamer final : public ByteStreamer {
 private:
  DIEHash &Hash;
 public:
   HashingByteStreamer(DIEHash &H) : Hash(H) {}
   void emitInt8(uint8_t Byte, const Twine &Comment) override {
     Hash.update(Byte);
  }
  void emitSLEB128(uint64_t DWord, const Twine &Comment) override {
    Hash.addSLEB128(DWord);
  }
  void emitULEB128(uint64_t DWord, const Twine &Comment,
                   unsigned PadTo) override {
    Hash.addULEB128(DWord);
  }
  unsigned emitDIERef(const DIE &D) override {
    Hash.hashRawTypeReference(D);
    return 0; // Only used together with the APByteStreamer.
  }
};

class BufferByteStreamer final : public ByteStreamer {
private:
  SmallVectorImpl<char> &Buffer;
  std::vector<std::string> &Comments;

public:
  /// Only verbose textual output needs comments.  This will be set to
  /// true for that case, and false otherwise.  If false, comments passed in to
  /// the emit methods will be ignored.
  const bool GenerateComments;

  BufferByteStreamer(SmallVectorImpl<char> &Buffer,
                     std::vector<std::string> &Comments, bool GenerateComments)
      : Buffer(Buffer), Comments(Comments), GenerateComments(GenerateComments) {
  }
  void emitInt8(uint8_t Byte, const Twine &Comment) override {
    Buffer.push_back(Byte);
    if (GenerateComments)
      Comments.push_back(Comment.str());
  }
  void emitSLEB128(uint64_t DWord, const Twine &Comment) override {
    raw_svector_ostream OSE(Buffer);
    unsigned Length = encodeSLEB128(DWord, OSE);
    if (GenerateComments) {
      Comments.push_back(Comment.str());
      // Add some empty comments to keep the Buffer and Comments vectors aligned
      // with each other.
      for (size_t i = 1; i < Length; ++i)
        Comments.push_back("");

    }
  }
  void emitULEB128(uint64_t DWord, const Twine &Comment,
                   unsigned PadTo) override {
    raw_svector_ostream OSE(Buffer);
    unsigned Length = encodeULEB128(DWord, OSE, PadTo);
    if (GenerateComments) {
      Comments.push_back(Comment.str());
      // Add some empty comments to keep the Buffer and Comments vectors aligned
      // with each other.
      for (size_t i = 1; i < Length; ++i)
        Comments.push_back("");
    }
  }
  unsigned emitDIERef(const DIE &D) override {
    uint64_t Offset = D.getOffset();
    static constexpr unsigned ULEB128PadSize = 4;
    assert(Offset < (1ULL << (ULEB128PadSize * 7)) && "Offset wont fit");
    emitULEB128(Offset, "", ULEB128PadSize);
    return 0; // Only used together with the APByteStreamer.
  }
};

}

#endif
