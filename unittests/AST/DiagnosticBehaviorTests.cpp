//===--- DiagnosticBehaviorTests.cpp --------------------------------------===//
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

#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/Basic/SourceManager.h"
#include "gtest/gtest.h"

using namespace language;

namespace {
class TestDiagnosticConsumer : public DiagnosticConsumer {
  toolchain::function_ref<void(const DiagnosticInfo &)> callback;

public:
  TestDiagnosticConsumer(decltype(callback) callback) : callback(callback) {}

  void handleDiagnostic(SourceManager &SM,
                        const DiagnosticInfo &Info) override {
    this->callback(Info);
  }
};

static void
testCase(toolchain::function_ref<void(DiagnosticEngine &)> diagnose,
         toolchain::function_ref<void(DiagnosticEngine &, const DiagnosticInfo &)>
             callback,
         unsigned expectedNumCallbackCalls) {
  SourceManager sourceMgr;
  DiagnosticEngine diags(sourceMgr);

  unsigned count = 0;

  const auto countingCallback = [&](const DiagnosticInfo &info) {
    ++count;
    callback(diags, info);
  };

  TestDiagnosticConsumer consumer(countingCallback);
  diags.addConsumer(consumer);
  diagnose(diags);
  diags.removeConsumer(consumer);

  EXPECT_EQ(count, expectedNumCallbackCalls);
}

TEST(DiagnosticBehavior, WarnUntilCodiraLangMode) {
  testCase(
      [](DiagnosticEngine &diags) {
        diags.setLanguageVersion(version::Version({5}));
        diags.diagnose(SourceLoc(), diag::error_immediate_mode_missing_stdlib)
            .warnUntilCodiraVersion(4);
      },
      [](DiagnosticEngine &diags, const DiagnosticInfo &info) {
        EXPECT_EQ(info.Kind, DiagnosticKind::Error);
        EXPECT_EQ(info.FormatString,
                  diags.getFormatStringForDiagnostic(
                      diag::error_immediate_mode_missing_stdlib.ID));
      },
      /*expectedNumCallbackCalls=*/1);

  testCase(
      [](DiagnosticEngine &diags) {
        diags.setLanguageVersion(version::Version({4}));
        diags.diagnose(SourceLoc(), diag::error_immediate_mode_missing_stdlib)
            .warnUntilCodiraVersion(5);
      },
      [](DiagnosticEngine &diags, const DiagnosticInfo &info) {
        EXPECT_EQ(info.Kind, DiagnosticKind::Warning);
        EXPECT_EQ(info.FormatString, diags.getFormatStringForDiagnostic(
                                         diag::error_in_language_lang_mode.ID));

        auto wrappedDiagInfo = info.FormatArgs.front().getAsDiagnostic();
        EXPECT_EQ(wrappedDiagInfo->FormatString,
                  diags.getFormatStringForDiagnostic(
                      diag::error_immediate_mode_missing_stdlib.ID));
      },
      /*expectedNumCallbackCalls=*/1);

  testCase(
      [](DiagnosticEngine &diags) {
        diags.setLanguageVersion(version::Version({4}));
        diags.diagnose(SourceLoc(), diag::error_immediate_mode_missing_stdlib)
            .warnUntilCodiraVersion(99);
      },
      [](DiagnosticEngine &diags, const DiagnosticInfo &info) {
        EXPECT_EQ(info.Kind, DiagnosticKind::Warning);
        EXPECT_EQ(info.FormatString,
                  diags.getFormatStringForDiagnostic(
                      diag::error_in_a_future_language_lang_mode.ID));

        auto wrappedDiagInfo = info.FormatArgs.front().getAsDiagnostic();
        EXPECT_EQ(wrappedDiagInfo->FormatString,
                  diags.getFormatStringForDiagnostic(
                      diag::error_immediate_mode_missing_stdlib.ID));
      },
      /*expectedNumCallbackCalls=*/1);
}

} // end anonymous namespace
