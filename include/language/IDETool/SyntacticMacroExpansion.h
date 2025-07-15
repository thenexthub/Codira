//===--- SyntacticMacroExpansion.h ----------------------------------------===//
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

#ifndef LANGUAGE_IDE_SYNTACTICMACROEXPANSION_H
#define LANGUAGE_IDE_SYNTACTICMACROEXPANSION_H

#include "language/AST/Decl.h"
#include "language/AST/MacroDefinition.h"
#include "language/AST/PluginRegistry.h"
#include "language/Basic/Fingerprint.h"
#include "language/Frontend/Frontend.h"
#include "toolchain/Support/MemoryBuffer.h"

namespace language {

class ASTContext;
class SourceFile;

namespace ide {
class SourceEditConsumer;

/// Simple object to specify a syntactic macro expansion.
struct MacroExpansionSpecifier {
  unsigned offset;
  language::MacroRoles macroRoles;
  language::MacroDefinition macroDefinition;
};

/// Instance of a syntactic macro expansion context. This is created for each
/// list of compiler arguments (i.e. 'argHash'), and reused as long as the
/// compiler arguments are not changed.
class SyntacticMacroExpansionInstance {
  CompilerInvocation invocation;

  SourceManager SourceMgr;
  DiagnosticEngine Diags{SourceMgr};
  std::unique_ptr<ASTContext> Ctx;
  ModuleDecl *TheModule = nullptr;
  SourceFile *SF = nullptr;
  toolchain::StringMap<MacroDecl *> MacroDecls;

  /// Synthesize 'MacroDecl' AST object to use the expansion.
  language::MacroDecl *
  getSynthesizedMacroDecl(language::Identifier name,
                          const MacroExpansionSpecifier &expansion);

  /// Expand single 'expansion'.
  void expand(const MacroExpansionSpecifier &expansion,
              SourceEditConsumer &consumer);

public:
  SyntacticMacroExpansionInstance() {}

  /// Setup the instance with \p args and a given \p inputBuf.
  bool setup(StringRef CodiraExecutablePath, ArrayRef<const char *> args,
             toolchain::MemoryBuffer *inputBuf,
             std::shared_ptr<PluginRegistry> plugins, std::string &error);

  ASTContext &getASTContext() { return *Ctx; }

  /// Expand all macros and send the edit results to \p consumer. Expansions are
  /// specified by \p expansions .
  void expandAll(ArrayRef<MacroExpansionSpecifier> expansions,
                 SourceEditConsumer &consumer);
};

/// Manager object to vend 'SyntacticMacroExpansionInstance'.
class SyntacticMacroExpansion {
  StringRef CodiraExecutablePath;
  std::shared_ptr<PluginRegistry> Plugins;

public:
  SyntacticMacroExpansion(StringRef CodiraExecutablePath,
                          std::shared_ptr<PluginRegistry> Plugins)
      : CodiraExecutablePath(CodiraExecutablePath), Plugins(Plugins) {}

  /// Get instance configured with the specified compiler arguments and
  /// input buffer.
  std::shared_ptr<SyntacticMacroExpansionInstance>
  getInstance(ArrayRef<const char *> args, toolchain::MemoryBuffer *inputBuf,
              std::string &error);
};

} // namespace ide
} // namespace language

#endif // LANGUAGE_IDE_SYNTACTICMACROEXPANSION_H
