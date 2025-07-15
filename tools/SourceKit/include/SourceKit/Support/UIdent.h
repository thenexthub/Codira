//===--- UIdent.h - ---------------------------------------------*- C++ -*-===//
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

#ifndef TOOLCHAIN_SOURCEKIT_SUPPORT_UIDENT_H
#define TOOLCHAIN_SOURCEKIT_SUPPORT_UIDENT_H

#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Compiler.h"
#include <atomic>

namespace toolchain {
  class raw_ostream;
}

namespace SourceKit {

/// A string identifier that is uniqued for the process lifetime.
/// The string identifier should not contain any spaces.
class UIdent {
  void *Ptr = 0;

  explicit UIdent(void *Ptr) : Ptr(Ptr) { }

public:
  UIdent() = default;
  explicit UIdent(toolchain::StringRef Str);

  bool isValid() const { return Ptr != 0; }
  bool isInvalid() const { return !isValid(); }

  void *getAsOpaqueValue() const { return Ptr; }
  static UIdent getFromOpaqueValue(void *Val) {
    return UIdent(Val);
  }

  void setTag(void *Tag);
  void *getTag() const;

  friend bool operator==(const UIdent &LHS, const UIdent &RHS) {
    return LHS.Ptr == RHS.Ptr;
  }
  friend bool operator!=(const UIdent &LHS, const UIdent &RHS) {
    return !(LHS == RHS);
  }

  toolchain::StringRef getName() const;
  const char *c_str() const;

  TOOLCHAIN_ATTRIBUTE_USED void dump() const;
  void print(toolchain::raw_ostream &OS) const;
};

class LazyUIdent {
  const char *Name;
  mutable std::atomic<UIdent> UID;
public:
  LazyUIdent(const char *Name) : Name(Name) { }

  UIdent get() const {
    if (UID.load().isInvalid())
      UID = UIdent(Name);
    return UID;
  }

  operator UIdent() const {
    return get();
  }
};

} // namespace SourceKit

#endif
