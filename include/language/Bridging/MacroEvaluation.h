//===--- MacroEvaluation.h --------------------------------------*- C++ -*-===//
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

#ifndef SWIFT_BRIDGING_MACROS_H
#define SWIFT_BRIDGING_MACROS_H

#include "language/Basic/BasicBridging.h"

#ifdef __cplusplus
extern "C" {
#endif

void *_Nonnull swift_Macros_resolveExternalMacro(
    const char *_Nonnull moduleName, const char *_Nonnull typeName,
    void *_Nonnull opaquePluginHandle);
void swift_Macros_destroyExternalMacro(void *_Nonnull macro);

bool swift_Macros_checkDefaultArgumentMacroExpression(
    void *_Nonnull diagEngine, void *_Nonnull sourceFile,
    const void *_Nonnull macroSourceLocation);

ptrdiff_t swift_Macros_checkMacroDefinition(
    void *_Nonnull diagEngine, BridgedStringRef sourceFileBuffer,
    BridgedStringRef macroDeclText,
    BridgedStringRef *_Nonnull expansionSourceOutPtr,
    ptrdiff_t *_Nullable *_Nonnull replacementsPtr,
    ptrdiff_t *_Nonnull numReplacements,
    ptrdiff_t *_Nullable *_Nonnull genericReplacementsPtr,
    ptrdiff_t *_Nonnull numGenericReplacements);
void swift_Macros_freeExpansionReplacements(
    ptrdiff_t *_Nullable replacementsPtr, ptrdiff_t numReplacements);

ptrdiff_t swift_Macros_expandFreestandingMacro(
    void *_Nonnull diagEngine, const void *_Nonnull macro,
    const char *_Nonnull discriminator, uint8_t rawMacroRole,
    void *_Nonnull sourceFile, const void *_Nullable sourceLocation,
    BridgedStringRef *_Nonnull evaluatedSourceOut);

ptrdiff_t swift_Macros_expandAttachedMacro(
    void *_Nonnull diagEngine, const void *_Nonnull macro,
    const char *_Nonnull discriminator, const char *_Nonnull qualifiedType,
    const char *_Nonnull conformances, uint8_t rawMacroRole,
    void *_Nonnull customAttrSourceFile,
    const void *_Nullable customAttrSourceLocation,
    void *_Nonnull declarationSourceFile,
    const void *_Nullable declarationSourceLocation,
    void *_Nullable parentDeclSourceFile,
    const void *_Nullable parentDeclSourceLocation,
    BridgedStringRef *_Nonnull evaluatedSourceOut);

bool swift_Macros_initializePlugin(void *_Nonnull handle,
                                   void *_Nullable diagEngine);
void swift_Macros_deinitializePlugin(void *_Nonnull handle);
bool swift_Macros_pluginServerLoadLibraryPlugin(
    void *_Nonnull handle, const char *_Nonnull libraryPath,
    const char *_Nonnull moduleName, BridgedStringRef *_Nullable errorOut);

#ifdef __cplusplus
}
#endif

#endif // SWIFT_BRIDGING_MACROS_H
