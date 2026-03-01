//===- DWARF.h -----------------------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
//===-------------------------------------------------------------------===//

#ifndef LLD_MACHO_DWARF_H
#define LLD_MACHO_DWARF_H

#include "llvm/ADT/StringRef.h"
#include "llvm/DebugInfo/DWARF/DWARFObject.h"

namespace lld::macho {

class ObjFile;

// Implements the interface between LLVM's DWARF-parsing utilities and LLD's
// InputSection structures.
class DwarfObject final : public llvm::DWARFObject {
public:
  bool isLittleEndian() const override { return true; }

  std::optional<llvm::RelocAddrEntry> find(const llvm::DWARFSection &sec,
                                           uint64_t pos) const override {
    // TODO: implement this
    return std::nullopt;
  }

  void forEachInfoSections(
      llvm::function_ref<void(const llvm::DWARFSection &)> f) const override {
    f(infoSection);
  }

  llvm::StringRef getAbbrevSection() const override { return abbrevSection; }
  llvm::StringRef getStrSection() const override { return strSection; }

  llvm::DWARFSection const &getLineSection() const override {
    return lineSection;
  }

  llvm::DWARFSection const &getStrOffsetsSection() const override {
    return strOffsSection;
  }

  // Returns an instance of DwarfObject if the given object file has the
  // relevant DWARF debug sections.
  static std::unique_ptr<DwarfObject> create(ObjFile *);

private:
  llvm::DWARFSection infoSection;
  llvm::DWARFSection lineSection;
  llvm::DWARFSection strOffsSection;
  llvm::StringRef abbrevSection;
  llvm::StringRef strSection;
};

} // namespace lld::macho

#endif
