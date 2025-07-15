//===--- LocalizationProducerTests.cpp -------------------------------------===//
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

#include "LocalizationTest.h"
#include "language/Localization/LocalizationFormat.h"
#include "toolchain/ADT/SmallBitVector.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/raw_ostream.h"
#include "gtest/gtest.h"
#include <string>
#include <random>

using namespace language::diag;
using namespace language::unittests;

TEST_F(LocalizationTest, TestStringsSerialization) {
  StringsLocalizationProducer strings(DiagsPath);

  auto dbFile = createTemporaryFile("en", "db");

  // First, let's serialize English translations
  {
    SerializedLocalizationWriter writer;

    strings.forEachAvailable(
        [&writer](language::DiagID id, toolchain::StringRef translation) {
          writer.insert(id, translation);
        });

    ASSERT_FALSE(writer.emit(dbFile));
  }

  // Now, let's make sure that serialized version matches "source" YAML
  auto dbContent = toolchain::MemoryBuffer::getFile(dbFile);
  ASSERT_TRUE(dbContent);

  SerializedLocalizationProducer db(std::move(dbContent.get()));
  strings.forEachAvailable(
      [&db](language::DiagID id, toolchain::StringRef translation) {
        ASSERT_EQ(translation, db.getMessageOr(id, "<no-fallback>"));
      });
}

TEST_F(LocalizationTest, TestSerializationOfEmptyFile) {
  auto dbFile = createTemporaryFile("by", "db");
  SerializedLocalizationWriter writer;
  ASSERT_FALSE(writer.emit(dbFile));

  StringsLocalizationProducer strings(DiagsPath);

  // Reading of the empty `db` file should always return default message.
  {
    auto dbContent = toolchain::MemoryBuffer::getFile(dbFile);
    ASSERT_TRUE(dbContent);

    SerializedLocalizationProducer db(std::move(dbContent.get()));
    strings.forEachAvailable([&db](language::DiagID id,
                                   toolchain::StringRef translation) {
      ASSERT_EQ("<<<default-fallback>>>",
                db.getMessageOr(id, "<<<default-fallback>>>"));
    });
  }
}

TEST_F(LocalizationTest, TestSerializationWithGaps) {
  // Initially all of the messages are included.
  toolchain::SmallBitVector includedMessages(LocalDiagID::NumDiags, true);

  // Let's punch some holes in the diagnostic content.
  for (unsigned i = 0, n = 200; i != n; ++i) {
    unsigned position = RandNumber(LocalDiagID::NumDiags);
    includedMessages.flip(position);
  }

  StringsLocalizationProducer strings(DiagsPath);
  auto dbFile = createTemporaryFile("en", "db");

  {
    SerializedLocalizationWriter writer;

    strings.forEachAvailable(
        [&](language::DiagID id, toolchain::StringRef translation) {
          if (includedMessages.test((unsigned)id))
            writer.insert(id, translation);
        });

    ASSERT_FALSE(writer.emit(dbFile));
  }


  {
    auto dbContent = toolchain::MemoryBuffer::getFile(dbFile);
    ASSERT_TRUE(dbContent);

    SerializedLocalizationProducer db(std::move(dbContent.get()));
    strings.forEachAvailable([&](language::DiagID id,
                                 toolchain::StringRef translation) {
      auto position = (unsigned)id;

      std::string expectedMessage = includedMessages.test(position)
                                        ? std::string(translation)
                                        : "<<<default-fallback>>>";

      ASSERT_EQ(expectedMessage, db.getMessageOr(id, "<<<default-fallback>>>"));
    });
  }
}
