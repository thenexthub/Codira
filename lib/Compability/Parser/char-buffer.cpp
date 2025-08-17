//===-- lib/Parser/char-buffer.cpp ----------------------------------------===//
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

#include "language/Compability/Parser/char-buffer.h"
#include "language/Compability/Common/idioms.h"
#include <algorithm>
#include <cstddef>
#include <cstring>

namespace language::Compability::parser {

char *CharBuffer::FreeSpace(std::size_t &n) {
  int offset{LastBlockOffset()};
  if (blocks_.empty()) {
    blocks_.emplace_front();
    lastBlockEmpty_ = true;
  } else if (offset == 0 && !lastBlockEmpty_) {
    blocks_.emplace_back();
    lastBlockEmpty_ = true;
  }
  n = Block::capacity - offset;
  return blocks_.back().data + offset;
}

void CharBuffer::Claim(std::size_t n) {
  if (n > 0) {
    bytes_ += n;
    lastBlockEmpty_ = false;
  }
}

std::size_t CharBuffer::Put(const char *data, std::size_t n) {
  std::size_t chunk;
  for (std::size_t at{0}; at < n; at += chunk) {
    char *to{FreeSpace(chunk)};
    chunk = std::min(n - at, chunk);
    Claim(chunk);
    std::memcpy(to, data + at, chunk);
  }
  return bytes_ - n;
}

std::size_t CharBuffer::Put(const std::string &str) {
  return Put(str.data(), str.size());
}

std::string CharBuffer::Marshal() const {
  std::string result;
  std::size_t bytes{bytes_};
  result.reserve(bytes);
  for (const Block &block : blocks_) {
    std::size_t chunk{std::min(bytes, Block::capacity)};
    for (std::size_t j{0}; j < chunk; ++j) {
      result += block.data[j];
    }
    bytes -= chunk;
  }
  result.shrink_to_fit();
  CHECK(result.size() == bytes_);
  return result;
}
} // namespace language::Compability::parser
