//===-- CompilerInstance.h - Flang Compiler Instance ------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_SUPPORT_STRINGOSTREAM_H
#define LANGUAGE_COMPABILITY_SUPPORT_STRINGOSTREAM_H

#include <toolchain/Support/raw_ostream.h>

namespace language::Compability::support {

/// Helper class to maintain both the an toolchain::raw_string_ostream object and
/// its associated buffer.
class string_ostream : public toolchain::raw_string_ostream {
private:
  std::string buf;

public:
  string_ostream() : toolchain::raw_string_ostream(buf) {}
};

} // namespace language::Compability::support

#endif // FORTRAN_SUPPORT_STRINGOSTREAM_H
