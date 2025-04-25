//===--- CancellationToken.h - ----------------------------------*- C++ -*-===//
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

#ifndef LLVM_SOURCEKIT_SUPPORT_CANCELLATION_TOKEN_H
#define LLVM_SOURCEKIT_SUPPORT_CANCELLATION_TOKEN_H

namespace SourceKit {

/// A token that uniquely identifies a SourceKit request that's served by a
/// \c SwiftASTConsumer. Used to cancel the request.
/// If the cancellation token is \c nullptr, it means that cancellation is not
/// supported.
typedef const void *SourceKitCancellationToken;

} // namespace SourceKit

#endif
