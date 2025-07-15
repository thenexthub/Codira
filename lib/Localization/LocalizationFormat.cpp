//===-- LocalizationFormat.cpp - Format for Diagnostic Messages -*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// This file implements the format for localized diagnostic messages.
//
//===----------------------------------------------------------------------===//

#include "language/Localization/LocalizationFormat.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Range.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Bitstream/BitstreamReader.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Path.h"
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <system_error>
#include <type_traits>

namespace {

enum LocalDiagID : uint32_t {
#define DIAG(KIND, ID, Group, Options, Text, Signature) ID,
#include "language/AST/DiagnosticsAll.def"
  NumDiags
};

} // namespace

namespace language {
namespace diag {

void SerializedLocalizationWriter::insert(language::DiagID id,
                                          toolchain::StringRef translation) {
  generator.insert(static_cast<uint32_t>(id), translation);
}

bool SerializedLocalizationWriter::emit(toolchain::StringRef filePath) {
  assert(toolchain::sys::path::extension(filePath) == ".db");
  std::error_code error;
  toolchain::raw_fd_ostream OS(filePath, error, toolchain::sys::fs::OF_None);
  if (OS.has_error()) {
    return true;
  }

  offset_type offset;
  {
    toolchain::support::endian::write<offset_type>(OS, 0, toolchain::endianness::little);
    offset = generator.Emit(OS);
  }
  OS.seek(0);
  toolchain::support::endian::write(OS, offset, toolchain::endianness::little);
  OS.close();

  return OS.has_error();
}

void LocalizationProducer::initializeIfNeeded() {
  if (state != NotInitialized)
    return;

  if (initializeImpl())
    state = Initialized;
  else
    state = FailedInitialization;
}

toolchain::StringRef
LocalizationProducer::getMessageOr(language::DiagID id,
                                   toolchain::StringRef defaultMessage) {
  initializeIfNeeded();
  if (getState() == FailedInitialization) {
    return defaultMessage;
  }

  auto localizedMessage = getMessage(id);
  if (localizedMessage.empty())
    return defaultMessage;
  return localizedMessage;
}

LocalizationProducerState LocalizationProducer::getState() const {
  return state;
}

SerializedLocalizationProducer::SerializedLocalizationProducer(
    std::unique_ptr<toolchain::MemoryBuffer> buffer)
    : LocalizationProducer(), Buffer(std::move(buffer)) {}

bool SerializedLocalizationProducer::initializeImpl() {
  auto base =
      reinterpret_cast<const unsigned char *>(Buffer.get()->getBufferStart());
  auto tableOffset =
      toolchain::support::endian::read<offset_type>(base, toolchain::endianness::little);
  SerializedTable.reset(SerializedLocalizationTable::Create(
      base + tableOffset, base + sizeof(offset_type), base));
  return true;
}

toolchain::StringRef
SerializedLocalizationProducer::getMessage(language::DiagID id) const {
  auto value = SerializedTable.get()->find(id);
  if (value.getDataLen() == 0)
    return toolchain::StringRef();
  return {(const char *)value.getDataPtr(), value.getDataLen()};
}

std::unique_ptr<LocalizationProducer>
LocalizationProducer::producerFor(toolchain::StringRef locale,
                                  toolchain::StringRef path) {
  toolchain::SmallString<128> filePath(path);
  toolchain::sys::path::append(filePath, locale);
  toolchain::sys::path::replace_extension(filePath, ".db");

  // If the serialized diagnostics file not available,
  // fallback to the `.strings` file.
  if (toolchain::sys::fs::exists(filePath)) {
    if (auto file = toolchain::MemoryBuffer::getFile(filePath)) {
      return std::make_unique<diag::SerializedLocalizationProducer>(
          std::move(file.get()));
    }
  } else {
    toolchain::sys::path::replace_extension(filePath, ".strings");
    if (toolchain::sys::fs::exists(filePath)) {
      return std::make_unique<diag::StringsLocalizationProducer>(
          filePath.str());
    }
  }

  return std::unique_ptr<LocalizationProducer>();
}

void DefToStringsConverter::convert(toolchain::raw_ostream &out) {
  // "<id>" = "<msg>";
  for (auto i : language::indices(IDs)) {
    out << "\"" << IDs[i] << "\"";
    out << " = ";

    const std::string &msg = Messages[i];

    out << "\"";
    for (unsigned j = 0; j < msg.length(); ++j) {
      // Escape '"' found in the message.
      if (msg[j] == '"')
        out << '\\';

      out << msg[j];
    }

    out << "\";\r\n";
  }
}

bool StringsLocalizationProducer::initializeImpl() {
  auto FileBufOrErr = toolchain::MemoryBuffer::getFileOrSTDIN(filePath);
  toolchain::MemoryBuffer *document = FileBufOrErr->get();
  readStringsFile(document, diagnostics);
  return true;
}

toolchain::StringRef
StringsLocalizationProducer::getMessage(language::DiagID id) const {
  return diagnostics[(unsigned)id];
}

void StringsLocalizationProducer::forEachAvailable(
    toolchain::function_ref<void(language::DiagID, toolchain::StringRef)> callback) {
  initializeIfNeeded();
  if (getState() == FailedInitialization) {
    return;
  }

  for (uint32_t i = 0, n = diagnostics.size(); i != n; ++i) {
    auto translation = diagnostics[i];
    if (!translation.empty())
      callback(static_cast<language::DiagID>(i), translation);
  }
}

void StringsLocalizationProducer::readStringsFile(
    toolchain::MemoryBuffer *in, std::vector<std::string> &diagnostics) {
  std::map<std::string, unsigned> diagLocs;
#define DIAG(KIND, ID, Group, Options, Text, Signature)                        \
  diagLocs[#ID] = static_cast<unsigned>(LocalDiagID::ID);
#include "language/AST/DiagnosticsAll.def"
#undef DIAG

  // Allocate enough slots to fit all the possible diagnostics
  // this helps to identify which diagnostics are missing.
  diagnostics.resize(LocalDiagID::NumDiags);

  // The format is as follows:
  //
  // - comment: /* ... */
  // - translation: "<id>" = "<message>";
  auto buffer = in->getBuffer();
  while (!buffer.empty()) {
    // consume comment.
    if (buffer.starts_with("/*")) {
      auto endOfComment = buffer.find("*/");
      assert(endOfComment != std::string::npos);
      // Consume the comment and trailing `*/`
      buffer = buffer.drop_front(endOfComment + 2).ltrim();
      continue;
    }

    assert(buffer.starts_with("\"") && "malformed diagnostics file");

    // Consume leading `"`
    buffer = buffer.drop_front();

    // Valid diagnostic id cannot have any `"` in it.
    auto idSize = buffer.find_first_of('\"');
    assert(idSize != std::string::npos);

    std::string id(buffer.data(), idSize);

    // consume id and `" = "`. There could be a variable number of
    // spaces on each side of `=`.
    {
      // Consume id, trailing `"`, and all spaces before `=`
      buffer = buffer.drop_front(idSize + 1).ltrim(' ');

      // Consume `=` and all trailing spaces until `"`
      {
        assert(!buffer.empty() && buffer.front() == '=');
        buffer = buffer.drop_front().ltrim(' ');
      }

      // Consume `"` at the beginning of the diagnostic message.
      {
        assert(!buffer.empty() && buffer.front() == '\"');
        buffer = buffer.drop_front();
      }
    }

    toolchain::SmallString<64> msg;
    {
      bool isValid = false;
      // Look for `";` which denotes the end of message
      for (unsigned i = 0, n = buffer.size(); i != n; ++i) {
        if (buffer[i] != '\"') {
          msg.push_back(buffer[i]);
          continue;
        }

        // Leading `"` has been comsumed.
        assert(i > 0);

        // Let's check whether this `"` is escaped, and if so - continue
        // because `"` is part of the message.
        if (buffer[i - 1] == '\\') {
          // Drop `\` added for escaping.
          msg.pop_back();
          msg.push_back(buffer[i]);
          continue;
        }

        // If current `"` was not escaped and it's followed by `;` -
        // we have reached the end of the message, otherwise
        // the input is malformed.
        if (i + 1 < n && buffer[i + 1] == ';') {
          // Consume the message and its trailing info.
          buffer = buffer.drop_front(i + 2).ltrim();
          // Mark message as valid.
          isValid = true;
          break;
        } else {
          toolchain_unreachable("malformed diagnostics file");
        }
      }

      assert(isValid && "malformed diagnostic message");
    }

    // Check whether extracted diagnostic still exists in the
    // system and if not - record as unknown.
    {
      auto existing = diagLocs.find(id);
      if (existing != diagLocs.end()) {
        diagnostics[existing->second] = std::string(msg);
      } else {
        toolchain::errs() << "[!] Unknown diagnostic: " << id << '\n';
      }
    }
  }
}

} // namespace diag
} // namespace language
