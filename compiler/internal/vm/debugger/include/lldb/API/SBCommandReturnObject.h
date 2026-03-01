//===-- SBCommandReturnObject.h ---------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBCOMMANDRETURNOBJECT_H
#define LLDB_API_SBCOMMANDRETURNOBJECT_H

#include <cstdio>

#include <memory>

#include "lldb/API/SBDefines.h"

namespace lldb_private {
class CommandPluginInterfaceImplementation;
class SBCommandReturnObjectImpl;
namespace python {
class SWIGBridge;
}
} // namespace lldb_private

namespace lldb {

class LLDB_API SBCommandReturnObject {
public:
  SBCommandReturnObject();

  // rvalue ctor+assignment are incompatible with Reproducers.

  SBCommandReturnObject(const lldb::SBCommandReturnObject &rhs);

  ~SBCommandReturnObject();

  lldb::SBCommandReturnObject &
  operator=(const lldb::SBCommandReturnObject &rhs);

  explicit operator bool() const;

  bool IsValid() const;

  /// Get the command as the user typed it. Empty string if commands were run on
  /// behalf of lldb.
  const char *GetCommand();

  const char *GetOutput();

  const char *GetError();
  SBStructuredData GetErrorData();

#ifndef SWIG
  LLDB_DEPRECATED_FIXME("Use PutOutput(SBFile) or PutOutput(FileSP)",
                        "PutOutput(SBFile)")
  size_t PutOutput(FILE *fh);
#endif

  size_t PutOutput(SBFile file);

  size_t PutOutput(FileSP BORROWED);

  size_t GetOutputSize();

  size_t GetErrorSize();

#ifndef SWIG
  LLDB_DEPRECATED_FIXME("Use PutError(SBFile) or PutError(FileSP)",
                        "PutError(SBFile)")
  size_t PutError(FILE *fh);
#endif

  size_t PutError(SBFile file);

  size_t PutError(FileSP BORROWED);

  void Clear();

  lldb::ReturnStatus GetStatus();

  void SetStatus(lldb::ReturnStatus status);

  bool Succeeded();

  bool HasResult();

  void AppendMessage(const char *message);

  void AppendWarning(const char *message);

  bool GetDescription(lldb::SBStream &description);

#ifndef SWIG
  LLDB_DEPRECATED_FIXME(
      "Use SetImmediateOutputFile(SBFile) or SetImmediateOutputFile(FileSP)",
      "SetImmediateOutputFile(SBFile)")
  void SetImmediateOutputFile(FILE *fh);

  LLDB_DEPRECATED_FIXME(
      "Use SetImmediateErrorFile(SBFile) or SetImmediateErrorFile(FileSP)",
      "SetImmediateErrorFile(SBFile)")
  void SetImmediateErrorFile(FILE *fh);

  LLDB_DEPRECATED_FIXME(
      "Use SetImmediateOutputFile(SBFile) or SetImmediateOutputFile(FileSP)",
      "SetImmediateOutputFile(SBFile)")
  void SetImmediateOutputFile(FILE *fh, bool transfer_ownership);

  LLDB_DEPRECATED_FIXME(
      "Use SetImmediateErrorFile(SBFile) or SetImmediateErrorFile(FileSP)",
      "SetImmediateErrorFile(SBFile)")
  void SetImmediateErrorFile(FILE *fh, bool transfer_ownership);
#endif

  void SetImmediateOutputFile(SBFile file);

  void SetImmediateErrorFile(SBFile file);

  void SetImmediateOutputFile(FileSP BORROWED);

  void SetImmediateErrorFile(FileSP BORROWED);

  void PutCString(const char *string, int len = -1);

#ifndef SWIG
  __attribute__((format(printf, 2, 3))) size_t Printf(const char *format, ...);
#endif

  const char *GetOutput(bool only_if_no_immediate);

  const char *GetError(bool only_if_no_immediate);

  void SetError(lldb::SBError &error,
                const char *fallback_error_cstr = nullptr);

  void SetError(const char *error_cstr);

  lldb::SBValueList GetValues(lldb::DynamicValueType use_dynamic);

protected:
  friend class SBCommandInterpreter;
  friend class SBOptions;

  friend class lldb_private::CommandPluginInterfaceImplementation;
  friend class lldb_private::python::SWIGBridge;

  SBCommandReturnObject(lldb_private::CommandReturnObject &ref);

  lldb_private::CommandReturnObject *operator->() const;

  lldb_private::CommandReturnObject *get() const;

  lldb_private::CommandReturnObject &operator*() const;

private:
  lldb_private::CommandReturnObject &ref() const;

  std::unique_ptr<lldb_private::SBCommandReturnObjectImpl> m_opaque_up;
};

} // namespace lldb

#endif // LLDB_API_SBCOMMANDRETURNOBJECT_H
