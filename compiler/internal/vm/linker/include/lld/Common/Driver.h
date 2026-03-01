//===- lld/Common/Driver.h - Linker Driver Emulator -----------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef LLD_COMMON_DRIVER_H
#define LLD_COMMON_DRIVER_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/raw_ostream.h"

namespace lld {
enum Flavor {
  Invalid,
  Gnu,     // -flavor gnu
  MinGW,   // -flavor gnu MinGW
  WinLink, // -flavor link
  Darwin,  // -flavor darwin
  Wasm,    // -flavor wasm
};

using Driver = bool (*)(llvm::ArrayRef<const char *>, llvm::raw_ostream &,
                        llvm::raw_ostream &, bool, bool);

struct DriverDef {
  Flavor f;
  Driver d;
};

struct Result {
  int retCode;
  bool canRunAgain;
};

// Generic entry point when using LLD as a library, safe for re-entry, supports
// crash recovery. Returns a general completion code and a boolean telling
// whether it can be called again. In some cases, a crash could corrupt memory
// and re-entry would not be possible anymore. Use exitLld() in that case to
// properly exit your application and avoid intermittent crashes on exit caused
// by cleanup.
Result lldMain(llvm::ArrayRef<const char *> args, llvm::raw_ostream &stdoutOS,
               llvm::raw_ostream &stderrOS, llvm::ArrayRef<DriverDef> drivers);
} // namespace lld

// With this macro, library users must specify which drivers they use, provide
// that information to lldMain() in the `drivers` param, and link the
// corresponding driver library in their executable.
#define LLD_HAS_DRIVER(name)                                                   \
  namespace lld {                                                              \
  namespace name {                                                             \
  bool link(llvm::ArrayRef<const char *> args, llvm::raw_ostream &stdoutOS,    \
            llvm::raw_ostream &stderrOS, bool exitEarly, bool disableOutput);  \
  }                                                                            \
  }

// An array which declares that all LLD drivers are linked in your executable.
// Must be used along with LLD_HAS_DRIVERS. See examples in LLD unittests.
#define LLD_ALL_DRIVERS                                                        \
  {                                                                            \
    {lld::WinLink, &lld::coff::link}, {lld::Gnu, &lld::elf::link},             \
        {lld::MinGW, &lld::mingw::link}, {lld::Darwin, &lld::macho::link}, {   \
      lld::Wasm, &lld::wasm::link                                              \
    }                                                                          \
  }

#endif
