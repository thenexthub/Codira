//===--- CodiraSyntaxMacro.cpp ---------------------------------------------===//
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

#include "CodiraLangSupport.h"
#include "language/AST/MacroDefinition.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/IDE/TypeContextInfo.h"
#include "language/IDETool/SyntacticMacroExpansion.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Comment.h"
#include "clang/AST/Decl.h"

using namespace SourceKit;
using namespace language;
using namespace ide;

void CodiraLangSupport::expandMacroSyntactically(
    toolchain::MemoryBuffer *inputBuf, ArrayRef<const char *> args,
    ArrayRef<MacroExpansionInfo> reqExpansions,
    CategorizedEditsReceiver receiver) {

  std::string error;
  auto instance = SyntacticMacroExpansions->getInstance(args, inputBuf, error);
  if (!instance) {
    return receiver(
        RequestResult<ArrayRef<CategorizedEdits>>::fromError(error));
  }
  auto &ctx = instance->getASTContext();

  // Convert 'SourceKit::MacroExpansionInfo' to 'ide::MacroExpansionSpecifier'.
  SmallVector<ide::MacroExpansionSpecifier, 4> expansions;
  for (auto &req : reqExpansions) {
    unsigned offset = req.offset;

    language::MacroRoles macroRoles;
#define MACRO_ROLE(Name, Description)                   \
    if (req.roles.contains(SourceKit::MacroRole::Name)) \
      macroRoles |= language::MacroRole::Name;
#include "language/Basic/MacroRoles.def"

    MacroDefinition definition = [&] {
      if (auto *expanded =
              std::get_if<MacroExpansionInfo::ExpandedMacroDefinition>(
                  &req.macroDefinition)) {
        SmallVector<ExpandedMacroReplacement, 2> replacements;
        for (auto &reqReplacement : expanded->replacements) {
          replacements.push_back(
              {/*startOffset=*/reqReplacement.range.Offset,
               /*endOffset=*/reqReplacement.range.Offset +
                   reqReplacement.range.Length,
               /*parameterIndex=*/reqReplacement.parameterIndex});
        }
        SmallVector<ExpandedMacroReplacement, 2> genericReplacements;
        for (auto &genReqReplacement : expanded->genericReplacements) {
          genericReplacements.push_back(
              {/*startOffset=*/genReqReplacement.range.Offset,
               /*endOffset=*/genReqReplacement.range.Offset +
                   genReqReplacement.range.Length,
               /*parameterIndex=*/genReqReplacement.parameterIndex});
        }
        return MacroDefinition::forExpanded(ctx, expanded->expansionText,
                                            replacements, genericReplacements);
      } else if (auto *externalRef =
                     std::get_if<MacroExpansionInfo::ExternalMacroReference>(
                         &req.macroDefinition)) {
        return MacroDefinition::forExternal(
            ctx.getIdentifier(externalRef->moduleName),
            ctx.getIdentifier(externalRef->typeName));
      } else {
        return MacroDefinition::forUndefined();
      }
    }();

    expansions.push_back({offset, macroRoles, definition});
  }

  RequestRefactoringEditConsumer consumer(receiver);
  instance->expandAll(expansions, consumer);
  // consumer automatically send the results on destruction.
}
