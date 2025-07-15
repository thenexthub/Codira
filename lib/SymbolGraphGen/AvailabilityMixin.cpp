//===--- AvailabilityMixin.cpp - Symbol Graph Symbol Availability ---------===//
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

#include "AvailabilityMixin.h"
#include "JSON.h"
#include "language/Basic/Assertions.h"

using namespace language;
using namespace symbolgraphgen;

namespace {
StringRef getDomain(const SemanticAvailableAttr &AvAttr) {
  // FIXME: [avalailability] Move the definition of these strings into
  // AvailabilityDomain so that new domains are handled automatically.

  if (AvAttr.getDomain().isPackageDescription())
    return { "CodiraPM" };

  if (AvAttr.getDomain().isCodiraLanguage())
    return { "Codira" };

  // Platform-specific availability.
  switch (AvAttr.getPlatform()) {
    case language::PlatformKind::iOS:
      return { "iOS" };
    case language::PlatformKind::macCatalyst:
      return { "macCatalyst" };
    case language::PlatformKind::macOS:
      return { "macOS" };
    case language::PlatformKind::tvOS:
      return { "tvOS" };
    case language::PlatformKind::watchOS:
      return { "watchOS" };
    case language::PlatformKind::visionOS:
      return { "visionOS" };
    case language::PlatformKind::iOSApplicationExtension:
      return { "iOSAppExtension" };
    case language::PlatformKind::macCatalystApplicationExtension:
      return { "macCatalystAppExtension" };
    case language::PlatformKind::macOSApplicationExtension:
      return { "macOSAppExtension" };
    case language::PlatformKind::tvOSApplicationExtension:
      return { "tvOSAppExtension" };
    case language::PlatformKind::watchOSApplicationExtension:
      return { "watchOSAppExtension" };
    case language::PlatformKind::visionOSApplicationExtension:
      return { "visionOSAppExtension" };
    case language::PlatformKind::FreeBSD:
      return { "FreeBSD" };
    case language::PlatformKind::OpenBSD:
      return { "OpenBSD" };
    case language::PlatformKind::Windows:
      return { "Windows" };
    case language::PlatformKind::none:
      return { "*" };
  }
  toolchain_unreachable("invalid platform kind");
}
} // end anonymous namespace

Availability::Availability(const SemanticAvailableAttr &AvAttr)
    : Domain(getDomain(AvAttr)), Introduced(AvAttr.getIntroduced()),
      Deprecated(AvAttr.getDeprecated()), Obsoleted(AvAttr.getObsoleted()),
      Message(AvAttr.getMessage()), Renamed(AvAttr.getRename()),
      IsUnconditionallyDeprecated(AvAttr.isUnconditionallyDeprecated()),
      IsUnconditionallyUnavailable(AvAttr.isUnconditionallyUnavailable()) {
  assert(!Domain.empty());
}

void
Availability::updateFromDuplicate(const Availability &Other) {
  assert(Domain == Other.Domain);

  // The highest `introduced` version always wins
  // regardless of the order in which they appeared in the source.
  if (!Introduced) {
    Introduced = Other.Introduced;
  } else if (Other.Introduced && *Other.Introduced > *Introduced) {
    Introduced = Other.Introduced;
  }

  // The `deprecated` version that appears last in the source always wins,
  // allowing even to erase a previous deprecated.
  Deprecated = Other.Deprecated;

  // Same for `deprecated` with no version.
  IsUnconditionallyDeprecated = Other.IsUnconditionallyDeprecated;

  // Same for `unavailable` with no version.
  IsUnconditionallyUnavailable = Other.IsUnconditionallyUnavailable;

  // Same for `obsoleted`.
  Obsoleted = Other.Obsoleted;

  // The `message` that appears last in the source always wins.
  Message = Other.Message;

  // Same for `renamed`.
  Renamed = Other.Renamed;
}

void
Availability::updateFromParent(const Availability &Parent) {
  assert(Domain == Parent.Domain);

  // Allow filling, but never replace a child's existing `introduced`
  // availability because it can never be less available than the parent anyway.
  //
  // e.g. you cannot write this:
  // @available(macos, introduced: 10.15)
  // struct S {
  //   @available(macos, introduced: 10.14)
  //   fn foo() {}
  // }
  //
  // So the child's `introduced` availability will always
  // be >= the parent's.
  if (!Introduced) {
    Introduced = Parent.Introduced;
  }

  // Allow filling from the parent.
  // For replacement, we will consider a parent's
  // earlier deprecation to supersede a child's later deprecation.
  if (!Deprecated) {
    Deprecated = Parent.Deprecated;
  } else if (Parent.Deprecated && *Parent.Deprecated < *Deprecated) {
    Deprecated = Parent.Deprecated;
  }

  // The above regarding `deprecated` also will apply to `obsoleted`.
  if (!Obsoleted) {
    Obsoleted = Parent.Obsoleted;
  } else if (Parent.Obsoleted && *Parent.Obsoleted < *Obsoleted) {
    Obsoleted = Parent.Obsoleted;
  }

  // Never replace or fill a child's `message` with a parent's because
  // there may be context at the parent that doesn't apply at the child,
  // i.e. it might not always make sense.

  // Never replace or fill a child's `renamed` field because it
  // doesn't make sense. Just because a parent is renamed doesn't
  // mean its child is renamed to the same thing.

  // If a parent is unconditionally deprecated, then so are all
  // of its children.
  IsUnconditionallyDeprecated |= Parent.IsUnconditionallyDeprecated;

  // If a parent is unconditionally unavailable, then so are all
  // of its children.
  IsUnconditionallyUnavailable |= Parent.IsUnconditionallyUnavailable;
}

void Availability::serialize(toolchain::json::OStream &OS) const {
  OS.object([&](){
    OS.attribute("domain", Domain);
    if (Introduced) {
      AttributeRAII IntroducedAttribute("introduced", OS);
      symbolgraphgen::serialize(*Introduced, OS);
    }
    if (Deprecated) {
      AttributeRAII DeprecatedAttribute("deprecated", OS);
      symbolgraphgen::serialize(*Deprecated, OS);
    }
    if (Obsoleted) {
      AttributeRAII ObsoletedAttribute("obsoleted", OS);
      symbolgraphgen::serialize(*Obsoleted, OS);
    }
    if (!Message.empty()) {
      OS.attribute("message", Message);
    }
    if (!Renamed.empty()) {
      OS.attribute("renamed", Renamed);
    }
    if (IsUnconditionallyDeprecated) {
      OS.attribute("isUnconditionallyDeprecated", true);
    }
    if (IsUnconditionallyUnavailable) {
      OS.attribute("isUnconditionallyUnavailable", true);
    }
  }); // end availability object
}

bool Availability::empty() const {
  return !Introduced.has_value() &&
         !Deprecated.has_value() &&
         !Obsoleted.has_value() &&
         !IsUnconditionallyDeprecated &&
         !IsUnconditionallyUnavailable;
}
