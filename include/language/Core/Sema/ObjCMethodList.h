//===--- ObjCMethodList.h - A singly linked list of methods -----*- C++ -*-===//
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
// This file defines ObjCMethodList, a singly-linked list of methods.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_OBJCMETHODLIST_H
#define LANGUAGE_CORE_SEMA_OBJCMETHODLIST_H

#include "language/Core/AST/DeclObjC.h"
#include "toolchain/ADT/PointerIntPair.h"

namespace language::Core {

class ObjCMethodDecl;

/// a linked list of methods with the same selector name but different
/// signatures.
struct ObjCMethodList {
  // NOTE: If you add any members to this struct, make sure to serialize them.
  /// If there is more than one decl with this signature.
  toolchain::PointerIntPair<ObjCMethodDecl *, 1> MethodAndHasMoreThanOneDecl;
  /// The next list object and 2 bits for extra info.
  toolchain::PointerIntPair<ObjCMethodList *, 2> NextAndExtraBits;

  ObjCMethodList() { }
  ObjCMethodList(ObjCMethodDecl *M)
      : MethodAndHasMoreThanOneDecl(M, 0) {}
  ObjCMethodList(const ObjCMethodList &L)
      : MethodAndHasMoreThanOneDecl(L.MethodAndHasMoreThanOneDecl),
        NextAndExtraBits(L.NextAndExtraBits) {}

  ObjCMethodList &operator=(const ObjCMethodList &L) {
    MethodAndHasMoreThanOneDecl = L.MethodAndHasMoreThanOneDecl;
    NextAndExtraBits = L.NextAndExtraBits;
    return *this;
  }

  ObjCMethodList *getNext() const { return NextAndExtraBits.getPointer(); }
  unsigned getBits() const { return NextAndExtraBits.getInt(); }
  void setNext(ObjCMethodList *L) { NextAndExtraBits.setPointer(L); }
  void setBits(unsigned B) { NextAndExtraBits.setInt(B); }

  ObjCMethodDecl *getMethod() const {
    return MethodAndHasMoreThanOneDecl.getPointer();
  }
  void setMethod(ObjCMethodDecl *M) {
    return MethodAndHasMoreThanOneDecl.setPointer(M);
  }

  bool hasMoreThanOneDecl() const {
    return MethodAndHasMoreThanOneDecl.getInt();
  }
  void setHasMoreThanOneDecl(bool B) {
    return MethodAndHasMoreThanOneDecl.setInt(B);
  }
};

}

#endif
