//===- LTO.h ----------------------------------------------------*- C++ -*-===//
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
//
// This file provides a way to combine bitcode files into one COFF
// file by compiling them using LLVM.
//
// If LTO is in use, your input files are not in regular COFF files
// but instead LLVM bitcode files. In that case, the linker has to
// convert bitcode files into the native format so that we can create
// a COFF file that contains native code. This file provides that
// functionality.
//
//===----------------------------------------------------------------------===//

#ifndef LLD_COFF_LTO_H
#define LLD_COFF_LTO_H

#include "lld/Common/LLVM.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>
#include <vector>

namespace llvm::lto {
struct Config;
class LTO;
}

namespace lld::coff {

class BitcodeFile;
class InputFile;
class COFFLinkerContext;

class BitcodeCompiler {
public:
  BitcodeCompiler(COFFLinkerContext &ctx);
  ~BitcodeCompiler();

  void add(BitcodeFile &f);
  std::vector<InputFile *> compile();

private:
  std::unique_ptr<llvm::lto::LTO> ltoObj;
  std::vector<std::pair<std::string, SmallString<0>>> buf;
  std::vector<std::unique_ptr<MemoryBuffer>> files;
  std::vector<std::string> file_names;
  std::unique_ptr<llvm::raw_fd_ostream> indexFile;
  llvm::DenseSet<StringRef> thinIndices;

  std::string getThinLTOOutputFile(StringRef path);
  llvm::lto::Config createConfig();

  COFFLinkerContext &ctx;
};
}

#endif
