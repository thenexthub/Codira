//===-- FileLineResolver.h --------------------------------------*- C++ -*-===//
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

#ifndef LLDB_CORE_FILELINERESOLVER_H
#define LLDB_CORE_FILELINERESOLVER_H

#include "lldb/Core/SearchFilter.h"
#include "lldb/Symbol/SymbolContext.h"
#include "lldb/Utility/FileSpec.h"
#include "lldb/lldb-defines.h"

#include <cstdint>

namespace lldb_private {
class Address;
class Stream;

/// \class FileLineResolver FileLineResolver.h "lldb/Core/FileLineResolver.h"
/// This class finds address for source file and line.  Optionally, it will
/// look for inlined instances of the file and line specification.

class FileLineResolver : public Searcher {
public:
  FileLineResolver()
      : m_file_spec(),
        // Set this to zero for all lines in a file
        m_sc_list() {}

  FileLineResolver(const FileSpec &resolver, uint32_t line_no,
                   bool check_inlines);

  ~FileLineResolver() override;

  Searcher::CallbackReturn SearchCallback(SearchFilter &filter,
                                          SymbolContext &context,
                                          Address *addr) override;

  lldb::SearchDepth GetDepth() override;

  void GetDescription(Stream *s) override;

  const SymbolContextList &GetFileLineMatches() { return m_sc_list; }

  void Clear();

  void Reset(const FileSpec &file_spec, uint32_t line, bool check_inlines);

protected:
  FileSpec m_file_spec;   // This is the file spec we are looking for.
  uint32_t m_line_number =
      UINT32_MAX; // This is the line number that we are looking for.
  SymbolContextList m_sc_list;
  bool m_inlines = true; // This determines whether the resolver looks for
                         // inlined functions or not.

private:
  FileLineResolver(const FileLineResolver &) = delete;
  const FileLineResolver &operator=(const FileLineResolver &) = delete;
};

} // namespace lldb_private

#endif // LLDB_CORE_FILELINERESOLVER_H
