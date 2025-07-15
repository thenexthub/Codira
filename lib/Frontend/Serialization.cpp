//===--- Serialization.cpp - Write Codira modules --------------------------===//
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

#include "language/Serialization/Serialization.h"
#include "language/APIDigester/ModuleAnalyzerNodes.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/AST/FileSystem.h"
#include "language/Basic/Assertions.h"
#include "language/Subsystems.h"
#include "language/SymbolGraphGen/SymbolGraphGen.h"
#include "language/SymbolGraphGen/SymbolGraphOptions.h"
#include "toolchain/Support/SmallVectorMemoryBuffer.h"
#include "toolchain/Support/VirtualOutputBackend.h"

using namespace language;

static ModuleDecl *getModule(ModuleOrSourceFile DC) {
  if (auto M = DC.dyn_cast<ModuleDecl *>())
    return M;
  return DC.get<SourceFile *>()->getParentModule();
}

static ASTContext &getContext(ModuleOrSourceFile DC) {
  return getModule(DC)->getASTContext();
}

static void emitABIDescriptor(ModuleOrSourceFile DC,
                              const SerializationOptions &options) {
  using namespace language::ide::api;
  if (!options.ABIDescriptorPath.empty()) {
    if (DC.is<ModuleDecl *>()) {
      auto &OutputBackend = getContext(DC).getOutputBackend();
      auto ABIDesFile = OutputBackend.createFile(options.ABIDescriptorPath);
      if (!ABIDesFile) {
        getContext(DC).Diags.diagnose(SourceLoc(), diag::error_opening_output,
                                      options.ABIDescriptorPath,
                                      toString(ABIDesFile.takeError()));
        return;
      }
      LANGUAGE_DEFER {
        if (auto E = ABIDesFile->keep()) {
          getContext(DC).Diags.diagnose(SourceLoc(), diag::error_closing_output,
                                        options.ABIDescriptorPath,
                                        toString(std::move(E)));
          return;
        }
      };
      dumpModuleContent(DC.get<ModuleDecl *>(), *ABIDesFile, true,
                        options.emptyABIDescriptor);
    }
  }
}

void language::serializeToBuffers(
    ModuleOrSourceFile DC, const SerializationOptions &options,
    std::unique_ptr<toolchain::MemoryBuffer> *moduleBuffer,
    std::unique_ptr<toolchain::MemoryBuffer> *moduleDocBuffer,
    std::unique_ptr<toolchain::MemoryBuffer> *moduleSourceInfoBuffer,
    const SILModule *M) {
  // Serialization output is disabled.
  if (options.OutputPath.empty())
    return;

  {
    FrontendStatsTracer tracer(getContext(DC).Stats,
                               "Serialization, languagemodule, to buffer");
    toolchain::SmallString<1024> buf;
    toolchain::raw_svector_ostream stream(buf);
    serialization::writeToStream(stream, DC, M, options,
                                 /*dependency info*/ nullptr);
    bool hadError = withOutputPath(
        getContext(DC).Diags, getContext(DC).getOutputBackend(),
        options.OutputPath, [&](raw_ostream &out) {
          out << stream.str();
          return false;
        });
    if (hadError)
      return;

    emitABIDescriptor(DC, options);
    if (moduleBuffer)
      *moduleBuffer = std::make_unique<toolchain::SmallVectorMemoryBuffer>(
          std::move(buf), options.OutputPath,
          /*RequiresNullTerminator=*/false);
  }

  if (!options.DocOutputPath.empty()) {
    FrontendStatsTracer tracer(getContext(DC).Stats,
                               "Serialization, languagedoc, to buffer");
    toolchain::SmallString<1024> buf;
    toolchain::raw_svector_ostream stream(buf);
    serialization::writeDocToStream(stream, DC, options.GroupInfoPath);
    (void)withOutputPath(getContext(DC).Diags,
                            getContext(DC).getOutputBackend(),
                            options.DocOutputPath, [&](raw_ostream &out) {
                              out << stream.str();
                              return false;
                            });
    if (moduleDocBuffer)
      *moduleDocBuffer = std::make_unique<toolchain::SmallVectorMemoryBuffer>(
          std::move(buf), options.DocOutputPath,
          /*RequiresNullTerminator=*/false);
  }

  if (!options.SourceInfoOutputPath.empty()) {
    FrontendStatsTracer tracer(getContext(DC).Stats,
                               "Serialization, languagesourceinfo, to buffer");
    toolchain::SmallString<1024> buf;
    toolchain::raw_svector_ostream stream(buf);
    serialization::writeSourceInfoToStream(stream, DC);
    (void)withOutputPath(
        getContext(DC).Diags, getContext(DC).getOutputBackend(),
        options.SourceInfoOutputPath, [&](raw_ostream &out) {
          out << stream.str();
          return false;
        });
    if (moduleSourceInfoBuffer)
      *moduleSourceInfoBuffer = std::make_unique<toolchain::SmallVectorMemoryBuffer>(
          std::move(buf), options.SourceInfoOutputPath,
          /*RequiresNullTerminator=*/false);
  }
}

void language::serialize(
    ModuleOrSourceFile DC, const SerializationOptions &options,
    const symbolgraphgen::SymbolGraphOptions &symbolGraphOptions,
    const SILModule *M,
    const fine_grained_dependencies::SourceFileDepGraph *DG) {
  assert(!options.OutputPath.empty());

  if (options.OutputPath == "-") {
    // Special-case writing to stdout.
    serialization::writeToStream(toolchain::outs(), DC, M, options, DG);
    assert(options.DocOutputPath.empty());
    return;
  }

  bool hadError = withOutputPath(
      getContext(DC).Diags, getContext(DC).getOutputBackend(),
      options.OutputPath, [&](raw_ostream &out) {
        FrontendStatsTracer tracer(getContext(DC).Stats,
                                   "Serialization, languagemodule");
        serialization::writeToStream(out, DC, M, options, DG);
        return false;
      });
  if (hadError)
    return;

  if (!options.DocOutputPath.empty()) {
    (void)withOutputPath(
        getContext(DC).Diags, getContext(DC).getOutputBackend(),
        options.DocOutputPath, [&](raw_ostream &out) {
          FrontendStatsTracer tracer(getContext(DC).Stats,
                                     "Serialization, languagedoc");
          serialization::writeDocToStream(out, DC, options.GroupInfoPath);
          return false;
        });
  }

  if (!options.SourceInfoOutputPath.empty()) {
    (void)withOutputPath(
        getContext(DC).Diags, getContext(DC).getOutputBackend(),
        options.SourceInfoOutputPath, [&](raw_ostream &out) {
          FrontendStatsTracer tracer(getContext(DC).Stats,
                                     "Serialization, languagesourceinfo");
          serialization::writeSourceInfoToStream(out, DC);
          return false;
        });
  }

  if (!symbolGraphOptions.OutputDir.empty()) {
    if (DC.is<ModuleDecl *>()) {
      auto *M = DC.get<ModuleDecl *>();
      FrontendStatsTracer tracer(getContext(DC).Stats,
                                 "Serialization, symbolgraph");
      symbolgraphgen::emitSymbolGraphForModule(M, symbolGraphOptions);
    }
  }
  emitABIDescriptor(DC, options);
}
