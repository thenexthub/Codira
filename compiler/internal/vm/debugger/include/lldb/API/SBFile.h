//===-- SBFile.h --------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBFILE_H
#define LLDB_API_SBFILE_H

#include "lldb/API/SBDefines.h"

#include <cstdio>

namespace lldb {

class LLDB_API SBFile {
  friend class SBInstruction;
  friend class SBInstructionList;
  friend class SBDebugger;
  friend class SBCommandReturnObject;
  friend class SBProcess;

public:
  SBFile();
  SBFile(FileSP file_sp);
#ifndef SWIG
  SBFile(const SBFile &rhs);
  LLDB_DEPRECATED_FIXME("Use the constructor that specifies mode instead",
                        "SBFile(FILE*, const char*, bool)")
  SBFile(FILE *file, bool transfer_ownership);
  SBFile(FILE *file, const char *mode, bool transfer_ownership);
#endif
  SBFile(int fd, const char *mode, bool transfer_ownership);
  ~SBFile();

  SBFile &operator=(const SBFile &rhs);

  SBError Read(uint8_t *buf, size_t num_bytes, size_t *OUTPUT);
  SBError Write(const uint8_t *buf, size_t num_bytes, size_t *OUTPUT);
  SBError Flush();
  bool IsValid() const;
  SBError Close();

  operator bool() const;
#ifndef SWIG
  bool operator!() const;
#endif

  FileSP GetFile() const;

private:
  FileSP m_opaque_sp;
};

} // namespace lldb

#endif // LLDB_API_SBFILE_H
