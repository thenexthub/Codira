//===- ScriptLexer.h --------------------------------------------*- C++ -*-===//
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

#ifndef LLD_ELF_SCRIPT_LEXER_H
#define LLD_ELF_SCRIPT_LEXER_H

#include "lld/Common/LLVM.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBufferRef.h"
#include <vector>

namespace lld::elf {
struct Ctx;

class ScriptLexer {
protected:
  struct Buffer {
    // The remaining content to parse and the filename.
    StringRef s, filename;
    const char *begin = nullptr;
    size_t lineNumber = 1;
    // True if the script is opened as an absolute path under the --sysroot
    // directory.
    bool isUnderSysroot = false;

    Buffer() = default;
    Buffer(Ctx &ctx, MemoryBufferRef mb);
  };
  Ctx &ctx;
  // The current buffer and parent buffers due to INCLUDE.
  Buffer curBuf;
  SmallVector<Buffer, 0> buffers;

  // Used to detect INCLUDE() cycles.
  llvm::DenseSet<StringRef> activeFilenames;

  enum class State {
    Script,
    Expr,
    // Used by version node and dynamic list parsing.
    VersionNode,
  };

  struct Token {
    StringRef str;
    explicit operator bool() const { return !str.empty(); }
    operator StringRef() const { return str; }
  };

  // The token before the last next().
  StringRef prevTok;
  // Rules for what is a token are different when we are in an expression.
  // curTok holds the cached return value of peek() and is invalid when the
  // expression state changes.
  StringRef curTok;
  size_t prevTokLine = 1;
  // The lex state when curTok is cached.
  State curTokState = State::Script;
  State lexState = State::Script;
  bool eof = false;

public:
  explicit ScriptLexer(Ctx &ctx, MemoryBufferRef mb);

  void setError(const Twine &msg);
  void lex();
  StringRef skipSpace(StringRef s);
  bool atEOF();
  StringRef next();
  StringRef peek();
  void skip();
  bool consume(StringRef tok);
  void expect(StringRef expect);
  Token till(StringRef tok);
  std::string getCurrentLocation();
  MemoryBufferRef getCurrentMB();

  std::vector<MemoryBufferRef> mbs;

private:
  StringRef getLine();
  size_t getColumnNumber();
};

} // namespace lld::elf

#endif
