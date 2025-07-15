//===--- Overrides.h - Compat overrides for Codira 5.0 runtime ------s------===//
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
//  This file provides compatibility override hooks for Codira 5.1 runtimes.
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Metadata.h"
#include "toolchain/ADT/StringRef.h"

namespace language {

using ConformsToCodiraProtocol_t =
  const ProtocolConformanceDescriptor *(const Metadata * const type,
                                        const ProtocolDescriptor *protocol,
                                        toolchain::StringRef moduleName);

const ProtocolConformanceDescriptor *
language51override_conformsToCodiraProtocol(const Metadata * const type,
                                        const ProtocolDescriptor *protocol,
                                        toolchain::StringRef moduleName,
                                        ConformsToCodiraProtocol_t *original);

} // end namespace language
