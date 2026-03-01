//===- Writer.h -------------------------------------------------*- C++ -*-===//
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
//===----------------------------------------------------------------------===//

#ifndef LLD_MACHO_WRITER_H
#define LLD_MACHO_WRITER_H

#include <cstdint>

namespace lld::macho {

class OutputSection;
class InputSection;
class Symbol;

class LoadCommand {
public:
  virtual ~LoadCommand() = default;
  virtual uint32_t getSize() const = 0;
  virtual void writeTo(uint8_t *buf) const = 0;
};

template <class LP> void writeResult();
void resetWriter();

void createSyntheticSections();

// Add bindings for symbols that need weak or non-lazy bindings.
void addNonLazyBindingEntries(const Symbol *, const InputSection *,
                              uint64_t offset, int64_t addend = 0);

extern OutputSection *firstTLVDataSection;

} // namespace lld::macho

#endif
