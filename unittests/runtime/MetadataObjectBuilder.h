//===--- MetadataObjectBuilder.h -------------------------------*- C++ -*--===//
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
// Defines routines for use with ObjectBuilder that are specifically
// useful for building metadata objects.
//
//===----------------------------------------------------------------------===//

#ifndef METADATA_OBJECT_BUILDER_H
#define METADATA_OBJECT_BUILDER_H

#include "ObjectBuilder.h"
#include "SpecifierDSL.h"
#include "language/ABI/Metadata.h"
#include <sstream>
#include <type_traits>

namespace language {
using namespace specifierDSL;

/// Add a simple ModuleContextDescriptor with the given name.
inline void addModuleContextDescriptor(AnyObjectBuilder &builder,
                                       StringRef moduleName) {
  // Context descriptor flags
  auto contextFlags = ContextDescriptorFlags(ContextDescriptorKind::Module,
                                             /*generic*/ false,
                                             /*unique*/ true,
                                             /*hasInvertibleProtocols*/ false,
                                             /*kindSpecific*/ 0);
  builder.add32(contextFlags.getIntValue());

  // Parent
  builder.add32(0);

  // Name
  builder.addRelativeReferenceToString(moduleName);
}

/// Add a simple ProtocolDescriptor with the given base protocols
/// but no other requirements.
inline void addProtocolDescriptor(AnyObjectBuilder &builder,
                                  const ModuleContextDescriptor *module,
                                  StringRef protocolName,
                            toolchain::ArrayRef<ProtocolSpecifier> baseProtocols) {
  // Context descriptor flags
  auto contextFlags = ContextDescriptorFlags(ContextDescriptorKind::Protocol,
                                             /*generic*/ false,
                                             /*unique*/ true,
                                             /*hasInvertibleProtocols*/ false,
                                             /*kindSpecific*/ 0);
  builder.add32(contextFlags.getIntValue());

  // Parent
  builder.addRelativeIndirectReference(module, /*addend*/ 1);

  // Name
  builder.addRelativeReferenceToString(protocolName);

  // NumRequirementsInSignature
  builder.add32(baseProtocols.size());

  // NumRequirements
  builder.add32(baseProtocols.size());

  // Associated type names
  builder.add32(0);

  ObjectRef<const char> selfType;
  if (!baseProtocols.empty()) {
    selfType = createMangledTypeString(builder, typeParam(0, 0));
  }

  // Requirement signature requirement descriptors
  for (auto &baseProtocol : baseProtocols) {
    addConformanceRequirement(builder, selfType, baseProtocol);
  }

  // Protocol requirement descriptors
  for (auto baseProtocol : baseProtocols) {
    (void) baseProtocol; // Requirements don't actually collect specifics here

    // Flags
    auto flags = ProtocolRequirementFlags(
                              ProtocolRequirementFlags::Kind::BaseProtocol);
    builder.add32(flags.getIntValue());

    // Default implementation
    builder.add32(0);
  }
}

template <class T>
class GlobalObjectBuilder {
  ObjectBuilder<T> builder;
public:
  template <class Fn>
  GlobalObjectBuilder(Fn &&fn) {
    std::forward<Fn>(fn)(builder);
    (void) builder.finish();
  }
  const T *get() const { return builder.get(); }
};

/// Build a global object with the given lambda.
///
/// This uses a static local to magically cache and preserve the
/// global object.  This global caching is uniqued by the template
/// arguments to this function, so callers should not re-use lambdas
/// for different calls.
template <class T, class Fn>
inline const T *buildGlobalObject(Fn &&fn) {
  static const GlobalObjectBuilder<T> builder(std::forward<Fn>(fn));
  return builder.get();
}

/// Build a global ModuleContextDescriptor with the name returned by the
/// given lambda.
///
/// This uses a static local to magically cache and preserve the
/// global object.  This global caching is uniqued by the template
/// arguments to this function, so callers should not re-use lambdas
/// for different calls.
template <class Fn>
inline const ModuleContextDescriptor *
buildGlobalModuleContextDescriptor(Fn &&fn) {
  static const GlobalObjectBuilder<ModuleContextDescriptor> builder(
      [&](AnyObjectBuilder &builder) {
    addModuleContextDescriptor(builder, std::forward<Fn>(fn)());
  });
  return builder.get();
}

/// Build a global protocol descriptor for an empty protocol with
/// the name returned by the given lambda.
///
/// This uses a static local to magically cache and preserve the
/// global object.  This global caching is uniqued by the template
/// arguments to this function, so callers should not re-use lambdas
/// for different calls.
template <class Fn>
inline const ProtocolDescriptor *
buildGlobalProtocolDescriptor(const ModuleContextDescriptor *module, Fn &&fn) {
  static const GlobalObjectBuilder<ProtocolDescriptor> builder(
      [&](AnyObjectBuilder &builder) {
    addProtocolDescriptor(builder, module, std::forward<Fn>(fn)(), {});
  });
  return builder.get();
}

} // end namespace language

#endif
