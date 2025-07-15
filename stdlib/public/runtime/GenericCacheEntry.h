//===--- GenericCacheEntry.h ------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_RUNTIME_GENERICCACHEENTRY_H
#define LANGUAGE_RUNTIME_GENERICCACHEENTRY_H

#include "MetadataCache.h"
#include "language/ABI/Metadata.h"
#include "language/ABI/MetadataValues.h"
#include "language/Basic/Unreachable.h"
#include "language/RemoteInspection/GenericMetadataCacheEntry.h"
#include "language/Runtime/LibPrespecialized.h"

namespace language {

bool areAllTransitiveMetadataComplete_cheap(const Metadata *type);
PrivateMetadataState inferStateForMetadata(Metadata *metadata);
MetadataDependency checkTransitiveCompleteness(const Metadata *initialType);

struct GenericCacheEntry final
    : VariadicMetadataCacheEntryBase<GenericCacheEntry> {
  static const char *getName() { return "GenericCache"; }

  // The constructor/allocate operations that take a `const Metadata *`
  // are used for the insertion of canonical specializations.
  // The metadata is always complete after construction.

  GenericCacheEntry(MetadataCacheKey key, MetadataWaitQueue::Worker &worker,
                    MetadataRequest request, const Metadata *candidate)
      : VariadicMetadataCacheEntryBase(key, worker,
                                       PrivateMetadataState::Complete,
                                       const_cast<Metadata *>(candidate)) {}

  AllocationResult allocate(const Metadata *candidate) {
    language_unreachable("always short-circuited");
  }

  static bool allowMangledNameVerification(const Metadata *candidate) {
    // Disallow mangled name verification for specialized candidates
    // because it will trigger recursive entry into the language_once
    // in cacheCanonicalSpecializedMetadata.
    // TODO: verify mangled names in a second pass in that function.
    return false;
  }

  // The constructor/allocate operations that take a descriptor
  // and arguments are used along the normal allocation path.

  GenericCacheEntry(MetadataCacheKey key, MetadataWaitQueue::Worker &worker,
                    MetadataRequest request,
                    const TypeContextDescriptor *description,
                    const void *const *arguments)
      : VariadicMetadataCacheEntryBase(key, worker,
                                       PrivateMetadataState::Allocating,
                                       /*candidate*/ nullptr) {}

  AllocationResult allocate(const TypeContextDescriptor *description,
                            const void *const *arguments) {
    if (auto *prespecialized =
            getLibPrespecializedMetadata(description, arguments))
      return {prespecialized, PrivateMetadataState::Complete};

    // Find a pattern.  Currently we always use the default pattern.
    auto &generics = description->getFullGenericContextHeader();
    auto pattern = generics.DefaultInstantiationPattern.get();

    // Call the pattern's instantiation function.
    auto metadata =
        pattern->InstantiationFunction(description, arguments, pattern);

    // If there's no completion function, do a quick-and-dirty check to
    // see if all of the type arguments are already complete.  If they
    // are, we can broadcast completion immediately and potentially avoid
    // some extra locking.
    PrivateMetadataState state;
    if (pattern->CompletionFunction.isNull()) {
      if (areAllTransitiveMetadataComplete_cheap(metadata)) {
        state = PrivateMetadataState::Complete;
      } else {
        state = PrivateMetadataState::NonTransitiveComplete;
      }
    } else {
      state = inferStateForMetadata(metadata);
    }

    return {metadata, state};
  }

  static bool
  allowMangledNameVerification(const TypeContextDescriptor *description,
                               const void *const *arguments) {
    return true;
  }

  MetadataStateWithDependency
  tryInitialize(Metadata *metadata, PrivateMetadataState state,
                PrivateMetadataCompletionContext *context) {
    assert(state != PrivateMetadataState::Complete);

    // Finish the completion function.
    if (state < PrivateMetadataState::NonTransitiveComplete) {
      // Find a pattern.  Currently we always use the default pattern.
      auto &generics =
          metadata->getTypeContextDescriptor()->getFullGenericContextHeader();
      auto pattern = generics.DefaultInstantiationPattern.get();

      // Complete the metadata's instantiation.
      auto dependency =
          pattern->CompletionFunction(metadata, &context->Public, pattern);

      // If this failed with a dependency, infer the current metadata state
      // and return.
      if (dependency) {
        return {inferStateForMetadata(metadata), dependency};
      }
    }

    // Check for transitive completeness.
    if (auto dependency = checkTransitiveCompleteness(metadata)) {
      return {PrivateMetadataState::NonTransitiveComplete, dependency};
    }

    // We're done.
    return {PrivateMetadataState::Complete, MetadataDependency()};
  }
};

} // namespace language

#endif
