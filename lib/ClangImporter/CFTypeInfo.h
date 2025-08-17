//===--- CFTypeInfo.h - Information about CF types  -------------*- C++ -*-===//
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
// This file provides support for reasoning about CF types
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_IMPORTER_CFTYPEINFO_H
#define LANGUAGE_IMPORTER_CFTYPEINFO_H

#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/ADT/StringRef.h"

namespace language::Core {
  class RecordDecl;
  class TypedefNameDecl;
}

namespace language {
namespace importer {

class CFPointeeInfo {
  bool IsValid;
  bool IsConst;
  toolchain::PointerUnion<const language::Core::RecordDecl *, const language::Core::TypedefNameDecl *>
      Decl;
  CFPointeeInfo() = default;

  static CFPointeeInfo forRecord(bool isConst, const language::Core::RecordDecl *decl) {
    assert(decl);
    CFPointeeInfo info;
    info.IsValid = true;
    info.IsConst = isConst;
    info.Decl = decl;
    return info;
  }

  static CFPointeeInfo forTypedef(const language::Core::TypedefNameDecl *decl) {
    assert(decl);
    CFPointeeInfo info;
    info.IsValid = true;
    info.IsConst = false;
    info.Decl = decl;
    return info;
  }

  static CFPointeeInfo forConstVoid() {
    CFPointeeInfo info;
    info.IsValid = true;
    info.IsConst = true;
    info.Decl = nullptr;
    return info;
  }

  static CFPointeeInfo forVoid() {
    CFPointeeInfo info;
    info.IsValid = true;
    info.IsConst = false;
    info.Decl = nullptr;
    return info;
  }

  static CFPointeeInfo forInvalid() {
    CFPointeeInfo info;
    info.IsValid = false;
    return info;
  }

public:
  static CFPointeeInfo classifyTypedef(const language::Core::TypedefNameDecl *decl);

  static bool isKnownCFTypeName(toolchain::StringRef name);

  bool isValid() const { return IsValid; }
  explicit operator bool() const { return isValid(); }

  bool isConst() const { return IsConst; }

  bool isVoid() const {
    assert(isValid());
    return Decl.isNull();
  }

  bool isRecord() const {
    assert(isValid());
    return !Decl.isNull() && Decl.is<const language::Core::RecordDecl *>();
  }
  const language::Core::RecordDecl *getRecord() const {
    assert(isRecord());
    return Decl.get<const language::Core::RecordDecl *>();
  }

  bool isTypedef() const {
    assert(isValid());
    return !Decl.isNull() && Decl.is<const language::Core::TypedefNameDecl *>();
  }
  const language::Core::TypedefNameDecl *getTypedef() const {
    assert(isTypedef());
    return Decl.get<const language::Core::TypedefNameDecl *>();
  }
};
}
}

#endif // LANGUAGE_IMPORTER_CFTYPEINFO_H
