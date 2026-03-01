//===-- PdbAstBuilder.h -----------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_NATIVEPDB_PDBASTBUILDER_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_NATIVEPDB_PDBASTBUILDER_H

#include "lldb/Symbol/CompilerDecl.h"
#include "lldb/Symbol/CompilerDeclContext.h"
#include "lldb/Symbol/CompilerType.h"
#include "lldb/lldb-types.h"
#include "llvm/ADT/StringRef.h"

#include "PdbSymUid.h"

namespace lldb_private {
class Stream;

namespace npdb {

class PdbAstBuilder {
public:
  virtual ~PdbAstBuilder() = default;

  virtual CompilerDecl GetOrCreateDeclForUid(PdbSymUid uid) = 0;
  virtual CompilerDeclContext GetOrCreateDeclContextForUid(PdbSymUid uid) = 0;
  virtual CompilerDeclContext GetParentDeclContext(PdbSymUid uid) = 0;

  virtual void EnsureFunction(PdbCompilandSymId func_id) = 0;
  virtual void EnsureInlinedFunction(PdbCompilandSymId inlinesite_id) = 0;
  virtual void EnsureBlock(PdbCompilandSymId block_id) = 0;
  virtual void EnsureVariable(PdbCompilandSymId scope_id,
                              PdbCompilandSymId var_id) = 0;
  virtual void EnsureVariable(PdbGlobalSymId var_id) = 0;

  virtual CompilerType GetOrCreateType(PdbTypeSymId type) = 0;
  virtual CompilerType GetOrCreateTypedefType(PdbGlobalSymId id) = 0;
  virtual bool CompleteType(CompilerType ct) = 0;

  virtual void ParseDeclsForContext(CompilerDeclContext context) = 0;

  virtual CompilerDeclContext FindNamespaceDecl(CompilerDeclContext parent_ctx,
                                                llvm::StringRef name) = 0;

  virtual void Dump(Stream &stream, llvm::StringRef filter,
                    bool show_color) = 0;
};

} // namespace npdb
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_SYMBOLFILE_NATIVEPDB_PDBASTBUILDER_H
