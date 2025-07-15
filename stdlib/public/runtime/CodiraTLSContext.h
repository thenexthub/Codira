//===--- CodiraTLSContext.h ------------------------------------------------===//
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

#ifndef LANGUAGE_RUNTIME_LANGUAGETLSCONTEXT_H
#define LANGUAGE_RUNTIME_LANGUAGETLSCONTEXT_H

#include "ExclusivityPrivate.h"

namespace language {
namespace runtime {

class CodiraTLSContext {
public:
  /// The set of tracked accesses.
  AccessSet accessSet;

  // The "implicit" boolean parameter which is passed to a dynamically
  // replaceable function.
  // If true, the original function should be executed instead of the
  // replacement function.
  bool CallOriginalOfReplacedFunction = false;

  static CodiraTLSContext &get();
};

} // namespace runtime
} // namespace language

#endif
