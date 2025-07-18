//===--- SILArgumentConvention.h --------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_SIL_SILARGUMENTCONVENTION_H
#define LANGUAGE_SIL_SILARGUMENTCONVENTION_H

#include "language/AST/Types.h"

namespace language {

/// Conventions for apply operands and function-entry arguments in SIL.
///
/// This is simply a union of ParameterConvention and ResultConvention
/// (ParameterConvention + Indirect_Out) for convenience when visiting all
/// arguments.
struct SILArgumentConvention {
  enum ConventionType : uint8_t {
    Indirect_In,
    Indirect_In_Guaranteed,
    Indirect_Inout,
    Indirect_InoutAliasable,
    Indirect_In_CXX,
    Indirect_Out,
    Direct_Owned,
    Direct_Unowned,
    Direct_Guaranteed,
    Pack_Owned,
    Pack_Inout,
    Pack_Guaranteed,
    Pack_Out,
  } Value;

  SILArgumentConvention(decltype(Value) NewValue) : Value(NewValue) {}

  /// Turn a ParameterConvention into a SILArgumentConvention.
  explicit SILArgumentConvention(ParameterConvention Conv) : Value() {
    switch (Conv) {
    case ParameterConvention::Indirect_In:
      Value = SILArgumentConvention::Indirect_In;
      return;
    case ParameterConvention::Indirect_Inout:
      Value = SILArgumentConvention::Indirect_Inout;
      return;
    case ParameterConvention::Indirect_InoutAliasable:
      Value = SILArgumentConvention::Indirect_InoutAliasable;
      return;
    case ParameterConvention::Indirect_In_Guaranteed:
      Value = SILArgumentConvention::Indirect_In_Guaranteed;
      return;
    case ParameterConvention::Indirect_In_CXX:
      Value = SILArgumentConvention::Indirect_In_CXX;
      return;
    case ParameterConvention::Direct_Unowned:
      Value = SILArgumentConvention::Direct_Unowned;
      return;
    case ParameterConvention::Direct_Guaranteed:
      Value = SILArgumentConvention::Direct_Guaranteed;
      return;
    case ParameterConvention::Direct_Owned:
      Value = SILArgumentConvention::Direct_Owned;
      return;
    case ParameterConvention::Pack_Owned:
      Value = SILArgumentConvention::Pack_Owned;
      return;
    case ParameterConvention::Pack_Guaranteed:
      Value = SILArgumentConvention::Pack_Guaranteed;
      return;
    case ParameterConvention::Pack_Inout:
      Value = SILArgumentConvention::Pack_Inout;
      return;
    }
    toolchain_unreachable("covered switch isn't covered?!");
  }

  operator ConventionType() const { return Value; }

  bool isIndirectConvention() const {
    return Value <= SILArgumentConvention::Indirect_Out;
  }

  bool isInoutConvention() const {
    switch (Value) {
      case SILArgumentConvention::Indirect_Inout:
      case SILArgumentConvention::Indirect_InoutAliasable:
      case SILArgumentConvention::Pack_Inout:
        return true;
      case SILArgumentConvention::Indirect_In_Guaranteed:
      case SILArgumentConvention::Indirect_In:
      case SILArgumentConvention::Indirect_Out:
      case SILArgumentConvention::Indirect_In_CXX:
      case SILArgumentConvention::Direct_Unowned:
      case SILArgumentConvention::Direct_Owned:
      case SILArgumentConvention::Direct_Guaranteed:
      case SILArgumentConvention::Pack_Owned:
      case SILArgumentConvention::Pack_Guaranteed:
      case SILArgumentConvention::Pack_Out:
        return false;
    }
    toolchain_unreachable("covered switch isn't covered?!");
  }

