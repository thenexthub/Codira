//===--- HTMLPrint.cpp - Source code -> HTML pretty-printing --------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
//
// Pretty-printing of source code to HTML.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/ASTConsumer.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/SourceManager.h"
#include "language/Core/Lex/Preprocessor.h"
#include "language/Core/Rewrite/Core/HTMLRewrite.h"
#include "language/Core/Rewrite/Core/Rewriter.h"
#include "language/Core/Rewrite/Frontend/ASTConsumers.h"
#include "toolchain/ADT/RewriteBuffer.h"
#include "toolchain/Support/raw_ostream.h"
using namespace language::Core;
using toolchain::RewriteBuffer;

//===----------------------------------------------------------------------===//
// Functional HTML pretty-printing.
//===----------------------------------------------------------------------===//

namespace {
  class HTMLPrinter : public ASTConsumer {
    Rewriter R;
    std::unique_ptr<raw_ostream> Out;
    Preprocessor &PP;
    bool SyntaxHighlight, HighlightMacros;

  public:
    HTMLPrinter(std::unique_ptr<raw_ostream> OS, Preprocessor &pp,
                bool _SyntaxHighlight, bool _HighlightMacros)
      : Out(std::move(OS)), PP(pp), SyntaxHighlight(_SyntaxHighlight),
        HighlightMacros(_HighlightMacros) {}

    void Initialize(ASTContext &context) override;
    void HandleTranslationUnit(ASTContext &Ctx) override;
  };
}

std::unique_ptr<ASTConsumer>
language::Core::CreateHTMLPrinter(std::unique_ptr<raw_ostream> OS, Preprocessor &PP,
                         bool SyntaxHighlight, bool HighlightMacros) {
  return std::make_unique<HTMLPrinter>(std::move(OS), PP, SyntaxHighlight,
                                        HighlightMacros);
}

void HTMLPrinter::Initialize(ASTContext &context) {
  R.setSourceMgr(context.getSourceManager(), context.getLangOpts());
}

void HTMLPrinter::HandleTranslationUnit(ASTContext &Ctx) {
  if (PP.getDiagnostics().hasErrorOccurred())
    return;

  // Format the file.
  FileID FID = R.getSourceMgr().getMainFileID();
  OptionalFileEntryRef Entry = R.getSourceMgr().getFileEntryRefForID(FID);
  StringRef Name;
  // In some cases, in particular the case where the input is from stdin,
  // there is no entry.  Fall back to the memory buffer for a name in those
  // cases.
  if (Entry)
    Name = Entry->getName();
  else
    Name = R.getSourceMgr().getBufferOrFake(FID).getBufferIdentifier();

  html::AddLineNumbers(R, FID);
  html::AddHeaderFooterInternalBuiltinCSS(R, FID, Name);

  // If we have a preprocessor, relex the file and syntax highlight.
  // We might not have a preprocessor if we come from a deserialized AST file,
  // for example.

  if (SyntaxHighlight) html::SyntaxHighlight(R, FID, PP);
  if (HighlightMacros) html::HighlightMacros(R, FID, PP);
  html::EscapeText(R, FID, false, true);

  // Emit the HTML.
  const RewriteBuffer &RewriteBuf = R.getEditBuffer(FID);
  std::unique_ptr<char[]> Buffer(new char[RewriteBuf.size()]);
  std::copy(RewriteBuf.begin(), RewriteBuf.end(), Buffer.get());
  Out->write(Buffer.get(), RewriteBuf.size());
}
