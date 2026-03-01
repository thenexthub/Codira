//===-- SBScriptObject.h ----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBSCRIPTOBJECT_H
#define LLDB_API_SBSCRIPTOBJECT_H

#include "lldb/API/SBDefines.h"

namespace lldb_private {
class ScriptObject;
}

namespace lldb {

class LLDB_API SBScriptObject {
public:
  SBScriptObject(const ScriptObjectPtr ptr, lldb::ScriptLanguage lang);

  SBScriptObject(const lldb::SBScriptObject &rhs);

  ~SBScriptObject();

  const lldb::SBScriptObject &operator=(const lldb::SBScriptObject &rhs);

  explicit operator bool() const;

  bool operator!=(const SBScriptObject &rhs) const;

  bool IsValid() const;

  lldb::ScriptObjectPtr GetPointer() const;

  lldb::ScriptLanguage GetLanguage() const;

protected:
  friend class SBStructuredData;

  lldb_private::ScriptObject *get();

  lldb_private::ScriptObject &ref();

  const lldb_private::ScriptObject &ref() const;

private:
  std::unique_ptr<lldb_private::ScriptObject> m_opaque_up;
};

} // namespace lldb

#endif // LLDB_API_SBSCRIPTOBJECT_H
