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

/**
 * @file
 *
 * This file declares core support for java mirror and mirror subtype
 */
#ifndef CODIRA_SEMA_NATIVE_FFI_JAVA_DESUGAR_INTEROP_MANAGER
#define CODIRA_SEMA_NATIVE_FFI_JAVA_DESUGAR_INTEROP_MANAGER

#include "Codira/Mangle/BaseMangler.h"
#include "Codira/Modules/ImportManager.h"
#include "Codira/Sema/TypeManager.h"

namespace Codira::Interop::Java {
using namespace AST;

class JavaInteropManager {
public:
    JavaInteropManager(ImportManager& importManager, TypeManager& typeManager, DiagnosticEngine& diag,
        const BaseMangler& mangler, const std::optional<std::string>& javagenOutputPath, const std::string outputPath,
        bool enableInteropCODEMapping = false)
        : importManager(importManager),
          typeManager(typeManager),
          diag(diag),
          mangler(mangler),
          javagenOutputPath(javagenOutputPath),
          outputPath(outputPath),
          enableInteropCODEMapping(enableInteropCODEMapping)
    {
    }

    void CheckImplRedefinition(Package& package);
    void CheckInheritance(ClassLikeDecl& decl) const;
    void CheckTypes(File& file);
    void CheckTypes(ClassLikeDecl& classLikeDecl);
    void CheckJavaMirrorTypes(ClassLikeDecl& decl);
    void CheckJavaImplTypes(ClassLikeDecl& decl);
    void CheckCODEMappingType(Decl& decl);
    void CheckCODEMappingDeclSupportRange(Decl& decl);
    void DesugarPackage(Package& pkg);

private:
    void CheckUsageOfJavaTypes(Decl& decl);

private:
    void CheckNonJavaSuperType(ClassLikeDecl& decl) const;
    void CheckJavaMirrorSubtypeAttrClassLikeDecl(ClassLikeDecl& decl) const;
    void CheckExtendDecl(ExtendDecl& decl) const;
    void CheckGenericsInstantiation(Decl& file);

    ImportManager& importManager;
    TypeManager& typeManager;
    DiagnosticEngine& diag;
    const BaseMangler& mangler;
    const std::optional<std::string>& javagenOutputPath;
    /**
     * Name of output codira library
     */
    const std::string outputPath;
    /**
     * Flag that informs on presence of any @JavaMirror- or @JavaImpl-annotated entities in the compilation package
     */
    bool hasMirrorOrImpl = false;
    bool enableInteropCODEMapping = false;
};
} // namespace Codira::Interop::Java

#endif // CODIRA_SEMA_NATIVE_FFI_JAVA_DESUGAR_INTEROP_MANAGER
