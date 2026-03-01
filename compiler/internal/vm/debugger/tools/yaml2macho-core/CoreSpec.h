//===----------------------------------------------------------------------===//
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
///
/// \file
/// CoreSpec holds the internal representation of the data that will be
/// written into the corefile.  Theads, register sets within threads, registers
/// within register sets.  Block of memory.  Metadata about the CPU or binaries
/// that were present.
//===----------------------------------------------------------------------===//

#ifndef YAML2MACHOCOREFILE_CORESPEC_H
#define YAML2MACHOCOREFILE_CORESPEC_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct RegisterNameAndValue {
  std::string name;
  uint64_t value;
};

enum RegisterFlavor { GPR = 0, FPR, EXC };

struct RegisterSet {
  RegisterFlavor flavor;
  std::vector<RegisterNameAndValue> registers;
};

struct Thread {
  std::vector<RegisterSet> regsets;
};

enum MemoryType { UInt8 = 0, UInt32, UInt64 };

struct MemoryRegion {
  uint64_t addr;
  MemoryType type;
  uint32_t size;
  // One of the following formats.
  std::vector<uint8_t> bytes;
  std::vector<uint32_t> words;
  std::vector<uint64_t> doublewords;
};

struct AddressableBits {
  std::optional<int> lowmem_bits;
  std::optional<int> highmem_bits;
};

struct Binary {
  std::string name;
  std::string uuid;
  bool value_is_slide;
  uint64_t value;
};

struct CoreSpec {
  uint32_t cputype;
  uint32_t cpusubtype;
  int wordsize;

  std::vector<Thread> threads;
  std::vector<MemoryRegion> memory_regions;

  std::optional<AddressableBits> addressable_bits;
  std::vector<Binary> binaries;

  CoreSpec() : cputype(0), cpusubtype(0), wordsize(0) {}
};

CoreSpec from_yaml(char *buf, size_t len);

#endif
