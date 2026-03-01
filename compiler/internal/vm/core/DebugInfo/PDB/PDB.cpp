//===- PDB.cpp - base header file for creating a PDB reader ---------------===//
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

#include "vm/core/DebugInfo/PDB/PDB.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Config/config.h"
#include "vm/core/DebugInfo/PDB/GenericError.h"
#if LLVM_ENABLE_DIA_SDK
#include "vm/core/DebugInfo/PDB/DIA/DIASession.h"
#endif
#include "vm/core/DebugInfo/PDB/Native/NativeSession.h"
#include "vm/core/Support/Error.h"

using namespace vm::core;
using namespace vm::core::pdb;

Error toolchain::pdb::loadDataForPDB(PDB_ReaderType Type, StringRef Path,
                                std::unique_ptr<IPDBSession> &Session) {
  // Create the correct concrete instance type based on the value of Type.
  if (Type == PDB_ReaderType::Native)
    return NativeSession::createFromPdbPath(Path, Session);

#if LLVM_ENABLE_DIA_SDK
  return DIASession::createFromPdb(Path, Session);
#else
  return make_error<PDBError>(pdb_error_code::dia_sdk_not_present);
#endif
}

Error toolchain::pdb::loadDataForEXE(PDB_ReaderType Type, StringRef Path,
                                std::unique_ptr<IPDBSession> &Session) {
  // Create the correct concrete instance type based on the value of Type.
  if (Type == PDB_ReaderType::Native) {
    Expected<std::string> PdbPath = NativeSession::searchForPdb({Path});
    if (!PdbPath)
      return PdbPath.takeError();
    return NativeSession::createFromPdbPath(PdbPath.get(), Session);
  }

#if LLVM_ENABLE_DIA_SDK
  return DIASession::createFromExe(Path, Session);
#else
  return make_error<PDBError>(pdb_error_code::dia_sdk_not_present);
#endif
}
