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

#ifndef VISIT_HELPER_INL_H
#define VISIT_HELPER_INL_H

#include "visit_helper.h"

template <class ModuleCallBack>
inline void VisitHelper::EnumerateModules(const ModuleCallBack &cb)
{
    implI_->fileEnumerateModules(file_, (void *)(&cb), [](AbckitCoreModule *mod, void *data) {
        const auto &cb = *((ModuleCallBack *)(data));
        cb(mod);
        return true;
    });
}

template <class ImportCallBack>
inline void VisitHelper::EnumerateModuleImports(AbckitCoreModule *mod, const ImportCallBack &cb)
{
    implI_->moduleEnumerateImports(mod, (void *)(&cb), [](AbckitCoreImportDescriptor *i, void *data) {
        const auto &cb = *((ImportCallBack *)(data));
        cb(i);
        return true;
    });
}

template <class ExportCallBack>
inline void VisitHelper::EnumerateModuleExports(AbckitCoreModule *mod, const ExportCallBack &cb)
{
    implI_->moduleEnumerateExports(mod, (void *)(&cb), [](AbckitCoreExportDescriptor *e, void *data) {
        const auto &cb = *((ExportCallBack *)(data));
        cb(e);
        return true;
    });
}

template <class ClassCallBack>
inline void VisitHelper::EnumerateModuleClasses(AbckitCoreModule *mod, const ClassCallBack &cb)
{
    implI_->moduleEnumerateClasses(mod, (void *)(&cb), [](AbckitCoreClass *klass, void *data) {
        const auto &cb = *((ClassCallBack *)data);
        cb(klass);
        return true;
    });
}

template <class MethodCallBack>
inline void VisitHelper::EnumerateClassMethods(AbckitCoreClass *klass, const MethodCallBack &cb)
{
    implI_->classEnumerateMethods(klass, (void *)(&cb), [](AbckitCoreFunction *method, void *data) {
        const auto &cb = *((MethodCallBack *)data);
        cb(method);
        return true;
    });
}

template <class FunctionCallBack>
inline void VisitHelper::EnumerateModuleTopLevelFunctions(AbckitCoreModule *mod, const FunctionCallBack &cb)
{
    implI_->moduleEnumerateTopLevelFunctions(mod, (void *)(&cb), [](AbckitCoreFunction *method, void *data) {
        const auto &cb = *((FunctionCallBack *)data);
        cb(method);
        return true;
    });
}

template <class FunctionCallBack>
inline void VisitHelper::EnumerateModuleFunctions(AbckitCoreModule *mod, const FunctionCallBack &cb)
{
    // NOTE: currently we can only enumerate class methods and top level functions. need to update.
    EnumerateModuleTopLevelFunctions(mod, cb);
    EnumerateModuleClasses(mod, [&](AbckitCoreClass *klass) { EnumerateClassMethods(klass, cb); });
}

template <class AnnotationCallBack>
inline void VisitHelper::EnumerateClassAnnotations(AbckitCoreClass *klass, const AnnotationCallBack &cb)
{
    implI_->classEnumerateAnnotations(klass, (void *)(&cb), [](AbckitCoreAnnotation *an, void *data) {
        const auto &cb = *((AnnotationCallBack *)data);
        cb(an);
        return true;
    });
}

template <class AnnotationCallBack>
inline void VisitHelper::EnumerateMethodAnnotations(AbckitCoreFunction *m, const AnnotationCallBack &cb)
{
    implI_->functionEnumerateAnnotations(m, (void *)(&cb), [](AbckitCoreAnnotation *an, void *data) {
        const auto &cb = *((AnnotationCallBack *)data);
        cb(an);
        return true;
    });
}

template <class AnnotationElementCallBack>
inline void VisitHelper::EnumerateAnnotationElements(AbckitCoreAnnotation *an, const AnnotationElementCallBack &cb)
{
    implI_->annotationEnumerateElements(an, (void *)(&cb), [](AbckitCoreAnnotationElement *ele, void *data) {
        const auto &cb = *((AnnotationElementCallBack *)data);
        cb(ele);
        return true;
    });
}

template <class InstCallBack>
inline void VisitHelper::EnumerateFunctionInsts(AbckitCoreFunction *func, const InstCallBack &cb)
{
    AbckitGraph *graph = implI_->createGraphFromFunction(func);

    EnumerateGraphInsts(graph, cb);

    impl_->destroyGraph(graph);
}

template <class InstCallBack>
inline void VisitHelper::EnumerateGraphInsts(AbckitGraph *graph, const InstCallBack &cb)
{
    CapturedData captured {(void *)(&cb), implG_};

    implG_->gVisitBlocksRpo(graph, &captured, [](AbckitBasicBlock *bb, void *data) {
        auto *captured = reinterpret_cast<CapturedData *>(data);
        const auto &cb = *((InstCallBack *)(captured->callback));
        auto *implG = captured->implG;
        for (auto *inst = implG->bbGetFirstInst(bb); inst != nullptr; inst = implG->iGetNext(inst)) {
            cb(inst);
        }
        return true;
    });
}

template <class InstCallBack>
inline AbckitInst *VisitHelper::GraphInstsFindIf(AbckitGraph *graph, const InstCallBack &cb)
{
    AbckitInst *ret = nullptr;
    bool found = false;
    EnumerateGraphInsts(graph, [&](AbckitInst *inst) {
        if (!found && cb(inst)) {
            found = true;
            ret = inst;
        }
    });

    return ret;
}

template <class InputCallBack>
inline void VisitHelper::EnumerateInstInputs(AbckitInst *inst, const InputCallBack &cb)
{
    implG_->iVisitInputs(inst, (void *)(&cb), [](AbckitInst *input, void *data) {
        const auto &cb = *((InputCallBack *)data);
        cb(input);
        return true;
    });
}

template <class UserCallBack>
inline void VisitHelper::EnumerateInstUsers(AbckitInst *inst, const UserCallBack &cb)
{
    implG_->iVisitUsers(inst, (bool *)(&cb), [](AbckitInst *user, void *data) {
        const auto &cb = *((UserCallBack *)data);
        cb(user);
        return true;
    });
}

template <class FunctionCallBack>
inline void VisitHelper::TransformMethod(AbckitCoreFunction *f, const FunctionCallBack &cb)
{
    cb(implI_->functionGetFile(f), f);
}

#endif /* VISIT_HELPER_INL_H */
