//===- Availability.cpp --------------------------------------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
// This file implements the Availability information for Decls.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/Availability.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Attr.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/Basic/TargetInfo.h"

namespace {

/// Represents the availability of a symbol across platforms.
struct AvailabilitySet {
  bool UnconditionallyDeprecated = false;
  bool UnconditionallyUnavailable = false;

  void insert(language::Core::AvailabilityInfo &&Availability) {
    auto *Found = getForPlatform(Availability.Domain);
    if (Found)
      Found->mergeWith(std::move(Availability));
    else
      Availabilities.emplace_back(std::move(Availability));
  }

  language::Core::AvailabilityInfo *getForPlatform(toolchain::StringRef Domain) {
    auto *It = toolchain::find_if(Availabilities,
                             [Domain](const language::Core::AvailabilityInfo &Info) {
                               return Domain.compare(Info.Domain) == 0;
                             });
    return It == Availabilities.end() ? nullptr : It;
  }

private:
  toolchain::SmallVector<language::Core::AvailabilityInfo> Availabilities;
};

static void createInfoForDecl(const language::Core::Decl *Decl,
                              AvailabilitySet &Availabilities) {
  // Collect availability attributes from all redeclarations.
  for (const auto *RD : Decl->redecls()) {
    for (const auto *A : RD->specific_attrs<language::Core::AvailabilityAttr>()) {
      Availabilities.insert(language::Core::AvailabilityInfo(
          A->getPlatform()->getName(), A->getIntroduced(), A->getDeprecated(),
          A->getObsoleted(), A->getUnavailable(), false, false));
    }

    if (const auto *A = RD->getAttr<language::Core::UnavailableAttr>())
      if (!A->isImplicit())
        Availabilities.UnconditionallyUnavailable = true;

    if (const auto *A = RD->getAttr<language::Core::DeprecatedAttr>())
      if (!A->isImplicit())
        Availabilities.UnconditionallyDeprecated = true;
  }
}

} // namespace

namespace language::Core {

void AvailabilityInfo::mergeWith(AvailabilityInfo Other) {
  if (isDefault() && Other.isDefault())
    return;

  if (Domain.empty())
    Domain = Other.Domain;

  UnconditionallyUnavailable |= Other.UnconditionallyUnavailable;
  UnconditionallyDeprecated |= Other.UnconditionallyDeprecated;
  Unavailable |= Other.Unavailable;

  Introduced = std::max(Introduced, Other.Introduced);

  // Default VersionTuple is 0.0.0 so if both are non default let's pick the
  // smallest version number, otherwise select the one that is non-zero if there
  // is one.
  if (!Deprecated.empty() && !Other.Deprecated.empty())
    Deprecated = std::min(Deprecated, Other.Deprecated);
  else
    Deprecated = std::max(Deprecated, Other.Deprecated);

  if (!Obsoleted.empty() && !Other.Obsoleted.empty())
    Obsoleted = std::min(Obsoleted, Other.Obsoleted);
  else
    Obsoleted = std::max(Obsoleted, Other.Obsoleted);
}

AvailabilityInfo AvailabilityInfo::createFromDecl(const Decl *D) {
  AvailabilitySet Availabilities;
  // Walk DeclContexts upwards starting from D to find the combined availability
  // of the symbol.
  for (const auto *Ctx = D; Ctx;
       Ctx = toolchain::cast_or_null<Decl>(Ctx->getDeclContext()))
    createInfoForDecl(Ctx, Availabilities);

  if (auto *Avail = Availabilities.getForPlatform(
          D->getASTContext().getTargetInfo().getPlatformName())) {
    Avail->UnconditionallyDeprecated = Availabilities.UnconditionallyDeprecated;
    Avail->UnconditionallyUnavailable =
        Availabilities.UnconditionallyUnavailable;
    return std::move(*Avail);
  }

  AvailabilityInfo Avail;
  Avail.UnconditionallyDeprecated = Availabilities.UnconditionallyDeprecated;
  Avail.UnconditionallyUnavailable = Availabilities.UnconditionallyUnavailable;
  return Avail;
}

} // namespace language::Core