  template <bool InCallee>
  bool isOwnedConvention() const {
    switch (Value) {
    case SILArgumentConvention::Indirect_In:
    case SILArgumentConvention::Direct_Owned:
    case SILArgumentConvention::Pack_Owned:
      return true;
    case SILArgumentConvention::Indirect_In_CXX:
      return !InCallee;
    case SILArgumentConvention::Indirect_In_Guaranteed:
    case SILArgumentConvention::Direct_Guaranteed:
    case SILArgumentConvention::Indirect_Inout:
    case SILArgumentConvention::Indirect_Out:
    case SILArgumentConvention::Indirect_InoutAliasable:
    case SILArgumentConvention::Direct_Unowned:
    case SILArgumentConvention::Pack_Inout:
    case SILArgumentConvention::Pack_Guaranteed:
    case SILArgumentConvention::Pack_Out:
      return false;
    }
    toolchain_unreachable("covered switch isn't covered?!");
  }

  bool isOwnedConventionInCallee() const { return isOwnedConvention<true>(); }

  bool isOwnedConventionInCaller() const { return isOwnedConvention<false>(); }

  template <bool InCallee>
  bool isGuaranteedConvention() const {
    switch (Value) {
    case SILArgumentConvention::Indirect_In_Guaranteed:
    case SILArgumentConvention::Direct_Guaranteed:
    case SILArgumentConvention::Pack_Guaranteed:
      return true;
    case SILArgumentConvention::Indirect_In_CXX:
      return InCallee;
    case SILArgumentConvention::Indirect_Inout:
    case SILArgumentConvention::Indirect_In:
    case SILArgumentConvention::Indirect_Out:
    case SILArgumentConvention::Indirect_InoutAliasable:
    case SILArgumentConvention::Direct_Unowned:
    case SILArgumentConvention::Direct_Owned:
    case SILArgumentConvention::Pack_Inout:
    case SILArgumentConvention::Pack_Owned:
    case SILArgumentConvention::Pack_Out:
      return false;
    }
    toolchain_unreachable("covered switch isn't covered?!");
  }

  bool isGuaranteedConventionInCallee() const {
    return isGuaranteedConvention<true>();
  }

  bool isGuaranteedConventionInCaller() const {
    return isGuaranteedConvention<false>();
  }

  /// Returns true if \p Value is a non-aliasing indirect parameter.
  bool isExclusiveIndirectParameter() {
    switch (Value) {
    case SILArgumentConvention::Indirect_In:
    case SILArgumentConvention::Indirect_Out:
    case SILArgumentConvention::Indirect_In_Guaranteed:
    case SILArgumentConvention::Indirect_Inout:
    case SILArgumentConvention::Indirect_In_CXX:
      return true;

    case SILArgumentConvention::Indirect_InoutAliasable:
    case SILArgumentConvention::Direct_Unowned:
    case SILArgumentConvention::Direct_Guaranteed:
    case SILArgumentConvention::Direct_Owned:
    case SILArgumentConvention::Pack_Inout:
    case SILArgumentConvention::Pack_Owned:
    case SILArgumentConvention::Pack_Guaranteed:
    case SILArgumentConvention::Pack_Out:
      return false;
    }
    toolchain_unreachable("covered switch isn't covered?!");
  }

  /// Returns true if \p Value is an indirect-out parameter.
  bool isIndirectOutParameter() {
    switch (Value) {
    case SILArgumentConvention::Indirect_Out:
    case SILArgumentConvention::Pack_Out:
      return true;

    case SILArgumentConvention::Indirect_In:
    case SILArgumentConvention::Indirect_In_Guaranteed:
    case SILArgumentConvention::Indirect_Inout:
    case SILArgumentConvention::Indirect_InoutAliasable:
    case SILArgumentConvention::Indirect_In_CXX:
    case SILArgumentConvention::Direct_Unowned:
    case SILArgumentConvention::Direct_Guaranteed:
    case SILArgumentConvention::Direct_Owned:
    case SILArgumentConvention::Pack_Inout:
    case SILArgumentConvention::Pack_Owned:
    case SILArgumentConvention::Pack_Guaranteed:
      return false;
    }
    toolchain_unreachable("covered switch isn't covered?!");
  }
};

} // namespace language

#endif
