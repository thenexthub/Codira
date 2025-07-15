//===- BinaryStreamError.h - Error extensions for Binary Streams *- C++ -*-===//
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

#ifndef TOOLCHAIN_SUPPORT_BINARYSTREAMERROR_H
#define TOOLCHAIN_SUPPORT_BINARYSTREAMERROR_H

#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Error.h"

#include <string>

namespace toolchain {
enum class stream_error_code {
  unspecified,
  stream_too_short,
  invalid_array_size,
  invalid_offset,
  filesystem_error
};

/// Base class for errors originating when parsing raw PDB files
class BinaryStreamError : public ErrorInfo<BinaryStreamError> {
public:
  static char ID;
  explicit BinaryStreamError(stream_error_code C);
  explicit BinaryStreamError(StringRef Context);
  BinaryStreamError(stream_error_code C, StringRef Context);

  void log(raw_ostream &OS) const override;
  std::error_code convertToErrorCode() const override;

  StringRef getErrorMessage() const;

  stream_error_code getErrorCode() const { return Code; }

private:
  std::string ErrMsg;
  stream_error_code Code;
};
} // namespace toolchain

#endif // TOOLCHAIN_SUPPORT_BINARYSTREAMERROR_H
