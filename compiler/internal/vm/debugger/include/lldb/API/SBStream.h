//===-- SBStream.h ----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBSTREAM_H
#define LLDB_API_SBSTREAM_H

#include <cstdio>

#include "lldb/API/SBDefines.h"

namespace lldb_private {
class ScriptInterpreter;
} // namespace lldb_private

namespace lldb {

class LLDB_API SBStream {
public:
  SBStream();

#ifndef SWIG
  SBStream(SBStream &&rhs);
#endif

  ~SBStream();

  explicit operator bool() const;

  bool IsValid() const;

  // If this stream is not redirected to a file, it will maintain a local cache
  // for the stream data which can be accessed using this accessor.
  const char *GetData();

  // If this stream is not redirected to a file, it will maintain a local cache
  // for the stream output whose length can be accessed using this accessor.
  size_t GetSize();

#ifndef SWIG
  __attribute__((format(printf, 2, 3))) void Printf(const char *format, ...);
#endif

  void Print(const char *str);

  void RedirectToFile(const char *path, bool append);

  void RedirectToFile(lldb::SBFile file);

  void RedirectToFile(lldb::FileSP file);

#ifndef SWIG
  void RedirectToFileHandle(FILE *fh, bool transfer_fh_ownership);
#endif

  void RedirectToFileDescriptor(int fd, bool transfer_fh_ownership);

  // If the stream is redirected to a file, forget about the file and if
  // ownership of the file was transferred to this object, close the file. If
  // the stream is backed by a local cache, clear this cache.
  void Clear();

protected:
  friend class SBAddress;
  friend class SBAddressRange;
  friend class SBAddressRangeList;
  friend class SBBlock;
  friend class SBBreakpoint;
  friend class SBBreakpointLocation;
  friend class SBBreakpointName;
  friend class SBCommandReturnObject;
  friend class SBCompileUnit;
  friend class SBData;
  friend class SBDebugger;
  friend class SBDeclaration;
  friend class SBEvent;
  friend class SBFileSpec;
  friend class SBFileSpecList;
  friend class SBFrame;
  friend class SBFrameList;
  friend class SBFunction;
  friend class SBInstruction;
  friend class SBInstructionList;
  friend class SBLaunchInfo;
  friend class SBLineEntry;
  friend class SBMemoryRegionInfo;
  friend class SBModule;
  friend class SBModuleSpec;
  friend class SBModuleSpecList;
  friend class SBProcess;
  friend class SBSection;
  friend class SBSourceManager;
  friend class SBStructuredData;
  friend class SBSymbol;
  friend class SBSymbolContext;
  friend class SBSymbolContextList;
  friend class SBTarget;
  friend class SBThread;
  friend class SBThreadPlan;
  friend class SBType;
  friend class SBTypeEnumMember;
  friend class SBTypeMemberFunction;
  friend class SBTypeMember;
  friend class SBValue;
  friend class SBWatchpoint;

  friend class lldb_private::ScriptInterpreter;

  lldb_private::Stream *operator->();

  lldb_private::Stream *get();

  lldb_private::Stream &ref();

private:
  SBStream(const SBStream &) = delete;
  const SBStream &operator=(const SBStream &) = delete;
  std::unique_ptr<lldb_private::Stream> m_opaque_up;
  bool m_is_file = false;
};

} // namespace lldb

#endif // LLDB_API_SBSTREAM_H
