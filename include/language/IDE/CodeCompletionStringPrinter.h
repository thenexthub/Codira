//===--- CodeCompletionStringPrinter.h ------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IDE_CODECOMPLETIONRESULTSTRINGPRINTER_H
#define LANGUAGE_IDE_CODECOMPLETIONRESULTSTRINGPRINTER_H

#include "language/AST/ASTPrinter.h"
#include "language/IDE/CodeCompletionString.h"

namespace language {
namespace ide {

/// 'ASTPrinter' printing to 'CodeCompletionString' with appropriate ChunkKind.
/// This is mainly used for printing types and override completions.
class CodeCompletionStringPrinter : public ASTPrinter {
protected:
  using ChunkKind = CodeCompletionString::Chunk::ChunkKind;

private:
  CodeCompletionResultBuilder &Builder;
  SmallString<16> Buffer;
  ChunkKind CurrChunkKind = ChunkKind::Text;
  ChunkKind NextChunkKind = ChunkKind::Text;
  SmallVector<PrintStructureKind, 2> StructureStack;
  unsigned int TypeDepth = 0;
  bool InPreamble = false;

  bool isCurrentStructureKind(PrintStructureKind Kind) const {
    return !StructureStack.empty() && StructureStack.back() == Kind;
  }

  bool isInType() const { return TypeDepth > 0; }

  std::optional<ChunkKind>
  getChunkKindForPrintNameContext(PrintNameContext context) const;

  std::optional<ChunkKind>
  getChunkKindForStructureKind(PrintStructureKind Kind) const;

  void startNestedGroup(ChunkKind Kind);

  void endNestedGroup();

protected:
  void setNextChunkKind(ChunkKind Kind) { NextChunkKind = Kind; }

public:
  CodeCompletionStringPrinter(CodeCompletionResultBuilder &Builder)
      : Builder(Builder) {}

  ~CodeCompletionStringPrinter() {
    // Flush the remainings.
    flush();
  }

  void flush();

  /// Start \c AttributeAndModifierListBegin group. This must be called before
  /// any attributes/modifiers printed to the output when printing an override
  /// compleion.
  void startPreamble();

  /// End the current \c AttributeAndModifierListBegin group if it's still open.
  /// This is automatically called before the main part of the signature.
  void endPremable();

  /// Implement \c ASTPrinter .
public:
  void printText(StringRef Text) override;

  void printTypeRef(
      Type T, const TypeDecl *TD, Identifier Name,
      PrintNameContext NameContext = PrintNameContext::Normal) override;

  void printDeclLoc(const Decl *D) override;

  void printDeclNameEndLoc(const Decl *D) override;

  void printNamePre(PrintNameContext context) override;

  void printNamePost(PrintNameContext context) override;

  void printTypePre(const TypeLoc &TL) override;

  void printTypePost(const TypeLoc &TL) override;

  void printStructurePre(PrintStructureKind Kind, const Decl *D) override;

  void printStructurePost(PrintStructureKind Kind, const Decl *D) override;
};

} // end namespace ide
} // end namespace language

#endif // LANGUAGE_IDE_CODECOMPLETIONRESULTSTRINGPRINTER_H
