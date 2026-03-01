//===-- SBFormat.h ----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBFORMAT_H
#define LLDB_API_SBFORMAT_H

#include "lldb/API/SBDefines.h"

namespace lldb_private {
namespace python {
class SWIGBridge;
} // namespace python
namespace lua {
class SWIGBridge;
} // namespace lua
} // namespace lldb_private

namespace lldb {

/// Class that represents a format string that can be used to generate
/// descriptions of objects like frames and threads. See
/// https://lldb.llvm.org/use/formatting.html for more information.
class LLDB_API SBFormat {
public:
  SBFormat();

  /// Create an \a SBFormat by parsing the given format string. If parsing
  /// fails, this object is initialized as invalid.
  ///
  /// \param[in] format
  ///   The format string to parse.
  ///
  /// \param[out] error
  ///   An object where error messages will be written to if parsing fails.
  SBFormat(const char *format, lldb::SBError &error);

  SBFormat(const lldb::SBFormat &rhs);

  lldb::SBFormat &operator=(const lldb::SBFormat &rhs);

  ~SBFormat();

  /// \return
  ///   \b true if and only if this object is valid and can be used for
  ///   formatting.
  explicit operator bool() const;

protected:
  friend class SBFrame;
  friend class SBThread;

  /// \return
  ///   The underlying shared pointer storage for this object.
  lldb::FormatEntrySP GetFormatEntrySP() const;

  /// The storage for this object.
  lldb::FormatEntrySP m_opaque_sp;
};

} // namespace lldb
#endif // LLDB_API_SBFORMAT_H
