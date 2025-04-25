//===--- XMLValidator.h - XML validation ------------------------*- C++ -*-===//
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

#ifndef SWIFT_IDE_TEST_XML_VALIDATOR_H
#define SWIFT_IDE_TEST_XML_VALIDATOR_H

#include "language/Basic/LLVM.h"
#include "llvm/ADT/StringRef.h"

namespace language {

class XMLValidator {
  struct Implementation;
  Implementation *Impl;

public:
  XMLValidator();
  ~XMLValidator();

  void setSchema(StringRef FileName);

  enum class ErrorCode {
    Valid,
    NotCompiledIn,
    NoSchema,
    BadSchema,
    NotWellFormed,
    NotValid,
    InternalError,
  };
  struct Status {
    ErrorCode Code;
    std::string Message;
  };

  Status validate(const std::string &XML);
};

} // end namespace language

#endif // SWIFT_IDE_TEST_XML_VALIDATOR_H

