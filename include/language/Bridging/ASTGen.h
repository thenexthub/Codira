//===--- ASTGen.h -----------------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_BRIDGING_ASTGEN_H
#define LANGUAGE_BRIDGING_ASTGEN_H

#include "language/AST/ASTBridging.h"

#ifdef __cplusplus
extern "C" {
#endif

void *_Nonnull language_ASTGen_createQueuedDiagnostics();
void language_ASTGen_destroyQueuedDiagnostics(void *_Nonnull queued);
void language_ASTGen_addQueuedSourceFile(
    void *_Nonnull queuedDiagnostics, ssize_t bufferID,
    void *_Nonnull sourceFile, const uint8_t *_Nonnull displayNamePtr,
    intptr_t displayNameLength, ssize_t parentID, ssize_t positionInParent);
void language_ASTGen_addQueuedDiagnostic(
    void *_Nonnull queued, void *_Nonnull state,
    BridgedStringRef text,
    language::DiagnosticKind severity,
    BridgedSourceLoc sourceLoc,
    BridgedStringRef categoryName,
    BridgedStringRef documentationPath,
    const BridgedCharSourceRange *_Nullable highlightRanges,
    ptrdiff_t numHighlightRanges,
    BridgedArrayRef /*BridgedFixIt*/ fixIts);
void language_ASTGen_renderSingleDiagnostic(
    void *_Nonnull state,
    BridgedStringRef text,
    language::DiagnosticKind severity,
    BridgedStringRef categoryName,
    BridgedStringRef documentationPath,
    ssize_t colorize,
    BridgedStringRef *_Nonnull renderedString);
void language_ASTGen_renderQueuedDiagnostics(
    void *_Nonnull queued, ssize_t contextSize, ssize_t colorize,
    BridgedStringRef *_Nonnull renderedString);

void *_Nonnull language_ASTGen_createPerFrontendDiagnosticState();
void language_ASTGen_destroyPerFrontendDiagnosticState(void * _Nonnull state);
void language_ASTGen_renderCategoryFootnotes(
    void * _Nonnull state, ssize_t colorize,
    BridgedStringRef *_Nonnull renderedString);

// FIXME: Hack because we cannot easily get to the already-parsed source
// file from here. Fix this egregious oversight!
void *_Nullable language_ASTGen_parseSourceFile(BridgedStringRef buffer,
                                             BridgedStringRef moduleName,
                                             BridgedStringRef filename,
                                             void *_Nullable declContextPtr,
                                             BridgedGeneratedSourceFileKind);
void language_ASTGen_destroySourceFile(void *_Nonnull sourceFile);

/// Check whether the given source file round-trips correctly. Returns 0 if
/// round-trip succeeded, non-zero otherwise.
int language_ASTGen_roundTripCheck(void *_Nonnull sourceFile);

/// Emit parser diagnostics for given source file.. Returns non-zero if any
/// diagnostics were emitted.
int language_ASTGen_emitParserDiagnostics(
    BridgedASTContext astContext,
    void *_Nonnull diagEngine, void *_Nonnull sourceFile, int emitOnlyErrors,
    int downgradePlaceholderErrorsToWarnings);

// Build AST nodes for the top-level entities in the syntax.
void language_ASTGen_buildTopLevelASTNodes(
    BridgedDiagnosticEngine diagEngine, void *_Nonnull sourceFile,
    BridgedDeclContext declContext, BridgedNullableDecl attachedDecl,
    BridgedASTContext astContext, void *_Nonnull outputContext,
    void (*_Nonnull)(BridgedASTNode, void *_Nonnull));

BridgedFingerprint
language_ASTGen_getSourceFileFingerprint(void *_Nonnull sourceFile,
                                      BridgedASTContext astContext);

void language_ASTGen_freeBridgedString(BridgedStringRef);

// MARK: - Regex parsing

bool language_ASTGen_lexRegexLiteral(const char *_Nonnull *_Nonnull curPtrPtr,
                                  const char *_Nonnull bufferEndPtr,
                                  bool mustBeRegex,
                                  BridgedNullableDiagnosticEngine diagEngine);

bool language_ASTGen_parseRegexLiteral(
    BridgedStringRef inputPtr, size_t *_Nonnull versionOut,
    void *_Nonnull UnsafeMutableRawPointer, size_t captureStructureSize,
    BridgedRegexLiteralPatternFeatures *_Nonnull featuresOut,
    BridgedSourceLoc diagLoc, BridgedDiagnosticEngine diagEngine);

void language_ASTGen_freeBridgedRegexLiteralPatternFeatures(
    BridgedRegexLiteralPatternFeatures features);

void language_ASTGen_getCodiraVersionForRegexPatternFeature(
    BridgedRegexLiteralPatternFeatureKind kind,
    BridgedCodiraVersion *_Nonnull versionOut);

void language_ASTGen_getDescriptionForRegexPatternFeature(
    BridgedRegexLiteralPatternFeatureKind kind, BridgedASTContext astContext,
    BridgedStringRef *_Nonnull descriptionOut);

intptr_t language_ASTGen_configuredRegions(
    BridgedASTContext astContext,
    void *_Nonnull sourceFile,
    BridgedIfConfigClauseRangeInfo *_Nullable *_Nonnull);
void language_ASTGen_freeConfiguredRegions(
    BridgedIfConfigClauseRangeInfo *_Nullable regions, intptr_t numRegions);

bool language_ASTGen_validateUnqualifiedLookup(
    void *_Nonnull sourceFile,
    BridgedASTContext astContext,
    BridgedSourceLoc sourceLoc,
    bool finishInSequentialScope,
    BridgedArrayRef astScopeResultRef);

size_t
language_ASTGen_virtualFiles(void *_Nonnull sourceFile,
                          BridgedVirtualFile *_Nullable *_Nonnull virtualFiles);
void language_ASTGen_freeBridgedVirtualFiles(
    BridgedVirtualFile *_Nullable virtualFiles, size_t numFiles);

bool language_ASTGen_parseAvailabilityMacroDefinition(
    BridgedASTContext ctx, BridgedDeclContext dc, BridgedDiagnosticEngine diags,
    BridgedStringRef buffer,
    BridgedAvailabilityMacroDefinition *_Nonnull outPtr);

void language_ASTGen_freeAvailabilityMacroDefinition(
    BridgedAvailabilityMacroDefinition *_Nonnull definition);

#ifdef __cplusplus
}
#endif

#endif // LANGUAGE_BRIDGING_ASTGEN_H
