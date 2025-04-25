//===--- ScopedTracking.h - Utilities for scoped tracking -------*- C++ -*-===//
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
// This file defines some miscellaneous utilities that are useful when
// working with tracked values that can be saved and restored in a scoped
// fashion.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_BASIC_SCOPEDTRACKING_H
#define SWIFT_BASIC_SCOPEDTRACKING_H

namespace llvm {
template <class K, class V, class T, class A>
class ScopedHashTable;
template <class K, class V, class T, class A>
class ScopedHashTableScope;
}

namespace language {

/// Must declare a nested type scope_type which can be initialized
/// with an l-value reference to the tracker type.
template <class Tracker>
struct ScopedTrackingTraits;

template <class K, class V, class T, class A>
struct ScopedTrackingTraits<llvm::ScopedHashTable<K,V,T,A>> {
  using scope_type = llvm::ScopedHashTableScope<K,V,T,A>;
};

/// A class which stores scopes for multiple trackers.  Can be
/// initialized with a pack of l-value references to the trackers.
template <class... Trackers>
class TrackingScopes;

template <>
class TrackingScopes<> {
public:
  TrackingScopes() {}
};

template <class Tracker, class... OtherTrackers>
class TrackingScopes<Tracker, OtherTrackers...> {
  typename ScopedTrackingTraits<Tracker>::scope_type Scope;
  TrackingScopes<OtherTrackers...> OtherScopes;
public:
  TrackingScopes(Tracker &tracker, OtherTrackers &...otherTrackers)
    : Scope(tracker), OtherScopes(otherTrackers...) {}
};

} // end namespace language

#endif
