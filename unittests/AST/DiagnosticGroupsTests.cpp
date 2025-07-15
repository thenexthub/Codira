//===--- DiagnosticGroupsTests.cpp ----------------------------------------===//
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
// NB: The correctness of the diagnostic group graph is verified in lib/AST
// ('namespace validation').
//
//===----------------------------------------------------------------------===//

#include "language/AST/DiagnosticGroups.h"
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

struct TestDiagnostic : public Diagnostic {
  TestDiagnostic(DiagID ID, DiagGroupID GroupID) : Diagnostic(ID, GroupID) {}
};

static void testCase(toolchain::function_ref<void(DiagnosticEngine &)> diagnose,
                     toolchain::function_ref<void(const DiagnosticInfo &)> callback,
                     unsigned expectedNumCallbackCalls) {
  SourceManager sourceMgr;
  DiagnosticEngine diags(sourceMgr);

  unsigned count = 0;

  const auto countingCallback = [&](const DiagnosticInfo &info) {
    ++count;
    callback(info);
  };

  TestDiagnosticConsumer consumer(countingCallback);
  diags.addConsumer(consumer);
  diagnose(diags);
  diags.removeConsumer(consumer);

  EXPECT_EQ(count, expectedNumCallbackCalls);
}

TEST(DiagnosticGroups, TargetAll) {
  // Test that uncategorized diagnostics are escalated when escalating all
  // warnings.
  testCase(
      [](DiagnosticEngine &diags) {
        const std::vector rules = {
            WarningAsErrorRule(WarningAsErrorRule::Action::Enable)};
        diags.setWarningsAsErrorsRules(rules);

        TestDiagnostic diagnostic(
            diag::warn_unsupported_module_interface_library_evolution.ID,
            DiagGroupID::no_group);
        diags.diagnose(SourceLoc(), diagnostic);
      },
      [](const DiagnosticInfo &info) {
        EXPECT_EQ(info.Kind, DiagnosticKind::Error);
      },
      /*expectedNumCallbackCalls=*/1);
}

TEST(DiagnosticGroups, OverrideBehaviorLimitations) {
  // Test that escalating warnings to errors for *errors* in a diagnostic group
  // overrides emission site behavior limitations.
  {
    TestDiagnostic diagnostic(diag::error_immediate_mode_missing_stdlib.ID,
                              DiagGroupID::DeprecatedDeclaration);

    // Make sure ID actually is an error by default.
    testCase(
        [&diagnostic](DiagnosticEngine &diags) {
          diags.diagnose(SourceLoc(), diagnostic);
        },
        [](const DiagnosticInfo &info) {
          EXPECT_EQ(info.Kind, DiagnosticKind::Error);
        },
        /*expectedNumCallbackCalls=*/1);

    testCase(
        [&diagnostic](DiagnosticEngine &diags) {
          const std::vector rules = {WarningAsErrorRule(
              WarningAsErrorRule::Action::Enable, "DeprecatedDeclaration")};
          diags.setWarningsAsErrorsRules(rules);

          diags.diagnose(SourceLoc(), diagnostic);
          diags.diagnose(SourceLoc(), diagnostic)
              .limitBehaviorUntilCodiraVersion(DiagnosticBehavior::Warning, 99);
        },
        [](const DiagnosticInfo &info) {
          EXPECT_EQ(info.Kind, DiagnosticKind::Error);
        },
        /*expectedNumCallbackCalls=*/2);
  }

  // Test that escalating warnings to errors for *warnings* in a diagnostic
  // group overrides emission site behavior limitations.
  {
    TestDiagnostic diagnostic(
        diag::warn_unsupported_module_interface_library_evolution.ID,
        DiagGroupID::DeprecatedDeclaration);

    // Make sure ID actually is a warning by default.
    testCase(
        [&diagnostic](DiagnosticEngine &diags) {
          diags.diagnose(SourceLoc(), diagnostic);
        },
        [](const DiagnosticInfo &info) {
          EXPECT_EQ(info.Kind, DiagnosticKind::Warning);
        },
        /*expectedNumCallbackCalls=*/1);

    testCase(
        [&diagnostic](DiagnosticEngine &diags) {
          const std::vector rules = {WarningAsErrorRule(
              WarningAsErrorRule::Action::Enable, "DeprecatedDeclaration")};
          diags.setWarningsAsErrorsRules(rules);

          diags.diagnose(SourceLoc(), diagnostic)
              .limitBehaviorUntilCodiraVersion(DiagnosticBehavior::Warning, 99);
        },
        [](const DiagnosticInfo &info) {
          EXPECT_EQ(info.Kind, DiagnosticKind::Error);
        },
        /*expectedNumCallbackCalls=*/1);
  }
}

} // end anonymous namespace
