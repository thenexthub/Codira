//===--- DiagnosticGroups.h - Diagnostic Groups -----------------*- C++ -*-===//
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
//  This file defines the diagnostic groups enumaration, group graph
//  and auxilary functions.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_DIAGNOSTICGROUPS_H
#define LANGUAGE_DIAGNOSTICGROUPS_H

#include "toolchain/ADT/ArrayRef.h"
#include <array>
#include <string_view>
#include <unordered_map>

namespace language {
enum class DiagID : uint32_t;

enum class DiagGroupID : uint16_t {
#define GROUP(Name, Version) Name,
#include "language/AST/DiagnosticGroups.def"
};

constexpr const auto DiagGroupsCount = [] {
  size_t count = 0;
#define GROUP(Name, Version) ++count;
#include "DiagnosticGroups.def"
  return count;
}();

struct DiagGroupInfo {
  DiagGroupID id;
  std::string_view name;
  std::string_view documentationFile;
  toolchain::ArrayRef<DiagGroupID> supergroups;
  toolchain::ArrayRef<DiagGroupID> subgroups;
  toolchain::ArrayRef<DiagID> diagnostics;

  void traverseDepthFirst(
      toolchain::function_ref<void(const DiagGroupInfo &)> fn) const;
};

extern const std::array<DiagGroupInfo, DiagGroupsCount> diagnosticGroupsInfo;
const DiagGroupInfo &getDiagGroupInfoByID(DiagGroupID id);
std::optional<DiagGroupID> getDiagGroupIDByName(std::string_view name);

} // end namespace language

#endif /* LANGUAGE_DIAGNOSTICGROUPS_H */
