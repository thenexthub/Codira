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

#include <gtest/gtest.h>

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "metadata_inspect_impl.h"
#include "libabckit/src/logger.h"

#include <cstring>

namespace {

auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

struct CapturedData {
    void *callback = nullptr;
    const AbckitGraphApi *gImplG = nullptr;
};

template <class InstCallBack>
inline bool VisitInst(AbckitBasicBlock *bb, void *data)
{
    auto *captured = reinterpret_cast<CapturedData *>(data);
    const auto &cb = *reinterpret_cast<InstCallBack *>(captured->callback);
    auto *implG = captured->gImplG;
    for (auto *inst = implG->bbGetFirstInst(bb); inst != nullptr; inst = implG->iGetNext(inst)) {
        cb(inst);
    }
    return true;
}

template <class InstCallBack>
inline void EnumerateGraphInsts(AbckitGraph *graph, const InstCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;

    CapturedData captured {(void *)(&cb), g_implG};

    g_implG->gVisitBlocksRpo(graph, &captured,
                             [](AbckitBasicBlock *bb, void *data) { return VisitInst<InstCallBack>(bb, data); });
}

template <class InstCallBack>
inline void EnumerateFunctionInsts(AbckitCoreFunction *func, const InstCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;

    AbckitGraph *graph = g_implI->createGraphFromFunction(func);
    EnumerateGraphInsts(graph, cb);
    g_impl->destroyGraph(graph);
}

template <class UserCallBack>
inline void EnumerateInstUsers(AbckitInst *inst, const UserCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;

    g_implG->iVisitUsers(inst, (void *)(&cb), [](AbckitInst *user, void *data) {
        const auto &cb = *((UserCallBack *)data);
        cb(user);
        return true;
    });
}

template <class ClassCallBack>
inline void EnumerateModuleClasses(AbckitCoreModule *mod, const ClassCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;

    g_implI->moduleEnumerateClasses(mod, (void *)(&cb), [](AbckitCoreClass *klass, void *data) {
        const auto &cb = *((ClassCallBack *)data);
        cb(klass);
        return true;
    });
}

template <class MethodCallBack>
inline void EnumerateClassMethods(AbckitCoreClass *klass, const MethodCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;
    g_implI->classEnumerateMethods(klass, (void *)(&cb), [](AbckitCoreFunction *method, void *data) {
        const auto &cb = *((MethodCallBack *)data);
        cb(method);
        return true;
    });
}

template <class AnnotationCallBack>
inline void EnumerateMethodAnnotations(AbckitCoreFunction *m, const AnnotationCallBack &cb)
{
    g_implI->functionEnumerateAnnotations(m, (void *)(&cb), [](AbckitCoreAnnotation *an, void *data) {
        const auto &cb = *((AnnotationCallBack *)data);
        cb(an);
        return true;
    });
}

template <class AnnotationElementCallBack>
inline void EnumerateAnnotationElements(AbckitCoreAnnotation *an, const AnnotationElementCallBack &cb)
{
    g_implI->annotationEnumerateElements(an, (void *)(&cb), [](AbckitCoreAnnotationElement *ele, void *data) {
        const auto &cb = *((AnnotationElementCallBack *)data);
        cb(ele);
        return true;
    });
}

template <class ModuleCallBack>
inline void EnumerateModules(const ModuleCallBack &cb, AbckitFile *file)
{
    LIBABCKIT_LOG_FUNC;

    g_implI->fileEnumerateModules(file, (void *)(&cb), [](AbckitCoreModule *mod, void *data) {
        const auto &cb = *((ModuleCallBack *)(data));
        cb(mod);
        return true;
    });
}

constexpr auto API_CLASS_NAME = "ApiControl";
constexpr auto API_METHOD_NAME = "fixOrientationLinearLayout";
constexpr auto API_MODULE_NAME = "modules/ApiControl";
constexpr auto ANNOTATION_INTERFACE_NAME = "CallSiteReplacement";

struct UserData {
    AbckitString *targetClass;
    AbckitString *methodName;
    AbckitCoreClass *classToReplace;
    AbckitCoreFunction *methodToReplace;
};

bool HasSearchedMethod(AbckitCoreFunction *method, UserData &userData)
{
    bool isFind = false;
    EnumerateFunctionInsts(method, [&](AbckitInst *inst) {
        if (g_dynG->iGetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR) {
            return;
        }
        bool findNew = false;
        EnumerateInstUsers(inst, [&](AbckitInst *user) {
            if (g_dynG->iGetOpcode(user) == ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWOBJRANGE) {
                findNew = true;
            }
        });
        EnumerateInstUsers(inst, [&](AbckitInst *user) {
            if (findNew && !isFind &&
                g_dynG->iGetOpcode(user) == ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME) {
                auto str = g_implI->abckitStringToString(g_implG->iGetString(user));
                auto targetClass = g_implI->abckitStringToString(userData.targetClass);
                isFind = targetClass == str;
            }
        });
    });

    return isFind;
}

struct ModuleByNameContext {
    AbckitCoreModule *module;
    const char *name;
};

bool ModuleByNameFinder(AbckitCoreModule *module, void *data)
{
    auto ctxFinder = reinterpret_cast<ModuleByNameContext *>(data);
    auto name = g_implI->abckitStringToString(g_implI->moduleGetName(module));
    if (strcmp(name, ctxFinder->name) == 0) {
        ctxFinder->module = module;
        return false;
    }

    return true;
}

AbckitInst *FindFirstInst(AbckitGraph *graph, AbckitIsaApiDynamicOpcode opcode)
{
    std::vector<AbckitBasicBlock *> bbs;
    g_implG->gVisitBlocksRpo(graph, &bbs, [](AbckitBasicBlock *bb, void *data) {
        reinterpret_cast<std::vector<AbckitBasicBlock *> *>(data)->emplace_back(bb);
        return true;
    });
    for (auto *bb : bbs) {
        auto *curInst = g_implG->bbGetFirstInst(bb);
        while (curInst != nullptr) {
            if (g_dynG->iGetOpcode(curInst) == opcode) {
                return curInst;
            }
            curInst = g_implG->iGetNext(curInst);
        }
    }
    return nullptr;
}

struct VisitData {
    AbckitGraph *ctxG = nullptr;
    UserData *ud = nullptr;
    AbckitCoreImportDescriptor *ci = nullptr;
    AbckitInst *newInst = nullptr;
};

bool VisitBlock(AbckitBasicBlock *bb, void *data)
{
    auto *vData = reinterpret_cast<VisitData *>(data);
    auto *inst = g_implG->bbGetFirstInst(bb);
    while (inst != nullptr) {
        if (g_dynG->iGetOpcode(inst) == ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS1) {
            auto *ldExternal = g_dynG->iCreateLdexternalmodulevar(vData->ctxG, vData->ci);
            auto *classThrow = g_dynG->iCreateThrowUndefinedifholewithname(
                vData->ctxG, ldExternal, g_implI->classGetName(vData->ud->classToReplace));
            auto *constInst = g_implG->gFindOrCreateConstantI32(vData->ctxG, 5);
            auto *ldobj = g_dynG->iCreateLdobjbyname(vData->ctxG, ldExternal,
                                                     g_implI->functionGetName(vData->ud->methodToReplace));

            auto *staticCall = g_dynG->iCreateCallthis2(vData->ctxG, ldobj, ldExternal, vData->newInst, constInst);

            if (inst == nullptr) {
                return false;
            }

            g_implG->iInsertAfter(ldExternal, inst);
            g_implG->iInsertAfter(classThrow, ldExternal);
            g_implG->iInsertAfter(ldobj, classThrow);
            g_implG->iInsertAfter(staticCall, ldobj);
        }
        inst = g_implG->iGetNext(inst);
    }
    return true;
}

void ReplaceCallSite(AbckitCoreFunction *method, UserData &userData)
{
    // Create import of modules/ApiControl module and import ApiControl from there
    AbckitArktsImportFromDynamicModuleCreateParams params {};
    params.name = API_CLASS_NAME;
    params.alias = API_CLASS_NAME;

    ModuleByNameContext ctxFinder = {nullptr, API_MODULE_NAME};
    g_implI->fileEnumerateModules(g_implI->functionGetFile(method), &ctxFinder, ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *newImport = g_implArkM->moduleAddImportFromArktsV1ToArktsV1(
        g_implArkI->coreModuleToArktsModule(g_implI->functionGetModule(method)),
        g_implArkI->coreModuleToArktsModule(ctxFinder.module), &params);

    auto *coreImport = g_implArkI->arktsImportDescriptorToCoreImportDescriptor(newImport);
    auto ctxG = g_implI->createGraphFromFunction(method);

    VisitData vd;
    vd.ctxG = ctxG;
    vd.ci = coreImport;
    vd.ud = &userData;
    vd.newInst = FindFirstInst(ctxG, ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWOBJRANGE);

    g_implG->gVisitBlocksRpo(ctxG, &vd, [](AbckitBasicBlock *bb, void *data) -> bool { return VisitBlock(bb, data); });
    g_implM->functionSetGraph(method, ctxG);
    g_impl->destroyGraph(ctxG);
}

void CollectAnnoInfo(std::vector<std::tuple<AbckitCoreClass *, AbckitCoreFunction *, AbckitCoreAnnotation *>> &infos,
                     AbckitCoreClass *klass)
{
    EnumerateClassMethods(klass, [&](AbckitCoreFunction *method) {
        EnumerateMethodAnnotations(method, [&](AbckitCoreAnnotation *anno) {
            auto *annoClass = g_implI->annotationGetInterface(anno);
            auto annoName = g_implI->abckitStringToString(g_implI->annotationInterfaceGetName(annoClass));
            if (strcmp(annoName, ANNOTATION_INTERFACE_NAME) != 0) {
                return;
            }
            infos.emplace_back(klass, method, anno);
        });
    });
}

void FillAnnotationInfo(AbckitCoreModule *mod, UserData &ud)
{
    std::vector<std::tuple<AbckitCoreClass *, AbckitCoreFunction *, AbckitCoreAnnotation *>> infos;
    EnumerateModuleClasses(mod, [&](AbckitCoreClass *klass) { CollectAnnoInfo(infos, klass); });
    for (auto &[klass, method, anno] : infos) {
        EnumerateAnnotationElements(anno, [&](AbckitCoreAnnotationElement *annoElem) {
            auto annoElemName = g_implI->abckitStringToString(g_implI->annotationElementGetName(annoElem));
            auto *value = g_implI->annotationElementGetValue(annoElem);
            if (std::string_view(annoElemName) == "targetClass") {
                ud.targetClass = g_implI->valueGetString(value);
            }
            if (std::string_view(annoElemName) == "methodName") {
                ud.methodName = g_implI->valueGetString(value);
            }
        });
        ud.classToReplace = klass;
        ud.methodToReplace = method;
    }
}
}  // namespace

namespace libabckit::test {

class AbckitScenarioCTestClean : public ::testing::Test {};

static void ClassReplaceCallSite(UserData &ud, AbckitCoreClass *klass)
{
    EnumerateClassMethods(klass, [&](AbckitCoreFunction *method) {
        if (!HasSearchedMethod(method, ud)) {
            return;
        }
        ReplaceCallSite(method, ud);
    });
}

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(AbckitScenarioCTestClean, LibAbcKitTestDynamicReplaceCallSiteClean)
{
    std::string inputPath = ABCKIT_ABC_DIR "clean_scenarios/c_api/dynamic/replace_call_site/replace_call_site.abc";
    std::string outputPath =
        ABCKIT_ABC_DIR "clean_scenarios/c_api/dynamic/replace_call_site/replace_call_site_modified.abc";

    auto output = helpers::ExecuteDynamicAbc(inputPath, "replace_call_site");
    EXPECT_TRUE(helpers::Match(output, "3\n"));

    AbckitFile *file = g_impl->openAbc(inputPath.c_str(), inputPath.size());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    UserData ud {};

    EnumerateModules(
        [&](AbckitCoreModule *mod) {
            auto moduleName = g_implI->abckitStringToString(g_implI->moduleGetName(mod));
            if (std::strcmp(moduleName, API_MODULE_NAME) != 0) {
                return;
            }

            FillAnnotationInfo(mod, ud);
        },
        file);

    // "modules/ApiControl" "modules/ApiControl"
    ASSERT_EQ((std::string)g_implI->abckitStringToString(g_implI->classGetName(ud.classToReplace)),
              (std::string)API_CLASS_NAME);
    ASSERT_EQ((std::string)g_implI->abckitStringToString(g_implI->functionGetName(ud.methodToReplace)),
              (std::string)API_METHOD_NAME);

    EnumerateModules(
        [&](AbckitCoreModule *mod) {
            auto moduleName = g_implI->abckitStringToString(g_implI->moduleGetName(mod));
            if (std::string_view(moduleName) != "replace_call_site") {
                return;
            }
            EnumerateModuleClasses(mod, [&](AbckitCoreClass *klass) { ClassReplaceCallSite(ud, klass); });
        },
        file);

    g_impl->writeAbc(file, outputPath.c_str(), outputPath.size());
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    output = helpers::ExecuteDynamicAbc(outputPath, "replace_call_site");
    EXPECT_TRUE(helpers::Match(output,
                               "fixOrientationLinearLayout was called\n"
                               "5\n"));
}
}  // namespace libabckit::test
