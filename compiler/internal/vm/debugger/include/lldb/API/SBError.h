//===-- SBError.h -----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBERROR_H
#define LLDB_API_SBERROR_H

#include "lldb/API/SBDefines.h"

namespace lldb_private {
class ScriptInterpreter;
namespace python {
class SWIGBridge;
}
} // namespace lldb_private

namespace lldb {

class LLDB_API SBError {
public:
  SBError();

  SBError(const lldb::SBError &rhs);

  SBError(const char *message);

  ~SBError();

  const SBError &operator=(const lldb::SBError &rhs);

  /// Get the error string as a NULL terminated UTF8 c-string.
  ///
  /// This SBError object owns the returned string and this object must be kept
  /// around long enough to use the returned string.
  const char *GetCString() const;

  void Clear();

  bool Fail() const;

  bool Success() const;

  /// Get the error code.
  uint32_t GetError() const;

  /// Get the error in machine-readable form. Particularly useful for
  /// compiler diagnostics.
  SBStructuredData GetErrorData() const;

  lldb::ErrorType GetType() const;

  void SetError(uint32_t err, lldb::ErrorType type);

  void SetErrorToErrno();

  void SetErrorToGenericError();

  void SetErrorString(const char *err_str);

#ifndef SWIG
  __attribute__((format(printf, 2, 3)))
#else
  // clang-format off
  %varargs(3, char *str = NULL) SetErrorStringWithFormat;
  // clang-format on
#endif
  int SetErrorStringWithFormat(const char *format, ...);

  explicit operator bool() const;

  /// \brief Returns \c true if this object contains an underlying \c Status
  /// object.
  ///
  /// That object may represent a success or a failure. When \c IsValid returns
  /// \c false, it may be the case that the \c SBError represents a success but
  /// does not contain a \c Status representing that success.
  ///
  /// It is safe to call \c Success or \c Fail in the case where \c IsValid
  /// returns \c false.
  bool IsValid() const;

  bool GetDescription(lldb::SBStream &description);

protected:
  friend class SBBreakpoint;
  friend class SBBreakpointLocation;
  friend class SBBreakpointName;
  friend class SBCommandReturnObject;
  friend class SBCommunication;
  friend class SBSaveCoreOptions;
  friend class SBData;
  friend class SBDebugger;
  friend class SBFile;
  friend class SBFormat;
  friend class SBFrame;
  friend class SBHostOS;
  friend class SBPlatform;
  friend class SBProcess;
  friend class SBReproducer;
  friend class SBStructuredData;
  friend class SBTarget;
  friend class SBThread;
  friend class SBTrace;
  friend class SBValue;
  friend class SBValueList;
  friend class SBWatchpoint;

  friend class lldb_private::ScriptInterpreter;
  friend class lldb_private::python::SWIGBridge;

  SBError(lldb_private::Status &&error);

  lldb_private::Status *get();

  lldb_private::Status *operator->();

  const lldb_private::Status &operator*() const;

  lldb_private::Status &ref();

  void SetError(lldb_private::Status &&lldb_error);

private:
  std::unique_ptr<lldb_private::Status> m_opaque_up;

  void CreateIfNeeded();
};

} // namespace lldb

#endif // LLDB_API_SBERROR_H
