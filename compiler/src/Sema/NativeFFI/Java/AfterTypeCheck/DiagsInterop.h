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

#ifndef CODIRA_SEMA_NATIVE_FFI_JAVA_DIAGS
#define CODIRA_SEMA_NATIVE_FFI_JAVA_DIAGS

#include "Codira/Basic/DiagnosticEngine.h"

namespace Codira::Interop::Java {
using namespace Codira;
using namespace AST;

void DiagJavaImplRedefinitionInJava(DiagnosticEngine& diag, const ClassLikeDecl& decl, const ClassLikeDecl& prevDecl);
void DiagJavaMirrorChildMustBeAnnotated(DiagnosticEngine& diag, const ClassLikeDecl& decl);
void DiagJavaDeclCannotInheritPureCodiraType(DiagnosticEngine& diag, ClassLikeDecl& decl);
void DiagJavaDeclCannotBeExtendedWithInterface(DiagnosticEngine& diag, ExtendDecl& decl);
void DiagUsageOfJavaTypes(DiagnosticEngine& diag, const Decl& varDecl, std::vector<Ptr<Decl>>&& javaDecls,
    Ptr<Decl> nonJavaOuterDecl = nullptr);
void DiagJavaTypesAsGenericParam(DiagnosticEngine& diag, const Node& expr, std::vector<Ptr<Decl>>&& javaDecls);

} // namespace Codira::Interop::Java

#endif // CODIRA_SEMA_NATIVE_FFI_JAVA_DIAGS
