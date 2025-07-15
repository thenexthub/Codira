//===--- ImportDepth.cpp --------------------------------------------------===//
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

#include "language/IDE/ImportDepth.h"
#include "language/AST/Module.h"
#include "language/Basic/Assertions.h"
#include "clang/Basic/Module.h"

#include <deque>

using namespace language;
using namespace language::ide;

ImportDepth::ImportDepth(ASTContext &context,
                         const FrontendOptions &frontendOptions) {
  toolchain::DenseSet<ModuleDecl *> seen;
  std::deque<std::pair<ModuleDecl *, uint8_t>> worklist;

  StringRef mainModule = frontendOptions.ModuleName;
  auto *main = context.getLoadedModule(context.getIdentifier(mainModule));
  assert(main && "missing main module");
  worklist.emplace_back(main, uint8_t(0));

  // Imports from -import-name such as Playground auxiliary sources are treated
  // specially by applying import depth 0.
  toolchain::StringSet<> auxImports;
  for (const auto &pair : frontendOptions.getImplicitImportModuleNames())
    auxImports.insert(pair.first);

  // Private imports from this module.
  SmallVector<ImportedModule, 16> mainImports;
  main->getImportedModules(mainImports, ModuleDecl::getImportFilterLocal());
  for (auto &import : mainImports) {
    uint8_t depth = 1;
    if (auxImports.count(import.importedModule->getName().str()))
      depth = 0;
    worklist.emplace_back(import.importedModule, depth);
  }

  // Fill depths with BFS over module imports.
  while (!worklist.empty()) {
    ModuleDecl *module;
    uint8_t depth;
    std::tie(module, depth) = worklist.front();
    worklist.pop_front();

    if (!seen.insert(module).second)
      continue;

    // Insert new module:depth mapping.
    const clang::Module *CM = module->findUnderlyingClangModule();
    if (CM) {
      depths[CM->getFullModuleName()] = depth;
    } else {
      depths[module->getName().str()] = depth;
    }

    // Add imports to the worklist.
    SmallVector<ImportedModule, 16> imports;
    module->getImportedModules(imports, ModuleDecl::ImportFilterKind::Exported);
    for (auto &import : imports) {
      uint8_t next = std::max(depth, uint8_t(depth + 1)); // unsigned wrap

      // Implicitly imported sub-modules get the same depth as their parent.
      if (const clang::Module *CMI =
              import.importedModule->findUnderlyingClangModule())
        if (CM && CMI->isSubModuleOf(CM))
          next = depth;
      worklist.emplace_back(import.importedModule, next);
    }
  }
}
