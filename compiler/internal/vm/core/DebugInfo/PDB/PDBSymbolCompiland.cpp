//===- PDBSymbolCompiland.cpp - compiland details ---------------*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/IPDBSession.h"
#include "vm/core/DebugInfo/PDB/IPDBSourceFile.h"

#include "vm/core/DebugInfo/PDB/ConcreteSymbolEnumerator.h"
#include "vm/core/DebugInfo/PDB/PDBSymDumper.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolCompiland.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolCompilandDetails.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolCompilandEnv.h"

#include "vm/core/ADT/StringSwitch.h"
#include "vm/core/Support/Path.h"

using namespace vm::core;
using namespace vm::core::pdb;

void PDBSymbolCompiland::dump(PDBSymDumper &Dumper) const {
  Dumper.dump(*this);
}

std::string PDBSymbolCompiland::getSourceFileName() const {
  return sys::path::filename(getSourceFileFullPath()).str();
}

std::string PDBSymbolCompiland::getSourceFileFullPath() const {
  std::string SourceFileFullPath;

  // RecordedResult could be the basename, relative path or full path of the
  // source file. Usually it is retrieved and recorded from the command that
  // compiles this compiland.
  //
  //  cmd FileName          -> RecordedResult = .\\FileName
  //  cmd (Path)\\FileName  -> RecordedResult = (Path)\\FileName
  //
  std::string RecordedResult = RawSymbol->getSourceFileName();

  if (RecordedResult.empty()) {
    if (auto Envs = findAllChildren<PDBSymbolCompilandEnv>()) {
      std::string EnvWorkingDir, EnvSrc;

      while (auto Env = Envs->getNext()) {
        std::string Var = Env->getName();
        if (Var == "cwd") {
          EnvWorkingDir = Env->getValue();
          continue;
        }
        if (Var == "src") {
          EnvSrc = Env->getValue();
          if (sys::path::is_absolute(EnvSrc))
            return EnvSrc;
          RecordedResult = EnvSrc;
          continue;
        }
      }
      if (!EnvWorkingDir.empty() && !EnvSrc.empty()) {
        auto Len = EnvWorkingDir.length();
        if (EnvWorkingDir[Len - 1] != '/' && EnvWorkingDir[Len - 1] != '\\') {
          std::string Path = EnvWorkingDir + "\\" + EnvSrc;
          toolchain::replace(Path, '/', '\\');
          // We will return it as full path if we can't find a better one.
          if (sys::path::is_absolute(Path))
            SourceFileFullPath = Path;
        }
      }
    }
  }

  if (!RecordedResult.empty()) {
    if (sys::path::is_absolute(RecordedResult))
      return RecordedResult;

    // This searches name that has same basename as the one in RecordedResult.
    auto OneSrcFile = Session.findOneSourceFile(
        this, RecordedResult, PDB_NameSearchFlags::NS_CaseInsensitive);
    if (OneSrcFile)
      return OneSrcFile->getFileName();
  }

  // At this point, we have to walk through all source files of this compiland,
  // and determine the right source file if any that is used to generate this
  // compiland based on language indicated in compilanddetails language field.
  auto Details = findOneChild<PDBSymbolCompilandDetails>();
  PDB_Lang Lang = Details ? Details->getLanguage() : PDB_Lang::Cpp;
  auto SrcFiles = Session.getSourceFilesForCompiland(*this);
  if (SrcFiles) {
    while (auto File = SrcFiles->getNext()) {
      std::string FileName = File->getFileName();
      auto file_extension = sys::path::extension(FileName);
      if (StringSwitch<bool>(file_extension.lower())
              .Case(".cpp", Lang == PDB_Lang::Cpp)
              .Case(".cc", Lang == PDB_Lang::Cpp)
              .Case(".cxx", Lang == PDB_Lang::Cpp)
              .Case(".c", Lang == PDB_Lang::C)
              .Case(".asm", Lang == PDB_Lang::Masm)
              .Case(".codira", Lang == PDB_Lang::Swift)
              .Case(".rs", Lang == PDB_Lang::Rust)
              .Case(".m", Lang == PDB_Lang::ObjC)
              .Case(".mm", Lang == PDB_Lang::ObjCpp)
              .Default(false))
        return File->getFileName();
    }
  }

  return SourceFileFullPath;
}
