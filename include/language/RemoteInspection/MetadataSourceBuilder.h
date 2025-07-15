//===--- MetadataSourceBuilder.h - Metadata Source Builder ------*- C++ -*-===//
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
// Implements utilities for constructing MetadataSources.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_REFLECTION_METADATASOURCEBUILDER_H
#define LANGUAGE_REFLECTION_METADATASOURCEBUILDER_H

#include "language/RemoteInspection/MetadataSource.h"

namespace language {
namespace reflection {

class MetadataSourceBuilder {
  std::vector<std::unique_ptr<const MetadataSource>> MetadataSourcePool;
public:
  using Source = const MetadataSource *;

  MetadataSourceBuilder() {}

  MetadataSourceBuilder(const MetadataSourceBuilder &Other) = delete;
  MetadataSourceBuilder &operator=(const MetadataSourceBuilder &Other) = delete;

  template <typename MetadataSourceTy, typename... Args>
  MetadataSourceTy *make_source(Args... args) {
    auto MS = new MetadataSourceTy(::std::forward<Args>(args)...);
    MetadataSourcePool.push_back(std::unique_ptr<const MetadataSource>(MS));
    return MS;
  }

  const GenericArgumentMetadataSource *
  createGenericArgument(unsigned Index, const MetadataSource *Source) {
    return GenericArgumentMetadataSource::create(*this, Index, Source);
  }

  const MetadataCaptureMetadataSource *
  createMetadataCapture(unsigned Index) {
    return MetadataCaptureMetadataSource::create(*this, Index);
  }

  const ReferenceCaptureMetadataSource *
  createReferenceCapture(unsigned Index) {
    return ReferenceCaptureMetadataSource::create(*this, Index);
  }

  const ClosureBindingMetadataSource *
  createClosureBinding(unsigned Index) {
    return ClosureBindingMetadataSource::create(*this, Index);
  }

  const SelfMetadataSource *
  createSelf() {
    return SelfMetadataSource::create(*this);
  }

  const SelfWitnessTableMetadataSource *
  createSelfWitnessTable() {
    return SelfWitnessTableMetadataSource::create(*this);
  }
};

} // end namespace reflection
} // end namespace language

#endif // LANGUAGE_REFLECTION_METADATASOURCEBUILDER_H
