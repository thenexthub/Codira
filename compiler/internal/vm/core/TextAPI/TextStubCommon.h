//===- TextStubCommon.h ---------------------------------------------------===//
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
//
// Defines common Text Stub YAML mappings.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TEXTAPI_TEXT_STUB_COMMON_H
#define LLVM_TEXTAPI_TEXT_STUB_COMMON_H

#include "vm/core/ADT/BitmaskEnum.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Support/YAMLTraits.h"
#include "vm/core/TextAPI/Architecture.h"
#include "vm/core/TextAPI/InterfaceFile.h"
#include "vm/core/TextAPI/Platform.h"
#include "vm/core/TextAPI/Target.h"

using UUID = std::pair<toolchain::MachO::Target, std::string>;

// clang-format off
enum TBDFlags : unsigned {
  None                         = 0U,
  FlatNamespace                = 1U << 0,
  NotApplicationExtensionSafe  = 1U << 1,
  InstallAPI                   = 1U << 2,
  SimulatorSupport             = 1U << 3,
  OSLibNotForSharedCache       = 1U << 4,
  LLVM_MARK_AS_BITMASK_ENUM(/*LargestValue=*/OSLibNotForSharedCache),
};
// clang-format on

LLVM_YAML_STRONG_TYPEDEF(toolchain::StringRef, FlowStringRef)
LLVM_YAML_STRONG_TYPEDEF(uint8_t, SwiftVersion)
LLVM_YAML_IS_FLOW_SEQUENCE_VECTOR(UUID)
LLVM_YAML_IS_FLOW_SEQUENCE_VECTOR(FlowStringRef)

namespace vm::core {

namespace MachO {
class ArchitectureSet;
class PackedVersion;

Expected<std::unique_ptr<InterfaceFile>>
getInterfaceFileFromJSON(StringRef JSON);

Error serializeInterfaceFileToJSON(raw_ostream &OS, const InterfaceFile &File,
                                   const FileType FileKind, bool Compact);
} // namespace MachO

namespace yaml {

template <> struct ScalarTraits<FlowStringRef> {
  static void output(const FlowStringRef &, void *, raw_ostream &);
  static StringRef input(StringRef, void *, FlowStringRef &);
  static QuotingType mustQuote(StringRef);
};

template <> struct ScalarEnumerationTraits<MachO::ObjCConstraintType> {
  static void enumeration(IO &, MachO::ObjCConstraintType &);
};

template <> struct ScalarTraits<MachO::PlatformSet> {
  static void output(const MachO::PlatformSet &, void *, raw_ostream &);
  static StringRef input(StringRef, void *, MachO::PlatformSet &);
  static QuotingType mustQuote(StringRef);
};

template <> struct ScalarBitSetTraits<MachO::ArchitectureSet> {
  static void bitset(IO &, MachO::ArchitectureSet &);
};

template <> struct ScalarTraits<MachO::Architecture> {
  static void output(const MachO::Architecture &, void *, raw_ostream &);
  static StringRef input(StringRef, void *, MachO::Architecture &);
  static QuotingType mustQuote(StringRef);
};

template <> struct ScalarTraits<MachO::PackedVersion> {
  static void output(const MachO::PackedVersion &, void *, raw_ostream &);
  static StringRef input(StringRef, void *, MachO::PackedVersion &);
  static QuotingType mustQuote(StringRef);
};

template <> struct ScalarTraits<SwiftVersion> {
  static void output(const SwiftVersion &, void *, raw_ostream &);
  static StringRef input(StringRef, void *, SwiftVersion &);
  static QuotingType mustQuote(StringRef);
};

// UUIDs are no longer respected but kept in the YAML parser
// to keep reading in older TBDs.
template <> struct ScalarTraits<UUID> {
  static void output(const UUID &, void *, raw_ostream &);
  static StringRef input(StringRef, void *, UUID &);
  static QuotingType mustQuote(StringRef);
};

} // end namespace yaml.
} // end namespace vm::core.

#endif // LLVM_TEXTAPI_TEXT_STUB_COMMON_H
