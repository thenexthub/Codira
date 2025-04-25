//===--- Replacement.h - Migrator Replacements ------------------*- C++ -*-===//
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

#ifndef SWIFT_MIGRATOR_REPLACEMENT_H
#define SWIFT_MIGRATOR_REPLACEMENT_H
namespace language {
namespace migrator {

struct Replacement {
  size_t Offset;
  size_t Remove;
  std::string Text;

  bool isRemove() const {
    return Remove > 0;
  }

  bool isInsert() const { return Remove == 0 && !Text.empty(); }

  bool isReplace() const { return Remove > 0 && !Text.empty(); }

  size_t endOffset() const {
    if (isInsert()) {
      return Offset + Text.size();
    } else {
      return Offset + Remove;
    }
  }

  bool operator<(const Replacement &Other) const {
    return Offset < Other.Offset;
  }

  bool operator==(const Replacement &Other) const {
    return Offset == Other.Offset && Remove == Other.Remove &&
      Text == Other.Text;
  }
};

} // end namespace migrator
} // end namespace language

#endif // SWIFT_MIGRATOR_REPLACEMENT_H
