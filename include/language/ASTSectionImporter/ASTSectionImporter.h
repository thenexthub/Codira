//===--- ASTSectionImporter.h - Import AST Section Modules ------*- C++ -*-===//
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
// This file implements support for loading modules serialized into a
// Mach-O AST section into Codira.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_ASTSECTION_IMPORTER_H
#define LANGUAGE_ASTSECTION_IMPORTER_H

#include "language/Basic/Toolchain.h"
#include "language/Serialization/Validation.h"
#include "toolchain/Support/Error.h"
#include <string>

namespace toolchain {
class Triple;
}
namespace language {
  class MemoryBufferSerializedModuleLoader;

  class ASTSectionParseError : public toolchain::ErrorInfo<ASTSectionParseError> {
  public:
    static char ID;

    serialization::Status Error;
    std::string ErrorMessage;

    ASTSectionParseError(serialization::Status Error,
                         StringRef ErrorMessage = {})
        : Error(Error), ErrorMessage(ErrorMessage) {
      assert(Error != serialization::Status::Valid);
    }
    ASTSectionParseError(const ASTSectionParseError &Other)
        : ASTSectionParseError(Other.Error, Other.ErrorMessage) {}
    ASTSectionParseError &operator=(const ASTSectionParseError &Other) {
      Error = Other.Error;
      ErrorMessage = Other.ErrorMessage;
      return *this;
    }

    std::string toString() const;
    void log(toolchain::raw_ostream &OS) const override;
    std::error_code convertToErrorCode() const override;
  };

  /// Provided a memory buffer with an entire Mach-O __language_ast section, this
  /// function makes memory buffer copies of all language modules found in it and
  /// registers them using registerMemoryBuffer() so they can be found by
  /// loadModule().
  /// \param filter  If fully specified, only matching modules are registered.
  /// \return a vector of the access path of all modules found in the
  /// section if successful.
  toolchain::Expected<SmallVector<std::string, 4>>
  parseASTSection(MemoryBufferSerializedModuleLoader &Loader,
                  StringRef Data, const toolchain::Triple &filter);

  // An old version temporarily left for remaining call site.
  // TODO: remove this once the other version is committed and used.
  bool parseASTSection(MemoryBufferSerializedModuleLoader &Loader,
                       StringRef Data, const toolchain::Triple &filter,
                       SmallVectorImpl<std::string> &foundModules);

}
#endif
