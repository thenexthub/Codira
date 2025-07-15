//===--- LocalizationTest.h - Helper for setting up locale tests -*- C++-*-===//
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

#ifndef LOCALIZATION_TEST_H
#define LOCALIZATION_TEST_H

#include "language/Localization/LocalizationFormat.h"
#include "language/Basic/Compiler.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Signals.h"
#include "toolchain/Support/raw_ostream.h"
#include "gtest/gtest.h"
#include <random>
#include <string>
#include <system_error>

using namespace language::diag;

namespace language {
namespace unittests {

enum LocalDiagID : uint32_t {
#define DIAG(KIND, ID, Group, Options, Text, Signature) ID,
#include "language/AST/DiagnosticsAll.def"
  NumDiags
};

static constexpr const char *const diagnosticID[] = {
#define DIAG(KIND, ID, Group, Options, Text, Signature) #ID,
#include "language/AST/DiagnosticsAll.def"
};

static constexpr const char *const diagnosticMessages[] = {
#define DIAG(KIND, ID, Group, Options, Text, Signature) Text,
#include "language/AST/DiagnosticsAll.def"
};

struct LocalizationTest : public ::testing::Test {
  toolchain::SmallVector<std::string, 4> TempFiles;

public:
  std::string DiagsPath;

  LocalizationTest() {
    DiagsPath = std::string(createTemporaryFile("en", "strings"));
  }

  void SetUp() override {
    bool failed = convertDefIntoStrings(DiagsPath);
    assert(!failed && "failed to generate a `.strings` file");
  }

  void TearDown() override {
    for (auto &tmp : TempFiles)
      toolchain::sys::fs::remove(tmp);
  }

  std::string createTemporaryFile(std::string prefix, std::string suffix) {
    toolchain::SmallString<128> tempFile;
    std::error_code error =
        toolchain::sys::fs::createTemporaryFile(prefix, suffix, tempFile);
    assert(!error);
    // Can't use toolchain::sys::RemoveFileOnSignal(tempFile) because
    // signals are not available on Windows.
    auto tmp = std::string(tempFile);
    TempFiles.push_back(tmp);
    return tmp;
  }

  /// Random number in [0,n)
  unsigned RandNumber(unsigned n) { return unsigned(rand()) % n; }

protected:
  static bool convertDefIntoStrings(std::string outputPath) {
    std::error_code error;
    toolchain::raw_fd_ostream OS(outputPath, error, toolchain::sys::fs::OF_None);
    if (OS.has_error() || error)
      return true;

    toolchain::ArrayRef<const char *> ids(diagnosticID, LocalDiagID::NumDiags);
    toolchain::ArrayRef<const char *> messages(diagnosticMessages,
                                          LocalDiagID::NumDiags);

    DefToStringsConverter converter(ids, messages);
    converter.convert(OS);

    OS.flush();

    return OS.has_error();
  }
};

} // end namespace unittests
} // end namespace language

#endif
