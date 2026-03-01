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

#ifndef VISIT_HELPER_H
#define VISIT_HELPER_H

#include <string>

#include "libabckit/c/abckit.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "macros.h"
#include "libpandabase/macros.h"

class VisitHelper {
public:
    VisitHelper() = default;
    ~VisitHelper() = default;
    DEFAULT_COPY_SEMANTIC(VisitHelper);
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(VisitHelper);
    explicit VisitHelper(AbckitFile *file, const AbckitApi *impl, const AbckitInspectApi *implI,
                         const AbckitGraphApi *implG, const AbckitIsaApiDynamic *dynG);

    // ModuleCallBack: (AbckitCoreModule *) -> void
    template <class ModuleCallBack>
    void EnumerateModules(const ModuleCallBack &cb);

    // ImportCallBack: (AbckitCoreImportDescriptor *) -> void
    template <class ImportCallBack>
    void EnumerateModuleImports(AbckitCoreModule *mod, const ImportCallBack &cb);

    // ExportCallBack: (AbckitCoreExportDescriptor *) -> void
    template <class ExportCallBack>
    void EnumerateModuleExports(AbckitCoreModule *mod, const ExportCallBack &cb);

    // FunctionCallBack: (AbckitCoreFunction *) -> void
    template <class FunctionCallBack>
    void EnumerateModuleTopLevelFunctions(AbckitCoreModule *mod, const FunctionCallBack &cb);

    // enumerate all functions (including methods) in the module
    // FunctionCallBack: (AbckitCoreFunction *) -> void
    template <class FunctionCallBack>
    void EnumerateModuleFunctions(AbckitCoreModule *mod, const FunctionCallBack &cb);

    // ClassCallBack: (AbckitCoreClass *) -> void
    template <class ClassCallBack>
    void EnumerateModuleClasses(AbckitCoreModule *mod, const ClassCallBack &cb);

    // MethodCallBack: (AbckitCoreFunction *) -> void
    template <class MethodCallBack>
    void EnumerateClassMethods(AbckitCoreClass *klass, const MethodCallBack &cb);

    // AnnotationCallBack: (AbckitCoreAnnotation *) -> void
    template <class AnnotationCallBack>
    void EnumerateClassAnnotations(AbckitCoreClass *klass, const AnnotationCallBack &cb);

    // AnnotationCallBack: (AbckitCoreAnnotation *) -> void
    template <class AnnotationCallBack>
    void EnumerateMethodAnnotations(AbckitCoreFunction *m, const AnnotationCallBack &cb);

    // AnnotationElementCallBack: (AbckitCoreAnnotationElement *) -> void
    template <class AnnotationElementCallBack>
    void EnumerateAnnotationElements(AbckitCoreAnnotation *an, const AnnotationElementCallBack &cb);

    // InstCallBack: (AbckitInst *) -> void
    // NB: the graph context and Inst pointers will be deleted at the end of enumeration
    template <class InstCallBack>
    void EnumerateFunctionInsts(AbckitCoreFunction *f, const InstCallBack &cb);

    struct CapturedData {
        void *callback = nullptr;
        const AbckitGraphApi *implG = nullptr;
    };

    // InstCallBack: (AbckitInst *) -> void
    template <class InstCallBack>
    void EnumerateGraphInsts(AbckitGraph *graph, const InstCallBack &cb);

    // InstCallBack: (AbckitInst *) -> bool
    template <class InstCallBack>
    AbckitInst *GraphInstsFindIf(AbckitGraph *graph, const InstCallBack &cb);

    // InputCallBack: (AbckitInst *) -> void
    template <class InputCallBack>
    void EnumerateInstInputs(AbckitInst *inst, const InputCallBack &cb);

    // UserCallBack: (AbckitInst *) -> void
    template <class UserCallBack>
    void EnumerateInstUsers(AbckitInst *inst, const UserCallBack &cb);

    // FunctionCallBack: (AbckitFile *, AbckitCoreFunction *) -> void
    template <class FunctionCallBack>
    void TransformMethod(AbckitCoreFunction *f, const FunctionCallBack &cb);

    std::string_view GetString(AbckitString *str) const;

private:
    AbckitFile *file_ = nullptr;
    const AbckitApi *impl_ = nullptr;
    const AbckitInspectApi *implI_ = nullptr;
    const AbckitGraphApi *implG_ = nullptr;
    const AbckitIsaApiDynamic *dynG_ = nullptr;
};

#endif /* VISIT_HELPER_H */
