//===- RemarkParser.cpp --------------------------------------------------===//
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
// This file provides utility methods used by clients that want to use the
// parser for remark diagnostics in LLVM.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Remarks/RemarkParser.h"
#include "BitstreamRemarkParser.h"
#include "YAMLRemarkParser.h"
#include "vm/core-c/Remarks.h"
#include "vm/core/Remarks/RemarkFormat.h"
#include "vm/core/Support/CBindingWrapping.h"
#include <optional>

using namespace vm::core;
using namespace vm::core::remarks;

char EndOfFileError::ID = 0;

ParsedStringTable::ParsedStringTable(StringRef InBuffer) : Buffer(InBuffer) {
  while (!InBuffer.empty()) {
    // Strings are separated by '\0' bytes.
    std::pair<StringRef, StringRef> Split = InBuffer.split('\0');
    // We only store the offset from the beginning of the buffer.
    Offsets.push_back(Split.first.data() - Buffer.data());
    InBuffer = Split.second;
  }
}

Expected<StringRef> ParsedStringTable::operator[](size_t Index) const {
  if (Index >= Offsets.size())
    return createStringError(
        std::make_error_code(std::errc::invalid_argument),
        "String with index %u is out of bounds (size = %u).", Index,
        Offsets.size());

  size_t Offset = Offsets[Index];
  // If it's the last offset, we can't use the next offset to know the size of
  // the string.
  size_t NextOffset =
      (Index == Offsets.size() - 1) ? Buffer.size() : Offsets[Index + 1];
  return StringRef(Buffer.data() + Offset, NextOffset - Offset - 1);
}

Expected<std::unique_ptr<RemarkParser>>
toolchain::remarks::createRemarkParser(Format ParserFormat, StringRef Buf) {
  auto DetectedFormat = detectFormat(ParserFormat, Buf);
  if (!DetectedFormat)
    return DetectedFormat.takeError();

  switch (*DetectedFormat) {
  case Format::YAML:
    return std::make_unique<YAMLRemarkParser>(Buf);
  case Format::Bitstream:
    return std::make_unique<BitstreamRemarkParser>(Buf);
  case Format::Unknown:
  case Format::Auto:
    break;
  }
  llvm_unreachable("unhandled ParseFormat");
}

Expected<std::unique_ptr<RemarkParser>>
toolchain::remarks::createRemarkParserFromMeta(
    Format ParserFormat, StringRef Buf,
    std::optional<StringRef> ExternalFilePrependPath) {
  auto DetectedFormat = detectFormat(ParserFormat, Buf);
  if (!DetectedFormat)
    return DetectedFormat.takeError();

  switch (*DetectedFormat) {
  case Format::YAML:
    return createYAMLParserFromMeta(Buf, std::move(ExternalFilePrependPath));
  case Format::Bitstream:
    return createBitstreamParserFromMeta(Buf,
                                         std::move(ExternalFilePrependPath));
  case Format::Unknown:
  case Format::Auto:
    break;
  }
  llvm_unreachable("unhandled ParseFormat");
}

namespace {
// Wrapper that holds the state needed to interact with the C API.
struct CParser {
  std::unique_ptr<RemarkParser> TheParser;
  std::optional<std::string> Err;

  CParser(Format ParserFormat, StringRef Buf)
      : TheParser(cantFail(createRemarkParser(ParserFormat, Buf))) {}

  void handleError(Error E) { Err.emplace(toString(std::move(E))); }
  bool hasError() const { return Err.has_value(); }
  const char *getMessage() const { return Err ? Err->c_str() : nullptr; };
};
} // namespace

// Create wrappers for C Binding types (see CBindingWrapping.h).
DEFINE_SIMPLE_CONVERSION_FUNCTIONS(CParser, LLVMRemarkParserRef)

extern "C" LLVMRemarkParserRef LLVMRemarkParserCreateYAML(const void *Buf,
                                                          uint64_t Size) {
  return wrap(new CParser(Format::YAML,
                          StringRef(static_cast<const char *>(Buf), Size)));
}

extern "C" LLVMRemarkParserRef LLVMRemarkParserCreateBitstream(const void *Buf,
                                                               uint64_t Size) {
  return wrap(new CParser(Format::Bitstream,
                          StringRef(static_cast<const char *>(Buf), Size)));
}

extern "C" LLVMRemarkEntryRef
LLVMRemarkParserGetNext(LLVMRemarkParserRef Parser) {
  CParser &TheCParser = *unwrap(Parser);
  remarks::RemarkParser &TheParser = *TheCParser.TheParser;

  Expected<std::unique_ptr<Remark>> MaybeRemark = TheParser.next();
  if (Error E = MaybeRemark.takeError()) {
    if (E.isA<EndOfFileError>()) {
      consumeError(std::move(E));
      return nullptr;
    }

    // Handle the error. Allow it to be checked through HasError and
    // GetErrorMessage.
    TheCParser.handleError(std::move(E));
    return nullptr;
  }

  // Valid remark.
  return wrap(MaybeRemark->release());
}

extern "C" LLVMBool LLVMRemarkParserHasError(LLVMRemarkParserRef Parser) {
  return unwrap(Parser)->hasError();
}

extern "C" const char *
LLVMRemarkParserGetErrorMessage(LLVMRemarkParserRef Parser) {
  return unwrap(Parser)->getMessage();
}

extern "C" void LLVMRemarkParserDispose(LLVMRemarkParserRef Parser) {
  delete unwrap(Parser);
}
