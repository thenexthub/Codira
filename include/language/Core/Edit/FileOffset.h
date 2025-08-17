//===- FileOffset.h - Offset in a file --------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_EDIT_FILEOFFSET_H
#define LANGUAGE_CORE_EDIT_FILEOFFSET_H

#include "language/Core/Basic/SourceLocation.h"
#include <tuple>

namespace language::Core {
namespace edit {

class FileOffset {
  FileID FID;
  unsigned Offs = 0;

public:
  FileOffset() = default;
  FileOffset(FileID fid, unsigned offs) : FID(fid), Offs(offs) {}

  bool isInvalid() const { return FID.isInvalid(); }

  FileID getFID() const { return FID; }
  unsigned getOffset() const { return Offs; }

  FileOffset getWithOffset(unsigned offset) const {
    FileOffset NewOffs = *this;
    NewOffs.Offs += offset;
    return NewOffs;
  }

  friend bool operator==(FileOffset LHS, FileOffset RHS) {
    return LHS.FID == RHS.FID && LHS.Offs == RHS.Offs;
  }

  friend bool operator!=(FileOffset LHS, FileOffset RHS) {
    return !(LHS == RHS);
  }

  friend bool operator<(FileOffset LHS, FileOffset RHS) {
    return std::tie(LHS.FID, LHS.Offs) < std::tie(RHS.FID, RHS.Offs);
  }

  friend bool operator>(FileOffset LHS, FileOffset RHS) {
    return RHS < LHS;
  }

  friend bool operator>=(FileOffset LHS, FileOffset RHS) {
    return !(LHS < RHS);
  }

  friend bool operator<=(FileOffset LHS, FileOffset RHS) {
    return !(RHS < LHS);
  }
};

} // namespace edit
} // namespace language::Core

#endif // LANGUAGE_CORE_EDIT_FILEOFFSET_H
