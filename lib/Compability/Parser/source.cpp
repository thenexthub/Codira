//===-- lib/Parser/source.cpp ---------------------------------------------===//
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

#include "language/Compability/Parser/source.h"
#include "language/Compability/Common/idioms.h"
#include "language/Compability/Parser/char-buffer.h"
#include "language/Compability/Parser/characters.h"
#include "toolchain/Support/Errno.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/raw_ostream.h"
#include <algorithm>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace language::Compability::parser {

SourceFile::~SourceFile() { Close(); }

void SourceFile::RecordLineStarts() {
  if (std::size_t chars{bytes()}; chars > 0) {
    origins_.emplace(1, SourcePositionOrigin{path_, 1});
    const char *source{content().data()};
    CHECK(source[chars - 1] == '\n' && "missing ultimate newline");
    std::size_t at{0};
    do { // "at" is always at the beginning of a source line
      lineStart_.push_back(at);
      at = reinterpret_cast<const char *>(
               std::memchr(source + at, '\n', chars - at)) -
          source + 1;
    } while (at < chars);
    CHECK(at == chars);
    lineStart_.shrink_to_fit();
  }
}

// Check for a Unicode byte order mark (BOM).
// Module files all have one; so can source files.
void SourceFile::IdentifyPayload() {
  toolchain::StringRef content{buf_->getBufferStart(), buf_->getBufferSize()};
  constexpr toolchain::StringLiteral UTF8_BOM{"\xef\xbb\xbf"};
  if (content.starts_with(UTF8_BOM)) {
    bom_end_ = UTF8_BOM.size();
    encoding_ = Encoding::UTF_8;
  }
}

std::string DirectoryName(std::string path) {
  toolchain::SmallString<128> pathBuf{path};
  toolchain::sys::path::remove_filename(pathBuf);
  return pathBuf.str().str();
}

std::optional<std::string> LocateSourceFile(
    std::string name, const std::list<std::string> &searchPath) {
  if (name == "-" || toolchain::sys::path::is_absolute(name)) {
    return name;
  }
  for (const std::string &dir : searchPath) {
    toolchain::SmallString<128> path{dir};
    toolchain::sys::path::append(path, name);
    bool isDir{false};
    auto er = toolchain::sys::fs::is_directory(path, isDir);
    if (!er && !isDir) {
      return path.str().str();
    }
  }
  return std::nullopt;
}

std::vector<std::string> LocateSourceFileAll(
    std::string name, const std::vector<std::string> &searchPath) {
  if (name == "-" || toolchain::sys::path::is_absolute(name)) {
    return {name};
  }
  std::vector<std::string> result;
  for (const std::string &dir : searchPath) {
    toolchain::SmallString<128> path{dir};
    toolchain::sys::path::append(path, name);
    bool isDir{false};
    auto er = toolchain::sys::fs::is_directory(path, isDir);
    if (!er && !isDir) {
      result.emplace_back(path.str().str());
    }
  }
  return result;
}

std::size_t RemoveCarriageReturns(toolchain::MutableArrayRef<char> buf) {
  std::size_t wrote{0};
  char *buffer{buf.data()};
  char *p{buf.data()};
  std::size_t bytes = buf.size();
  while (bytes > 0) {
    void *vp{static_cast<void *>(p)};
    void *crvp{std::memchr(vp, '\r', bytes)};
    char *crcp{static_cast<char *>(crvp)};
    if (!crcp) {
      std::memmove(buffer + wrote, p, bytes);
      wrote += bytes;
      break;
    }
    std::size_t chunk = crcp - p;
    auto advance{chunk + 1};
    if (chunk + 1 >= bytes || crcp[1] == '\n') {
      // CR followed by LF or EOF: omit
    } else if ((chunk == 0 && p == buf.data()) || crcp[-1] == '\n') {
      // CR preceded by LF or BOF: omit
    } else {
      // CR in line: retain
      ++chunk;
    }
    std::memmove(buffer + wrote, p, chunk);
    wrote += chunk;
    p += advance;
    bytes -= advance;
  }
  return wrote;
}

bool SourceFile::Open(std::string path, toolchain::raw_ostream &error) {
  Close();
  path_ = path;
  std::string errorPath{"'"s + path_ + "'"};
  auto bufOr{toolchain::WritableMemoryBuffer::getFile(path)};
  if (!bufOr) {
    auto err = bufOr.getError();
    error << "Could not open " << errorPath << ": " << err.message();
    return false;
  }
  buf_ = std::move(bufOr.get());
  ReadFile();
  return true;
}

bool SourceFile::ReadStandardInput(toolchain::raw_ostream &error) {
  Close();
  path_ = "standard input";
  auto buf_or = toolchain::MemoryBuffer::getSTDIN();
  if (!buf_or) {
    auto err = buf_or.getError();
    error << err.message();
    return false;
  }
  auto inbuf = std::move(buf_or.get());
  buf_ =
      toolchain::WritableMemoryBuffer::getNewUninitMemBuffer(inbuf->getBufferSize());
  toolchain::copy(inbuf->getBuffer(), buf_->getBufferStart());
  ReadFile();
  return true;
}

void SourceFile::ReadFile() {
  buf_end_ = RemoveCarriageReturns(buf_->getBuffer());
  if (content().size() == 0 || content().back() != '\n') {
    // Don't bother to copy if we have spare memory
    if (content().size() >= buf_->getBufferSize()) {
      auto tmp_buf{toolchain::WritableMemoryBuffer::getNewUninitMemBuffer(
          content().size() + 1)};
      toolchain::copy(content(), tmp_buf->getBufferStart());
      buf_ = std::move(tmp_buf);
    }
    buf_end_++;
    buf_->getBuffer()[buf_end_ - 1] = '\n';
  }
  IdentifyPayload();
  RecordLineStarts();
}

void SourceFile::Close() {
  path_.clear();
  buf_.reset();
  distinctPaths_.clear();
  origins_.clear();
}

SourcePosition SourceFile::GetSourcePosition(std::size_t at) const {
  CHECK(at < bytes());
  auto it{toolchain::upper_bound(lineStart_, at)};
  auto trueLineNumber{std::distance(lineStart_.begin(), it - 1) + 1};
  auto ub{origins_.upper_bound(trueLineNumber)};
  auto column{static_cast<int>(at - lineStart_[trueLineNumber - 1] + 1)};
  if (ub == origins_.begin()) {
    return {*this, path_, static_cast<int>(trueLineNumber), column,
        static_cast<int>(trueLineNumber)};
  } else {
    --ub;
    const SourcePositionOrigin &origin{ub->second};
    auto lineNumber{
        trueLineNumber - ub->first + static_cast<std::size_t>(origin.line)};
    return {*this, origin.path, static_cast<int>(lineNumber), column,
        static_cast<int>(trueLineNumber)};
  }
}

const std::string &SourceFile::SavePath(std::string &&path) {
  return *distinctPaths_.emplace(std::move(path)).first;
}

void SourceFile::LineDirective(
    int trueLineNumber, const std::string &path, int lineNumber) {
  origins_.emplace(trueLineNumber, SourcePositionOrigin{path, lineNumber});
}

toolchain::raw_ostream &SourceFile::Dump(toolchain::raw_ostream &o) const {
  o << "SourceFile '" << path_ << "'\n";
  for (const auto &[at, spo] : origins_) {
    o << "  origin_[" << at << "] -> '" << spo.path << "' " << spo.line << '\n';
  }
  return o;
}
} // namespace language::Compability::parser
