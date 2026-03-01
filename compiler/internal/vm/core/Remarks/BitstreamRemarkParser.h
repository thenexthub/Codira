//===-- BitstreamRemarkParser.h - Parser for Bitstream remarks --*- C++/-*-===//
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
// This file provides the impementation of the Bitstream remark parser.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_REMARKS_BITSTREAM_REMARK_PARSER_H
#define LLVM_LIB_REMARKS_BITSTREAM_REMARK_PARSER_H

#include "vm/core/ADT/StringRef.h"
#include "vm/core/Bitstream/BitstreamReader.h"
#include "vm/core/Remarks/BitstreamRemarkContainer.h"
#include "vm/core/Remarks/Remark.h"
#include "vm/core/Remarks/RemarkFormat.h"
#include "vm/core/Remarks/RemarkParser.h"
#include "vm/core/Remarks/RemarkStringTable.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/FormatVariadic.h"
#include <cstdint>
#include <memory>
#include <optional>

namespace vm::core {
namespace remarks {

class BitstreamBlockParserHelperBase {
protected:
  BitstreamCursor &Stream;

  StringRef BlockName;
  unsigned BlockID;

public:
  BitstreamBlockParserHelperBase(BitstreamCursor &Stream, unsigned BlockID,
                                 StringRef BlockName)
      : Stream(Stream), BlockName(BlockName), BlockID(BlockID) {}

  template <typename... Ts> Error error(char const *Fmt, const Ts &...Vals) {
    std::string Buffer;
    raw_string_ostream OS(Buffer);
    OS << "Error while parsing " << BlockName << " block: ";
    OS << formatv(Fmt, Vals...);
    return make_error<StringError>(
        std::move(Buffer),
        std::make_error_code(std::errc::illegal_byte_sequence));
  }

  Error expectBlock();

protected:
  Error enterBlock();

  Error unknownRecord(unsigned AbbrevID);
  Error unexpectedRecord(StringRef RecordName);
  Error malformedRecord(StringRef RecordName);
  Error unexpectedBlock(unsigned Code);
};

template <typename Derived>
class BitstreamBlockParserHelper : public BitstreamBlockParserHelperBase {
protected:
  using BitstreamBlockParserHelperBase::BitstreamBlockParserHelperBase;
  Derived &derived() { return *static_cast<Derived *>(this); }

  /// Parse a record and fill in the fields in the parser.
  /// The subclass must statically override this method.
  Error parseRecord(unsigned Code) = delete;

  /// Parse a subblock and fill in the fields in the parser.
  /// The subclass can statically override this method.
  Error parseSubBlock(unsigned Code) { return unexpectedBlock(Code); }

public:
  /// Enter, parse, and leave this bitstream block. This expects the
  /// BitstreamCursor to be right after the SubBlock entry (i.e. after calling
  /// expectBlock).
  Error parseBlock() {
    if (Error E = enterBlock())
      return E;

    // Stop when there is nothing to read anymore or when we encounter an
    // END_BLOCK.
    while (true) {
      Expected<BitstreamEntry> Next = Stream.advance();
      if (!Next)
        return Next.takeError();
      switch (Next->Kind) {
      case BitstreamEntry::SubBlock:
        if (Error E = derived().parseSubBlock(Next->ID))
          return E;
        continue;
      case BitstreamEntry::EndBlock:
        return Error::success();
      case BitstreamEntry::Record:
        if (Error E = derived().parseRecord(Next->ID))
          return E;
        continue;
      case BitstreamEntry::Error:
        return error("Unexpected end of bitstream.");
      }
      llvm_unreachable("Unexpected BitstreamEntry");
    }
  }
};

/// Helper to parse a META_BLOCK for a bitstream remark container.
class BitstreamMetaParserHelper
    : public BitstreamBlockParserHelper<BitstreamMetaParserHelper> {
  friend class BitstreamBlockParserHelper<BitstreamMetaParserHelper>;

public:
  struct ContainerInfo {
    uint64_t Version;
    uint64_t Type;
  };

  /// The parsed content: depending on the container type, some fields might
  /// be empty.
  std::optional<ContainerInfo> Container;
  std::optional<uint64_t> RemarkVersion;
  std::optional<StringRef> ExternalFilePath;
  std::optional<StringRef> StrTabBuf;

  BitstreamMetaParserHelper(BitstreamCursor &Stream)
      : BitstreamBlockParserHelper(Stream, META_BLOCK_ID, MetaBlockName) {}

protected:
  Error parseRecord(unsigned Code);
};

/// Helper to parse a REMARK_BLOCK for a bitstream remark container.
class BitstreamRemarkParserHelper
    : public BitstreamBlockParserHelper<BitstreamRemarkParserHelper> {
  friend class BitstreamBlockParserHelper<BitstreamRemarkParserHelper>;

protected:
  SmallVector<uint64_t, 5> Record;
  StringRef RecordBlob;
  unsigned RecordID;

public:
  struct RemarkLoc {
    uint64_t SourceFileNameIdx;
    uint64_t SourceLine;
    uint64_t SourceColumn;
  };

  struct Argument {
    std::optional<uint64_t> KeyIdx;
    std::optional<uint64_t> ValueIdx;
    std::optional<RemarkLoc> Loc;

    Argument(std::optional<uint64_t> KeyIdx, std::optional<uint64_t> ValueIdx)
        : KeyIdx(KeyIdx), ValueIdx(ValueIdx) {}
  };

  /// The parsed content: depending on the remark, some fields might be empty.
  std::optional<uint8_t> Type;
  std::optional<uint64_t> RemarkNameIdx;
  std::optional<uint64_t> PassNameIdx;
  std::optional<uint64_t> FunctionNameIdx;
  std::optional<uint64_t> Hotness;
  std::optional<RemarkLoc> Loc;

  SmallVector<Argument, 8> Args;

  BitstreamRemarkParserHelper(BitstreamCursor &Stream)
      : BitstreamBlockParserHelper(Stream, REMARK_BLOCK_ID, RemarkBlockName) {}

  /// Clear helper state and parse next remark block.
  Error parseNext();

protected:
  Error parseRecord(unsigned Code);
  Error handleRecord();
};

/// Helper to parse any bitstream remark container.
struct BitstreamParserHelper {
  /// The Bitstream reader.
  BitstreamCursor Stream;
  /// The block info block.
  BitstreamBlockInfo BlockInfo;

