/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef LSPSERVER_ARKSERVER_H
#define LSPSERVER_ARKSERVER_H

#include <cstdint>
#include <sstream>
#include "../json-rpc/Protocol.h"
#include "../json-rpc/Transport.h"
#include "ArkScheduler.h"
#include "CompilerCodiraProject.h"
#include "common/BasicHelper.h"
#include "capabilities/semanticHighlight/SemanticHighlightImpl.h"
#include "capabilities/documentHighlight/DocumentHighlightImpl.h"
#include "capabilities/completion/CompletionImpl.h"
#include "capabilities/prepareRename/PrepareRename.h"
#include "capabilities/rename/RenameImpl.h"
#include "capabilities/definition/LocateSymbolAtImpl.h"
#include "capabilities/reference/FindReferencesImpl.h"
#include "capabilities/breakpoints/BreakpointsImpl.h"
#include "capabilities/callHierarchy/CallHierarchyImpl.h"
#include "capabilities/codeLens/CodeLensImpl.h"
#include "capabilities/overrideMethods/FindOverrideMethodsImpl.h"
#include "capabilities/refactor/Tweak.h"
#include "capabilities/fileRefactor/FileMove.h"
#include "selection/SelectionTree.h"

namespace ark {
enum class CompleteMode {
    SEMA_COMPLETE,
    PARSE_COMPLETE,
    NONE_COMPLETE
};
class ArkServer {
public:
    explicit ArkServer(Callbacks *callbacks = nullptr);

    // Get document highlights for a given position.
    void FindDocumentHighlights(const std::string &file, const TextDocumentPositionParams &params,
                                const Callback<ValueOrError> &reply) const;

    void FindTypeHierarchys(const std::string &file, const TextDocumentPositionParams &params,
                            const Callback<ValueOrError> &reply) const;

    void
    FindSuperTypes(const std::string &file, const TypeHierarchyItem &params,
                   const Callback<ValueOrError> &reply) const;

    void
    FindSubTypes(const std::string &file, const TypeHierarchyItem &params,
                 const Callback<ValueOrError> &reply) const;

    void FindCallHierarchys(const std::string &file, const TextDocumentPositionParams &params,
                            const Callback<ValueOrError> &reply) const;

    void
    FindOnIncomingCalls(const std::string &file, const CallHierarchyItem &params,
                        const Callback<ValueOrError> &reply) const;

    void
    FindOnOutgoingCalls(const std::string &file, const CallHierarchyItem &params,
                        const Callback<ValueOrError> &reply) const;

    void FindReferences(const std::string &file, const TextDocumentPositionParams &params,
                        const Callback<ValueOrError> &reply) const;

    void ApplyFileRefactor(const std::string &file,
        const std::string &selectedElement,
        const std::string &target,
        bool &isTest,
        const Callback<ValueOrError> &reply) const;

    void FindFileReferences(const std::string &file, const Callback<ValueOrError> &reply) const;

    void LocateSymbolAt(const std::string &file, const TextDocumentPositionParams &params,
                        const Callback<ValueOrError> &reply) const;

    void GetExportsName(const std::string &file, const ExportsNameParams &params,
                        const Callback<ValueOrError> &reply) const;

    void LocateRegisterCrossSymbolAt(const CrossLanguageJumpParams &params, const Callback<ValueOrError> &reply) const;

    void LocateCrossSymbolAt(const CrossLanguageJumpParams &params, const Callback<ValueOrError> &reply) const;

    // Get semantic tokens for the full doc
    void FindSemanticTokensHighlight(const std::string &file, const Callback<ValueOrError> &reply) const;

    void FindDocumentLink(const std::string &file, const Callback<ValueOrError> &reply) const;

    void FindCompletion(const CompletionParams &params, const std::string &file,
                        const Callback <ValueOrError> &reply) const;

    void PrepareRename(const std::string &file,
                       const TextDocumentPositionParams &params, const Callback<ValueOrError> &reply) const;

    void Rename(const std::string &file, const RenameParams &params,
                const Callback <ValueOrError> &reply) const;

    void FindBreakpoints(const std::string &file, const Callback <ValueOrError> &reply) const;

    void FindSignatureHelp(const SignatureHelpParams &params, const std::string &file,
                           const Callback<ValueOrError> &reply) const;

    void FindCodeLens(const std::string &file, const Callback <ValueOrError> &reply) const;

    void FindWorkspaceSymbols(const std::string &query, const Callback<ValueOrError> &reply) const;

    // Get hover for a given position.
    void FindHover(const std::string &file,
                   const TextDocumentPositionParams &params, const Callback<ValueOrError> &reply) const;

    void ChangeWatchedFiles(const std::string &file, FileChangeType type, DocCache *docMgr) const;

    void AddDoc(const std::string &file, const std::string &contents, int64_t version, NeedDiagnostics needDiagnostics,
                bool forceRebuild) const;

    void FindDocumentSymbol(const DocumentSymbolParams &params, const Callback<ValueOrError> &reply) const;

    Codira::Position AlterPosition(const std::string &file, const TextDocumentPositionParams &params) const;

    void UpdateModifierDiag(const InputsAndAST &inputAST,
        std::vector<DiagnosticToken> &diagnostics, const std::string &file) const;

    void FindOverrideMethods(const std::string &file,
                                       const OverrideMethodsParams &params, const Callback<ValueOrError> &reply) const;

    struct TweakRef {
        std::string id;    // ID to pass for applyTweak.
        std::string title; // A single-line message to show in the UI.
        std::string kind;
        std::map<std::string, std::string> extraOptions;
    };

    void EnumerateTweaks(const std::string &file, Range range, const Callback<std::vector<TweakRef>> &cb) const;

    void ApplyTweak(const std::string &file, Range selection, const std::string &id,
        std::map<std::string, std::string> extraOptions, const Callback<Tweak::Effect> &cb);

private:
    Callbacks *callback = nullptr;
    std::unique_ptr<ark::ArkScheduler> arkScheduler;
    std::unique_ptr<ark::ArkScheduler> arkSchedulerOfComplete;
    std::unique_ptr<ark::ArkScheduler> arkSchedulerOfSignature;
};
} // namespace ark


#endif // LSPSERVER_ARKSERVER_H
