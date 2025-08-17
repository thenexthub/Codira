//===-- language/Compability/Parser/char-buffer.h ----------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_PARSER_CHAR_BUFFER_H_
#define LANGUAGE_COMPABILITY_PARSER_CHAR_BUFFER_H_

// Defines a simple expandable buffer suitable for efficiently accumulating
// a stream of bytes.

#include <cstddef>
#include <list>
#include <string>
#include <utility>
#include <vector>

namespace language::Compability::parser {

class CharBuffer {
public:
  CharBuffer() {}
  CharBuffer(CharBuffer &&that)
      : blocks_(std::move(that.blocks_)), bytes_{that.bytes_},
        lastBlockEmpty_{that.lastBlockEmpty_} {
    that.clear();
  }
  CharBuffer &operator=(CharBuffer &&that) {
    blocks_ = std::move(that.blocks_);
    bytes_ = that.bytes_;
    lastBlockEmpty_ = that.lastBlockEmpty_;
    that.clear();
    return *this;
  }

  bool empty() const { return bytes_ == 0; }
  std::size_t bytes() const { return bytes_; }

  void clear() {
    blocks_.clear();
    bytes_ = 0;
    lastBlockEmpty_ = false;
  }

  char *FreeSpace(std::size_t &);
  void Claim(std::size_t);

  // The return value is the byte offset of the new data,
  // i.e. the value of size() before the call.
  std::size_t Put(const char *data, std::size_t n);
  std::size_t Put(const std::string &);
  std::size_t Put(char x) { return Put(&x, 1); }

  std::string Marshal() const;

private:
  struct Block {
    static constexpr std::size_t capacity{1 << 20};
    char data[capacity];
  };

  int LastBlockOffset() const { return bytes_ % Block::capacity; }
  std::list<Block> blocks_;
  std::size_t bytes_{0};
  bool lastBlockEmpty_{false};
};
} // namespace language::Compability::parser
#endif // FORTRAN_PARSER_CHAR_BUFFER_H_
