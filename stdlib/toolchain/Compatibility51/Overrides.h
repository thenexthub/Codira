//===--- Overrides.h - Compat overrides for Swift 5.0 runtime ------s------===//
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
//  This file provides compatibility override hooks for Swift 5.1 runtimes.
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Metadata.h"
#include "llvm/ADT/StringRef.h"

namespace language {

using ConformsToSwiftProtocol_t =
  const ProtocolConformanceDescriptor *(const Metadata * const type,
                                        const ProtocolDescriptor *protocol,
                                        llvm::StringRef moduleName);

const ProtocolConformanceDescriptor *
swift51override_conformsToSwiftProtocol(const Metadata * const type,
                                        const ProtocolDescriptor *protocol,
                                        llvm::StringRef moduleName,
                                        ConformsToSwiftProtocol_t *original);

} // end namespace language
