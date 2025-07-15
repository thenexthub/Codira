//===--- LocalizationFormat.h - Format for Diagnostic Messages --*- C++ -*-===//
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
// This file defines the format for localized diagnostic messages.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_LOCALIZATIONFORMAT_H
#define LANGUAGE_LOCALIZATIONFORMAT_H

#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Bitstream/BitstreamReader.h"
#include "toolchain/Support/EndianStream.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/OnDiskHashTable.h"
#include "toolchain/Support/StringSaver.h"
#include "toolchain/Support/raw_ostream.h"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace language {
enum class DiagID : uint32_t;

namespace diag {

enum LocalizationProducerState : uint8_t {
  NotInitialized,
  Initialized,
  FailedInitialization
};

class DefToStringsConverter {
  toolchain::ArrayRef<const char *> IDs;
  toolchain::ArrayRef<const char *> Messages;

public:
  DefToStringsConverter(toolchain::ArrayRef<const char *> ids,
                        toolchain::ArrayRef<const char *> messages)
    : IDs(ids), Messages(messages) {
    assert(IDs.size() == Messages.size());
  }

  void convert(toolchain::raw_ostream &out);
};

class LocalizationWriterInfo {
public:
  using key_type = uint32_t;
  using key_type_ref = const uint32_t &;
  using data_type = std::string;
  using data_type_ref = toolchain::StringRef;
  using hash_value_type = uint32_t;
  using offset_type = uint32_t;

  hash_value_type ComputeHash(key_type_ref key) { return key; }

  std::pair<offset_type, offset_type> EmitKeyDataLength(toolchain::raw_ostream &out,
                                                        key_type_ref key,
                                                        data_type_ref data) {
    offset_type dataLength = static_cast<offset_type>(data.size());
    toolchain::support::endian::write<offset_type>(out, dataLength,
                                              toolchain::endianness::little);
    // No need to write the key length; it's constant.
    return {sizeof(key_type), dataLength};
  }

  void EmitKey(toolchain::raw_ostream &out, key_type_ref key, unsigned len) {
    assert(len == sizeof(key_type));
    toolchain::support::endian::write<key_type>(out, key, toolchain::endianness::little);
  }

  void EmitData(toolchain::raw_ostream &out, key_type_ref key, data_type_ref data,
                unsigned len) {
    out << data;
  }
};

class LocalizationReaderInfo {
public:
  using internal_key_type = uint32_t;
  using external_key_type = language::DiagID;
  using data_type = toolchain::StringRef;
  using hash_value_type = uint32_t;
  using offset_type = uint32_t;

  internal_key_type GetInternalKey(external_key_type key) {
    return static_cast<internal_key_type>(key);
  }

  external_key_type GetExternalKey(internal_key_type key) {
    return static_cast<external_key_type>(key);
  }

  static bool EqualKey(internal_key_type lhs, internal_key_type rhs) {
    return lhs == rhs;
  }

  hash_value_type ComputeHash(internal_key_type key) { return key; }

  static std::pair<offset_type, offset_type>
  ReadKeyDataLength(const unsigned char *&data) {
    offset_type dataLength =
        toolchain::support::endian::readNext<offset_type, toolchain::endianness::little,
                                        toolchain::support::unaligned>(data);
    return {sizeof(uint32_t), dataLength};
  }

  internal_key_type ReadKey(const unsigned char *data, offset_type length) {
    return toolchain::support::endian::readNext<
        internal_key_type, toolchain::endianness::little, toolchain::support::unaligned>(
        data);
  }

  data_type ReadData(internal_key_type Key, const unsigned char *data,
                     offset_type length) {
    return data_type((const char *)data, length);
  }
};

class SerializedLocalizationWriter {
  using offset_type = LocalizationWriterInfo::offset_type;
  toolchain::OnDiskChainedHashTableGenerator<LocalizationWriterInfo> generator;

public:
  /// Enqueue the given diagnostic to be included in a serialized translations
  /// file.
  ///
  /// \param id The identifier associated with the given diagnostic message e.g.
  ///           'cannot_convert_argument'.
  /// \param translation The localized diagnostic message for the given
  ///                    identifier.
  void insert(language::DiagID id, toolchain::StringRef translation);

  /// Write out previously inserted diagnostic translations into the given
  /// location.
  ///
  /// \param filePath The location of the serialized diagnostics file. It's
  /// supposed to be a file with '.db' postfix.
  /// \returns true if all diagnostic
  /// messages have been successfully serialized, false otherwise.
  bool emit(toolchain::StringRef filePath);
};

class LocalizationProducer {
  LocalizationProducerState state = NotInitialized;

public:
  /// If the  message isn't available/localized in current context
  /// return the fallback default message.
  virtual toolchain::StringRef getMessageOr(language::DiagID id,
                                       toolchain::StringRef defaultMessage);

  /// \returns a `SerializedLocalizationProducer` pointer if the serialized
  /// diagnostics file available, otherwise returns a
  /// `StringsLocalizationProducer` if the `.strings` file is available. If both
  /// files aren't available returns a `nullptr`.
  static std::unique_ptr<LocalizationProducer>
  producerFor(toolchain::StringRef locale, toolchain::StringRef path);

  virtual ~LocalizationProducer() {}

protected:
  LocalizationProducerState getState() const;

  /// Used to lazily initialize `LocalizationProducer`s.
  /// \returns true if the producer is successfully initialized, false
  /// otherwise.
  virtual bool initializeImpl() = 0;
  virtual void initializeIfNeeded() final;

  /// Retrieve a message for the given diagnostic id.
  /// \returns empty string if message couldn't be found.
  virtual toolchain::StringRef getMessage(language::DiagID id) const = 0;
};

class StringsLocalizationProducer final : public LocalizationProducer {
  std::string filePath;

  std::vector<std::string> diagnostics;

public:
  explicit StringsLocalizationProducer(toolchain::StringRef filePath)
      : LocalizationProducer(), filePath(filePath) {}

  /// Iterate over all of the available (non-empty) translations
  /// maintained by this producer, callback gets each translation
  /// with its unique identifier.
  void forEachAvailable(
      toolchain::function_ref<void(language::DiagID, toolchain::StringRef)> callback);

protected:
  bool initializeImpl() override;
  toolchain::StringRef getMessage(language::DiagID id) const override;

private:
  static void readStringsFile(toolchain::MemoryBuffer *in,
                              std::vector<std::string> &diagnostics);
};

class SerializedLocalizationProducer final : public LocalizationProducer {
  using SerializedLocalizationTable =
      toolchain::OnDiskIterableChainedHashTable<LocalizationReaderInfo>;
  using offset_type = LocalizationReaderInfo::offset_type;
  std::unique_ptr<toolchain::MemoryBuffer> Buffer;
  std::unique_ptr<SerializedLocalizationTable> SerializedTable;

public:
  explicit SerializedLocalizationProducer(
      std::unique_ptr<toolchain::MemoryBuffer> buffer);

protected:
  bool initializeImpl() override;
  toolchain::StringRef getMessage(language::DiagID id) const override;
};

} // namespace diag
} // namespace language

#endif