  /// Helper to parse the metadata blocks in this bitstream.
  BitstreamMetaParserHelper MetaHelper;
  /// Helper to parse the remark blocks in this bitstream. Only needed
  /// for ContainerType RemarksFile.
  std::optional<BitstreamRemarkParserHelper> RemarksHelper;
  /// The position of the first remark block we encounter after
  /// the initial metadata block.
  std::optional<uint64_t> RemarkStartBitPos;

  /// Start parsing at \p Buffer.
  BitstreamParserHelper(StringRef Buffer)
      : Stream(Buffer), MetaHelper(Stream), RemarksHelper(Stream) {}

  /// Parse and validate the magic number.
  Error expectMagic();
  /// Parse the block info block containing all the abbrevs.
  /// This needs to be called before calling any other parsing function.
  Error parseBlockInfoBlock();

  /// Parse all metadata blocks in the file. This populates the meta helper.
  Error parseMeta();
  /// Parse the next remark. This populates the remark helper data.
  Error parseRemark();
};

/// Parses and holds the state of the latest parsed remark.
struct BitstreamRemarkParser : public RemarkParser {
  /// The buffer to parse.
  std::optional<BitstreamParserHelper> ParserHelper;
  /// The string table used for parsing strings.
  std::optional<ParsedStringTable> StrTab;
  /// Temporary remark buffer used when the remarks are stored separately.
  std::unique_ptr<MemoryBuffer> TmpRemarkBuffer;
  /// Whether the metadata has already been parsed, so we can continue parsing
  /// remarks.
  bool IsMetaReady = false;
  /// The common metadata used to decide how to parse the buffer.
  /// This is filled when parsing the metadata block.
  uint64_t ContainerVersion = 0;
  uint64_t RemarkVersion = 0;
  BitstreamRemarkContainerType ContainerType =
      BitstreamRemarkContainerType::RemarksFile;

  /// Create a parser that expects to find a string table embedded in the
  /// stream.
  explicit BitstreamRemarkParser(StringRef Buf);

  Expected<std::unique_ptr<Remark>> next() override;

  static bool classof(const RemarkParser *P) {
    return P->ParserFormat == Format::Bitstream;
  }

  /// Parse and process the metadata of the buffer.
  Error parseMeta();

private:
  Error processCommonMeta();
  Error processFileContainerMeta();
  Error processExternalFilePath();

  Expected<std::unique_ptr<Remark>> processRemark();

  Error processStrTab();
  Error processRemarkVersion();
};

Expected<std::unique_ptr<BitstreamRemarkParser>> createBitstreamParserFromMeta(
    StringRef Buf,
    std::optional<StringRef> ExternalFilePrependPath = std::nullopt);

} // end namespace remarks
} // end namespace vm::core

#endif /* LLVM_LIB_REMARKS_BITSTREAM_REMARK_PARSER_H */
