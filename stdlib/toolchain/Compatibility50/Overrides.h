//===--- Overrides.h --- Compat overrides for Swift 5.0 runtime ----s------===//
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
//  This file provides compatibility override hooks for Swift 5.0 runtimes.
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Metadata.h"

namespace language {

using ConformsToProtocol_t =
  const WitnessTable *(const Metadata *, const ProtocolDescriptor *);

const WitnessTable *
swift50override_conformsToProtocol(const Metadata * const type,
  const ProtocolDescriptor *protocol,
  ConformsToProtocol_t *original);

}
