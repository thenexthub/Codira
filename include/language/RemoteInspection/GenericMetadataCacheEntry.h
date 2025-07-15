//===--- GenericMetadataCacheEntry.h ----------------------------*- C++ -*-===//
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
// Declares a struct that mirrors the layout of GenericCacheEntry in
// Metadata.cpp and use a static assert to check that the offset of
// the member Value match between the two.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_REFLECTION_GENERICMETADATACACHEENTRY_H
#define LANGUAGE_REFLECTION_GENERICMETADATACACHEENTRY_H

#include <cstdint>

namespace language {

template<typename StoredPointer>
struct GenericMetadataCacheEntry {
  StoredPointer TrackingInfo;
  uint16_t NumKeyParameters;
  uint16_t NumWitnessTables;
  uint16_t NumPacks;
  uint16_t NumShapeClasses;
  StoredPointer PackShapeDescriptors;
  uint32_t Hash;
  StoredPointer Value;
};

} // namespace language

#endif // LANGUAGE_REFLECTION_GENERICMETADATACACHEENTRY_H
