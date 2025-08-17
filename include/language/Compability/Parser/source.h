//===-- language/Compability/Parser/source.h ---------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_PARSER_SOURCE_H_
#define LANGUAGE_COMPABILITY_PARSER_SOURCE_H_

// Source file content is lightly normalized when the file is read.
//  - Line ending markers are converted to single newline characters
//  - A newline character is added to the last line of the file if one is needed
//  - A Unicode byte order mark is recognized if present.

#include "characters.h"
#include "language/Compability/Common/reference.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/raw_ostream.h"
#include <cstddef>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace toolchain {
class raw_ostream;
}

namespace language::Compability::parser {

std::string DirectoryName(std::string path);
std::optional<std::string> LocateSourceFile(
    std::string name, const std::list<std::string> &searchPath);
std::vector<std::string> LocateSourceFileAll(
    std::string name, const std::vector<std::string> &searchPath);

class SourceFile;

struct SourcePosition {
  common::Reference<const SourceFile> sourceFile;
  common::Reference<const std::string>
      path; // may not be sourceFile.path() when #line present
  int line, column;
  int trueLineNumber;
};

class SourceFile {
public:
  explicit SourceFile(Encoding e) : encoding_{e} {}
  ~SourceFile();
  const std::string &path() const { return path_; }
  toolchain::ArrayRef<char> content() const {
    return buf_->getBuffer().slice(bom_end_, buf_end_ - bom_end_);
  }
  std::size_t bytes() const { return content().size(); }
  std::size_t lines() const { return lineStart_.size(); }
  Encoding encoding() const { return encoding_; }

  bool Open(std::string path, toolchain::raw_ostream &error);
  bool ReadStandardInput(toolchain::raw_ostream &error);
  void Close();
  SourcePosition GetSourcePosition(std::size_t) const;
  std::size_t GetLineStartOffset(int lineNumber) const {
    return lineStart_.at(lineNumber - 1);
  }
  const std::string &SavePath(std::string &&);
  void LineDirective(int trueLineNumber, const std::string &, int);
  toolchain::raw_ostream &Dump(toolchain::raw_ostream &) const;

private:
  struct SourcePositionOrigin {
    const std::string &path;
    int line;
  };

  void ReadFile();
  void IdentifyPayload();
  void RecordLineStarts();

  std::string path_;
  std::unique_ptr<toolchain::WritableMemoryBuffer> buf_;
  std::vector<std::size_t> lineStart_;
  std::size_t bom_end_{0};
  std::size_t buf_end_;
  Encoding encoding_;
  std::set<std::string> distinctPaths_;
  std::map<std::size_t, SourcePositionOrigin> origins_;
};
} // namespace language::Compability::parser
#endif // FORTRAN_PARSER_SOURCE_H_
