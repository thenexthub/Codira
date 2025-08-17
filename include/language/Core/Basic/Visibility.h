//===--- Visibility.h - Visibility enumeration and utilities ----*- C++ -*-===//
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
///
/// \file
/// Defines the language::Core::Visibility enumeration and various utility
/// functions.
///
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_BASIC_VISIBILITY_H
#define LANGUAGE_CORE_BASIC_VISIBILITY_H

#include "language/Core/Basic/Linkage.h"
#include "toolchain/ADT/STLForwardCompat.h"
#include <cassert>
#include <cstdint>

namespace language::Core {

/// Describes the different kinds of visibility that a declaration
/// may have.
///
/// Visibility determines how a declaration interacts with the dynamic
/// linker.  It may also affect whether the symbol can be found by runtime
/// symbol lookup APIs.
///
/// Visibility is not described in any language standard and
/// (nonetheless) sometimes has odd behavior.  Not all platforms
/// support all visibility kinds.
enum Visibility {
  /// Objects with "hidden" visibility are not seen by the dynamic
  /// linker.
  HiddenVisibility,

  /// Objects with "protected" visibility are seen by the dynamic
  /// linker but always dynamically resolve to an object within this
  /// shared object.
  ProtectedVisibility,

  /// Objects with "default" visibility are seen by the dynamic linker
  /// and act like normal objects.
  DefaultVisibility
};

inline Visibility minVisibility(Visibility L, Visibility R) {
  return L < R ? L : R;
}

class LinkageInfo {
  LLVM_PREFERRED_TYPE(Linkage)
  uint8_t linkage_    : 3;
  LLVM_PREFERRED_TYPE(Visibility)
  uint8_t visibility_ : 2;
  LLVM_PREFERRED_TYPE(bool)
  uint8_t explicit_   : 1;

  void setVisibility(Visibility V, bool E) { visibility_ = V; explicit_ = E; }
public:
  LinkageInfo()
      : linkage_(toolchain::to_underlying(Linkage::External)),
        visibility_(DefaultVisibility), explicit_(false) {}
  LinkageInfo(Linkage L, Visibility V, bool E)
      : linkage_(toolchain::to_underlying(L)), visibility_(V), explicit_(E) {
    assert(getLinkage() == L && getVisibility() == V &&
           isVisibilityExplicit() == E && "Enum truncated!");
  }

  static LinkageInfo external() {
    return LinkageInfo();
  }
  static LinkageInfo internal() {
    return LinkageInfo(Linkage::Internal, DefaultVisibility, false);
  }
  static LinkageInfo uniqueExternal() {
    return LinkageInfo(Linkage::UniqueExternal, DefaultVisibility, false);
  }
  static LinkageInfo none() {
    return LinkageInfo(Linkage::None, DefaultVisibility, false);
  }
  static LinkageInfo visible_none() {
    return LinkageInfo(Linkage::VisibleNone, DefaultVisibility, false);
  }

  Linkage getLinkage() const { return static_cast<Linkage>(linkage_); }
  Visibility getVisibility() const { return (Visibility)visibility_; }
  bool isVisibilityExplicit() const { return explicit_; }

  void setLinkage(Linkage L) { linkage_ = toolchain::to_underlying(L); }

  void mergeLinkage(Linkage L) {
    setLinkage(minLinkage(getLinkage(), L));
  }
  void mergeLinkage(LinkageInfo other) {
    mergeLinkage(other.getLinkage());
  }

  void mergeExternalVisibility(Linkage L) {
    Linkage ThisL = getLinkage();
    if (!isExternallyVisible(L)) {
      if (ThisL == Linkage::VisibleNone)
        ThisL = Linkage::None;
      else if (ThisL == Linkage::External)
        ThisL = Linkage::UniqueExternal;
    }
    setLinkage(ThisL);
  }
  void mergeExternalVisibility(LinkageInfo Other) {
    mergeExternalVisibility(Other.getLinkage());
  }

  /// Merge in the visibility 'newVis'.
  void mergeVisibility(Visibility newVis, bool newExplicit) {
    Visibility oldVis = getVisibility();

    // Never increase visibility.
    if (oldVis < newVis)
      return;

    // If the new visibility is the same as the old and the new
    // visibility isn't explicit, we have nothing to add.
    if (oldVis == newVis && !newExplicit)
      return;

    // Otherwise, we're either decreasing visibility or making our
    // existing visibility explicit.
    setVisibility(newVis, newExplicit);
  }
  void mergeVisibility(LinkageInfo other) {
    mergeVisibility(other.getVisibility(), other.isVisibilityExplicit());
  }

  /// Merge both linkage and visibility.
  void merge(LinkageInfo other) {
    mergeLinkage(other);
    mergeVisibility(other);
  }

  /// Merge linkage and conditionally merge visibility.
  void mergeMaybeWithVisibility(LinkageInfo other, bool withVis) {
    mergeLinkage(other);
    if (withVis) mergeVisibility(other);
  }
};
}

#endif // LANGUAGE_CORE_BASIC_VISIBILITY_H
