//===--- WarningAsErrorRule.h -----------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_BASIC_WARNINGASERRORRULE_H
#define LANGUAGE_BASIC_WARNINGASERRORRULE_H

#include "toolchain/Support/ErrorHandling.h"
#include <string>
#include <variant>
#include <vector>

namespace language {

/// Describes a rule how to treat a warning or all warnings.
class WarningAsErrorRule {
public:
  enum class Action { Disable, Enable };
  struct TargetAll {};
  struct TargetGroup {
    std::string name;
  };
  using Target = std::variant<TargetAll, TargetGroup>;

  /// Init as a rule targeting all diagnostic groups
  WarningAsErrorRule(Action action) : action(action), target(TargetAll()) {}
  /// Init as a rule targeting a specific diagnostic group
  WarningAsErrorRule(Action action, const std::string &group)
      : action(action), target(TargetGroup{group}) {}

  Action getAction() const { return action; }

  Target getTarget() const { return target; }

  static bool hasConflictsWithSuppressWarnings(
      const std::vector<WarningAsErrorRule> &rules) {
    bool warningsAsErrorsAllEnabled = false;
    for (const auto &rule : rules) {
      const auto target = rule.getTarget();
      if (std::holds_alternative<TargetAll>(target)) {
        // Only `-warnings-as-errors` conflicts with `-suppress-warnings`
        switch (rule.getAction()) {
        case WarningAsErrorRule::Action::Enable:
          warningsAsErrorsAllEnabled = true;
          break;
        case WarningAsErrorRule::Action::Disable:
          warningsAsErrorsAllEnabled = false;
          break;
        }
      } else if (std::holds_alternative<TargetGroup>(target)) {
        // Both `-Wwarning` and `-Werror` conflict with `-suppress-warnings`
        return true;
      } else {
        toolchain_unreachable("unhandled WarningAsErrorRule::Target");
      }
    }
    return warningsAsErrorsAllEnabled;
  }

private:
  Action action;
  Target target;
};

} // end namespace language

#endif // LANGUAGE_BASIC_WARNINGASERRORRULE_H