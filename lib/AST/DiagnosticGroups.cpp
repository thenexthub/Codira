//===--- DiagnosticGroups.cpp - Diagnostic Groups ---------------*- C++ -*-===//
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

#include "language/AST/DiagnosticGroups.h"
#include "language/AST/DiagnosticList.h"
#include <unordered_set>

namespace language {

namespace {

template <size_t SupergroupsSize, size_t SubgroupsSize, size_t DiagnosticsSize>
struct GroupConnections {
  std::array<DiagGroupID, SupergroupsSize> supergroups;
  std::array<DiagGroupID, SubgroupsSize> subgroups;
  std::array<DiagID, DiagnosticsSize> diagnostics;

  constexpr GroupConnections(
      const std::array<DiagGroupID, SupergroupsSize> &supergroups,
      const std::array<DiagGroupID, SubgroupsSize> &subgroups,
      const std::array<DiagID, DiagnosticsSize> &diagnostics)
      : supergroups(supergroups), subgroups(subgroups),
        diagnostics(diagnostics) {}
};

// This CTAD is needed in C++17 only. Remove after update to C++20.
template <size_t SupergroupsSize, size_t SubgroupsSize, size_t DiagnosticsSize>
GroupConnections(const std::array<DiagGroupID, SupergroupsSize> &,
                 const std::array<DiagGroupID, SubgroupsSize> &,
                 const std::array<DiagID, DiagnosticsSize> &)
    -> GroupConnections<SupergroupsSize, SubgroupsSize, DiagnosticsSize>;

constexpr const auto diagnosticGroupConnections = [] {
  constexpr auto sizes = [] {
    std::array<size_t, DiagGroupsCount> supergroupsCount{};
    std::array<size_t, DiagGroupsCount> subgroupsCount{};
    std::array<size_t, DiagGroupsCount> diagnosticsCount{};

    // Count edges for each diagnostic group
#define GROUP_LINK(Parent, Child)                                              \
  subgroupsCount[(size_t)DiagGroupID::Parent]++;                               \
  supergroupsCount[(size_t)DiagGroupID::Child]++;
#include "language/AST/DiagnosticGroups.def"

    // Count attached diagnostic IDs for each diagnostic group
#define DIAG(KIND, ID, Group, Options, Text, Signature)                        \
  diagnosticsCount[(size_t)DiagGroupID::Group]++;
#include "language/AST/DiagnosticsAll.def"

    return std::tuple{supergroupsCount, subgroupsCount, diagnosticsCount};
  }();
  constexpr auto supergroupsCount = std::get<0>(sizes);
  constexpr auto subgroupsCount = std::get<1>(sizes);
  constexpr auto diagnosticsCount = std::get<2>(sizes);

  // Declare all edges
#define GROUP(Name, DocsFile)                                                  \
  std::array<DiagGroupID, supergroupsCount[(size_t)DiagGroupID::Name]>         \
      Name##_supergroups{};                                                    \
  std::array<DiagGroupID, subgroupsCount[(size_t)DiagGroupID::Name]>           \
      Name##_subgroups{};                                                      \
  std::array<DiagID, diagnosticsCount[(size_t)DiagGroupID::Name]>              \
      Name##_diagnostics{};                                                    \
  [[maybe_unused]] size_t Name##_supergroupsIndex = 0;                         \
  [[maybe_unused]] size_t Name##_subgroupsIndex = 0;                           \
  [[maybe_unused]] size_t Name##_diagnosticsIndex = 0;
#include "language/AST/DiagnosticGroups.def"

  // Bind all groups to each other
#define GROUP_LINK(Parent, Child)                                              \
  Parent##_subgroups[Parent##_subgroupsIndex++] = DiagGroupID::Child;          \
  Child##_supergroups[Child##_supergroupsIndex++] = DiagGroupID::Parent;
#include "language/AST/DiagnosticGroups.def"

  // Bind all diagnostics to their groups
#define DIAG(KIND, ID, Group, Options, Text, Signature)                        \
  Group##_diagnostics[Group##_diagnosticsIndex++] = DiagID::ID;
#include "language/AST/DiagnosticsAll.def"

  // Produce the resulting structure with all the edges
#define GROUP(Name, DocsFile)                                              \
  GroupConnections(Name##_supergroups, Name##_subgroups, Name##_diagnostics),
  return std::tuple{
#include "language/AST/DiagnosticGroups.def"
  };
}();

std::unordered_map<std::string_view, DiagGroupID> nameToIDMap{
#define GROUP(Name, DocsFile) {#Name, DiagGroupID::Name},
#include "language/AST/DiagnosticGroups.def"
};

void traverseDepthFirst(DiagGroupID id,
                        std::unordered_set<DiagGroupID> &visited,
                        toolchain::function_ref<void(const DiagGroupInfo &)> fn) {
  if (visited.insert(id).second) {
    const auto &info = getDiagGroupInfoByID(id);
    fn(info);
    for (const auto subgroup : info.subgroups) {
      traverseDepthFirst(subgroup, visited, fn);
    }
  }
}

} // end anonymous namespace

constexpr const std::array<DiagGroupInfo, DiagGroupsCount> diagnosticGroupsInfo{
#define GROUP(Name, DocsFile)                                              \
  DiagGroupInfo{                                                               \
      DiagGroupID::Name,                                                       \
      #Name,                                                                   \
      DocsFile,                                                                \
      toolchain::ArrayRef<DiagGroupID>(                                             \
          std::get<(size_t)DiagGroupID::Name>(diagnosticGroupConnections)      \
              .supergroups),                                                   \
      toolchain::ArrayRef<DiagGroupID>(                                             \
          std::get<(size_t)DiagGroupID::Name>(diagnosticGroupConnections)      \
              .subgroups),                                                     \
      toolchain::ArrayRef<DiagID>(                                                  \
          std::get<(size_t)DiagGroupID::Name>(diagnosticGroupConnections)      \
              .diagnostics)},
#include "language/AST/DiagnosticGroups.def"
};

const DiagGroupInfo &getDiagGroupInfoByID(DiagGroupID id) {
  return diagnosticGroupsInfo[(size_t)id];
}

std::optional<DiagGroupID> getDiagGroupIDByName(std::string_view name) {
  auto it = nameToIDMap.find(name);
  if (it == nameToIDMap.end())
    return std::nullopt;
  return it->second;
}

void DiagGroupInfo::traverseDepthFirst(
    toolchain::function_ref<void(const DiagGroupInfo &)> fn) const {
  std::unordered_set<DiagGroupID> visited;
  ::language::traverseDepthFirst(id, visited, fn);
}

namespace validation {

template <typename F, size_t... Ints>
constexpr void unfold(F &&function, std::index_sequence<Ints...>) {
  (function(std::integral_constant<size_t, Ints>{}), ...);
}

template <size_t Size, typename F>
constexpr void constexpr_for(F &&function) {
  unfold(function, std::make_index_sequence<Size>());
}

template <size_t Group>
constexpr bool
hasCycleFromGroup(std::array<bool, DiagGroupsCount> &visited,
                  std::array<bool, DiagGroupsCount> &recursionStack) {
  if (!visited[Group]) {
    visited[Group] = true;
    recursionStack[Group] = true;
    constexpr auto subgroups =
        std::get<Group>(diagnosticGroupConnections).subgroups;
    bool isCycleFound = false;
    constexpr_for<subgroups.size()>([&](auto i) {
      constexpr auto subgroup = (size_t)subgroups[i];
      if (!visited[subgroup] &&
          hasCycleFromGroup<subgroup>(visited, recursionStack))
        isCycleFound = true;
      else if (recursionStack[subgroup])
        isCycleFound = true;
    });
    if (isCycleFound)
      return true;
  }
  recursionStack[Group] = false;
  return false;
}

constexpr bool hasCycle() {
  std::array<bool, DiagGroupsCount> recursionStack{};
  std::array<bool, DiagGroupsCount> visited{};
  bool isCycleFound = false;
  constexpr_for<DiagGroupsCount>([&](auto i) {
    if (!visited[i] && hasCycleFromGroup<i>(visited, recursionStack))
      isCycleFound = true;
  });
  return isCycleFound;
}

template <DiagGroupID Child, DiagGroupID Parent>
constexpr bool isGroupInSupergroup() {
  for (const auto group :
       std::get<(size_t)Child>(diagnosticGroupConnections).supergroups)
    if (group == Parent)
      return true;
  return false;
}
// Check for isGroupInSupergroup itself
static_assert(!isGroupInSupergroup<DiagGroupID::no_group,
                                   DiagGroupID::DeprecatedDeclaration>() &&
              "Bug in isGroupInSupergroup");

static_assert(!hasCycle(), "Diagnostic groups graph has a cycle!");
// Sanity check for the "no_group" group
static_assert((uint16_t)DiagGroupID::no_group == 0, "0 isn't no_group");
static_assert(std::get<0>(diagnosticGroupConnections).supergroups.size() == 0,
              "no_group isn't a top-level group");
static_assert(std::get<0>(diagnosticGroupConnections).subgroups.size() == 0,
              "no_group shouldn't have subgroups");
// Check groups have associated diagnostics
#define CHECK_NOT_EMPTY(Group)                                                 \
  static_assert(                                                               \
      std::get<(uint16_t)DiagGroupID::Group>(diagnosticGroupConnections)       \
              .diagnostics.size() > 0,                                         \
      "'" #Group "' group shouldn't be empty.");
CHECK_NOT_EMPTY(DeprecatedDeclaration)
CHECK_NOT_EMPTY(UnknownWarningGroup)
#undef CHECK_NOT_EMPTY

} // end namespace validation

} // end namespace language
