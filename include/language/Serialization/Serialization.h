//===--- Serialization.h - Codiramodule emission -----------------*- C++ -*-===//
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

#ifndef LANGUAGE_SERIALIZATION_H
#define LANGUAGE_SERIALIZATION_H

#include "language/Subsystems.h"

namespace language {

class SILModule;

namespace serialization {

/// Serialize a module to the given stream.
void writeToStream(
    raw_ostream &os, ModuleOrSourceFile DC, const SILModule *M,
    const SerializationOptions &options,
    const fine_grained_dependencies::SourceFileDepGraph *DepGraph);

/// Serialize module documentation to the given stream.
void writeDocToStream(raw_ostream &os, ModuleOrSourceFile DC,
                      StringRef GroupInfoPath);

/// Serialize module source info to the given stream.
void writeSourceInfoToStream(raw_ostream &os, ModuleOrSourceFile DC);

} // end namespace serialization
} // end namespace language

#endif // LANGUAGE_SERIALIZATION_H
