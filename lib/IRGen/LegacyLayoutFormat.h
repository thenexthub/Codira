//===--- LegacyLayoutFormat.h - YAML format for legacy layout ---*- C++ -*-===//
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
// This file defines the YAML format for the backward deployment type layout
// dump.
//
// When a class with @objc ancestry is statically visible to Clang code, older
// Objective-C runtimes do not give us the opportunity to run any code to
// initialize the class metadata when it is realized.
//
// This creates a problem if the class has resiliently-sized fields. Since the
// standard library and overlays are now built with resilience enabled, this
// creates a backward-compatibility issue where such class definitions would now
// require singleton metadata initialization instead of idempotent metadata
// initialization, and singleton metadata initialization precludes the class
// from being statically visible to Clang.
//
// To support this case, we emit fixed metadata for any such class, and in place
// of each resilient field type, we use previously-emitted fixed type info.
//
// This fixed type info must match the Codira standard library and overlays used
// for backward deployment, since on older Objective-C runtimes these layouts
// will be used at runtime.
//
// However, since these types are resilient, their layouts might change in the
// future. Newer Objective-C runtimes will expose a hook allowing the Codira
// runtime to re-compute the class layout when the class is realized.
//
// Note that except for metadata emission, field accesses and instance
// allocation for such classes must proceed as if they use singleton metadata
// initialization, loading the field offsets from global variables and loading
// the size and alignment dynamically from metadata when allocating.
//
// Also, any Codira-side accesses of the metadata must call the metadata accessor
// function, allowing the Codira runtime to re-initialize the layout if
// necessary.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_LEGACY_LAYOUT_FORMAT_H
#define LANGUAGE_IRGEN_LEGACY_LAYOUT_FORMAT_H

#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/YAMLTraits.h"
#include "toolchain/Support/raw_ostream.h"

namespace language {
namespace irgen {

struct YAMLTypeInfoNode {
  std::string Name;
  uint64_t Size;
  uint64_t Alignment;
  uint64_t NumExtraInhabitants;

  bool operator<(const YAMLTypeInfoNode &other) const {
    return Name < other.Name;
  }
};

struct YAMLModuleNode {
  StringRef Name;
  std::vector<YAMLTypeInfoNode> Decls;
};

} // namespace irgen
} // namespace language

namespace toolchain {
namespace yaml {

template <> struct MappingTraits<language::irgen::YAMLTypeInfoNode> {
  static void mapping(IO &io, language::irgen::YAMLTypeInfoNode &node) {
    io.mapRequired("Name", node.Name);
    io.mapRequired("Size", node.Size);
    io.mapRequired("Alignment", node.Alignment);
    io.mapRequired("ExtraInhabitants", node.NumExtraInhabitants);
  }
};

template <> struct MappingTraits<language::irgen::YAMLModuleNode> {
  static void mapping(IO &io, language::irgen::YAMLModuleNode &node) {
    io.mapRequired("Name", node.Name);
    io.mapOptional("Decls", node.Decls);
  }
};

} // namespace yaml
} // namespace toolchain

TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(language::irgen::YAMLTypeInfoNode)
TOOLCHAIN_YAML_IS_DOCUMENT_LIST_VECTOR(language::irgen::YAMLModuleNode)

#endif // LANGUAGE_IRGEN_LEGACY_LAYOUT_FORMAT_H
