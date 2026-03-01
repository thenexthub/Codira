//===-- AppleObjCTypeEncodingParser.h ---------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_LANGUAGERUNTIME_OBJC_APPLEOBJCRUNTIME_APPLEOBJCTYPEENCODINGPARSER_H
#define LLDB_SOURCE_PLUGINS_LANGUAGERUNTIME_OBJC_APPLEOBJCRUNTIME_APPLEOBJCTYPEENCODINGPARSER_H

#include "Plugins/Language/ObjC/ObjCConstants.h"
#include "Plugins/LanguageRuntime/ObjC/ObjCLanguageRuntime.h"
#include "lldb/lldb-private.h"

#include "clang/AST/ASTContext.h"

namespace lldb_private {
class AppleObjCTypeEncodingParser : public ObjCLanguageRuntime::EncodingToType {
public:
  AppleObjCTypeEncodingParser(ObjCLanguageRuntime &runtime);
  ~AppleObjCTypeEncodingParser() override = default;

  CompilerType RealizeType(TypeSystemClang &ast_ctx, const char *name,
                           bool for_expression) override;

private:
  struct StructElement {
    std::string name;
    clang::QualType type;
    uint32_t bitfield = 0;

    StructElement();
    ~StructElement() = default;
  };

  clang::QualType BuildType(TypeSystemClang &clang_ast_ctx,
                            llvm::StringRef &type, bool for_expression,
                            uint32_t *bitfield_bit_size = nullptr);

  clang::QualType BuildStruct(TypeSystemClang &ast_ctx, llvm::StringRef &type,
                              bool for_expression);

  clang::QualType BuildAggregate(TypeSystemClang &clang_ast_ctx,
                                 llvm::StringRef &type, bool for_expression,
                                 char opener, char closer, uint32_t kind);

  clang::QualType BuildUnion(TypeSystemClang &ast_ctx, llvm::StringRef &type,
                             bool for_expression);

  clang::QualType BuildArray(TypeSystemClang &ast_ctx, llvm::StringRef &type,
                             bool for_expression);

  std::string ReadStructName(llvm::StringRef &type);

  StructElement ReadStructElement(TypeSystemClang &ast_ctx,
                                  llvm::StringRef &type, bool for_expression);

  clang::QualType BuildObjCObjectPointerType(TypeSystemClang &clang_ast_ctx,
                                             llvm::StringRef &type,
                                             bool for_expression);

  uint32_t ReadNumber(llvm::StringRef &type);

  std::optional<std::string> ReadQuotedString(llvm::StringRef &type);

  ObjCLanguageRuntime &m_runtime;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_LANGUAGERUNTIME_OBJC_APPLEOBJCRUNTIME_APPLEOBJCTYPEENCODINGPARSER_H
