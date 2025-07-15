//===--- TLSKeys.h - Reserved TLS keys ------------------------ -*- C++ -*-===//
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

#ifndef LANGUAGE_THREADING_TLSKEYS_H
#define LANGUAGE_THREADING_TLSKEYS_H

namespace language {

enum class tls_key {
  runtime,
  stdlib,
  compatibility50,
  concurrency_task,
  concurrency_executor_tracking_info,
  concurrency_fallback,
  observation_transaction
};

} // namespace language

#endif // LANGUAGE_THREADING_TLSKEYS_H
