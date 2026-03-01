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

#include "helpers/visit_helper/visit_helper-inl.h"
#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "helpers/visit_helper/visit_helper.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "metadata_inspect_impl.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class AbckitScenarioTest : public ::testing::Test {};

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

static bool HasSearchedMethod(VisitHelper &visitor, AbckitCoreFunction *method, UserData &userData)
{
    bool isFind = false;
    visitor.EnumerateFunctionInsts(method, [&](AbckitInst *inst) {
        if (g_dynG->iGetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR) {
            return;
        }
        bool findNew = false;
        visitor.EnumerateInstUsers(inst, [&](AbckitInst *user) {
            if (g_dynG->iGetOpcode(user) == ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWOBJRANGE) {
                findNew = true;
            }
        });
        visitor.EnumerateInstUsers(inst, [&](AbckitInst *user) {
            if (findNew && !isFind &&
                g_dynG->iGetOpcode(user) == ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME) {
                auto str = visitor.GetString(g_implG->iGetString(user));
                auto targetClass = visitor.GetString(userData.targetClass);
                isFind = targetClass == str;
            }
        });
    });

    return isFind;
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

    helpers::ModuleByNameContext ctxFinder = {nullptr, API_MODULE_NAME};
    g_implI->fileEnumerateModules(g_implI->functionGetFile(method), &ctxFinder, helpers::ModuleByNameFinder);
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
    vd.newInst = helpers::FindFirstInst(ctxG, ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWOBJRANGE);

    g_implG->gVisitBlocksRpo(ctxG, &vd, [](AbckitBasicBlock *bb, void *data) -> bool { return VisitBlock(bb, data); });
    g_implM->functionSetGraph(method, ctxG);
    g_impl->destroyGraph(ctxG);
}

static void CollectAnnoInfo(
    VisitHelper &visitor,
    std::vector<std::tuple<AbckitCoreClass *, AbckitCoreFunction *, AbckitCoreAnnotation *>> &infos,
    AbckitCoreClass *klass, AbckitCoreFunction *method)
{
    visitor.EnumerateMethodAnnotations(method, [&](AbckitCoreAnnotation *anno) {
        auto *annoClass = g_implI->annotationGetInterface(anno);
        auto annoName = visitor.GetString(g_implI->annotationInterfaceGetName(annoClass));
        if (annoName != ANNOTATION_INTERFACE_NAME) {
            return;
        }
        infos.emplace_back(klass, method, anno);
    });
}

static void FillAnnotationInfo(VisitHelper &visitor, AbckitCoreModule *mod, UserData &ud)
{
    std::vector<std::tuple<AbckitCoreClass *, AbckitCoreFunction *, AbckitCoreAnnotation *>> infos;
    visitor.EnumerateModuleClasses(mod, [&](AbckitCoreClass *klass) {
        visitor.EnumerateClassMethods(
            klass, [&](AbckitCoreFunction *method) { CollectAnnoInfo(visitor, infos, klass, method); });
    });
    for (auto &[klass, method, anno] : infos) {
        visitor.EnumerateAnnotationElements(anno, [&](AbckitCoreAnnotationElement *annoElem) {
            auto annoElemName = visitor.GetString(g_implI->annotationElementGetName(annoElem));
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

static void ClassReplaceCallSite(VisitHelper &visitor, UserData &ud, AbckitCoreClass *klass)
{
    visitor.EnumerateClassMethods(klass, [&](AbckitCoreFunction *method) {
        if (!HasSearchedMethod(visitor, method, ud)) {
            return;
        }
        ReplaceCallSite(method, ud);
    });
}

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(AbckitScenarioTest, LibAbcKitTestDynamicReplaceCallSite)
{
    std::string inputPath = ABCKIT_ABC_DIR "scenarios/replace_call_site/dynamic/replace_call_site.abc";
    std::string outputPath = ABCKIT_ABC_DIR "scenarios/replace_call_site/dynamic/replace_call_site_modified.abc";

    auto output = helpers::ExecuteDynamicAbc(inputPath, "replace_call_site");
    EXPECT_TRUE(helpers::Match(output, "3\n"));

    AbckitFile *ctxI = g_impl->openAbc(inputPath.c_str(), inputPath.size());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto visitor = VisitHelper(ctxI, g_impl, g_implI, g_implG, g_dynG);

    UserData ud {};

    visitor.EnumerateModules([&](AbckitCoreModule *mod) {
        if (visitor.GetString(g_implI->moduleGetName(mod)) != API_MODULE_NAME) {
            return;
        }
        FillAnnotationInfo(visitor, mod, ud);
    });

    ASSERT_EQ(visitor.GetString(g_implI->classGetName(ud.classToReplace)), API_CLASS_NAME);
    ASSERT_EQ(visitor.GetString(g_implI->functionGetName(ud.methodToReplace)), API_METHOD_NAME);

    visitor.EnumerateModules([&](AbckitCoreModule *mod) {
        auto moduleName = visitor.GetString(g_implI->moduleGetName(mod));
        if (std::string_view(moduleName) != "replace_call_site") {
            return;
        }
        visitor.EnumerateModuleClasses(mod, [&](AbckitCoreClass *klass) { ClassReplaceCallSite(visitor, ud, klass); });
    });

    g_impl->writeAbc(ctxI, outputPath.c_str(), outputPath.size());
    g_impl->closeFile(ctxI);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    output = helpers::ExecuteDynamicAbc(outputPath, "replace_call_site");
    EXPECT_TRUE(helpers::Match(output,
                               "fixOrientationLinearLayout was called\n"
                               "5\n"));
}

}  // namespace libabckit::test
