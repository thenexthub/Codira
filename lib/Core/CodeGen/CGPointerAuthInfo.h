//===----- CGPointerAuthInfo.h -  -------------------------------*- C++ -*-===//
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
// Pointer auth info class.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CODEGEN_CGPOINTERAUTHINFO_H
#define LANGUAGE_CORE_LIB_CODEGEN_CGPOINTERAUTHINFO_H

#include "language/Core/AST/Type.h"
#include "language/Core/Basic/LangOptions.h"
#include "toolchain/IR/Type.h"
#include "toolchain/IR/Value.h"

namespace language::Core {
namespace CodeGen {

class CGPointerAuthInfo {
private:
  PointerAuthenticationMode AuthenticationMode : 2;
  unsigned IsIsaPointer : 1;
  unsigned AuthenticatesNullValues : 1;
  unsigned Key : 2;
  toolchain::Value *Discriminator;

public:
  CGPointerAuthInfo()
      : AuthenticationMode(PointerAuthenticationMode::None),
        IsIsaPointer(false), AuthenticatesNullValues(false), Key(0),
        Discriminator(nullptr) {}
  CGPointerAuthInfo(unsigned Key, PointerAuthenticationMode AuthenticationMode,
                    bool IsIsaPointer, bool AuthenticatesNullValues,
                    toolchain::Value *Discriminator)
      : AuthenticationMode(AuthenticationMode), IsIsaPointer(IsIsaPointer),
        AuthenticatesNullValues(AuthenticatesNullValues), Key(Key),
        Discriminator(Discriminator) {
    assert(!Discriminator || Discriminator->getType()->isIntegerTy() ||
           Discriminator->getType()->isPointerTy());
  }

  explicit operator bool() const { return isSigned(); }

  bool isSigned() const {
    return AuthenticationMode != PointerAuthenticationMode::None;
  }

  unsigned getKey() const {
    assert(isSigned());
    return Key;
  }
  toolchain::Value *getDiscriminator() const {
    assert(isSigned());
    return Discriminator;
  }

  PointerAuthenticationMode getAuthenticationMode() const {
    return AuthenticationMode;
  }

  bool isIsaPointer() const { return IsIsaPointer; }

  bool authenticatesNullValues() const { return AuthenticatesNullValues; }

  bool shouldStrip() const {
    return AuthenticationMode == PointerAuthenticationMode::Strip ||
           AuthenticationMode == PointerAuthenticationMode::SignAndStrip;
  }

  bool shouldSign() const {
    return AuthenticationMode == PointerAuthenticationMode::SignAndStrip ||
           AuthenticationMode == PointerAuthenticationMode::SignAndAuth;
  }

  bool shouldAuth() const {
    return AuthenticationMode == PointerAuthenticationMode::SignAndAuth;
  }

  friend bool operator!=(const CGPointerAuthInfo &LHS,
                         const CGPointerAuthInfo &RHS) {
    return LHS.Key != RHS.Key || LHS.Discriminator != RHS.Discriminator ||
           LHS.AuthenticationMode != RHS.AuthenticationMode;
  }

  friend bool operator==(const CGPointerAuthInfo &LHS,
                         const CGPointerAuthInfo &RHS) {
    return !(LHS != RHS);
  }
};

} // end namespace CodeGen
} // end namespace language::Core

#endif
