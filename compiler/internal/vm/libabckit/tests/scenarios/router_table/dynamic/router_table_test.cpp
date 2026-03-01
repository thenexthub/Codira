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

#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "helpers/visit_helper/visit_helper-inl.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

static const std::string ROUTER_MAP_FILE_MODULE_NAME = "modules/routerMap";
static const std::string ROUTER_ANNOTATION_NAME = "Router";
static const std::string FUNC_MAIN_0 = "func_main_0";

class AbckitScenarioTest : public ::testing::Test {};

struct RouterAnnotation {
    AbckitCoreClass *owner;
    AbckitCoreAnnotation *ptrToAnno;
    AbckitString *scheme;
    AbckitString *path;
};

struct UserData {
    AbckitString *classStr;
    AbckitString *moduleStr;
    RouterAnnotation routerInfo;
};

struct LocalData {
    size_t idx;
    AbckitInst *insertAfterInst;
    AbckitCoreImportDescriptor *coreImport;
};

static void TransformMethod(AbckitCoreFunction *method, VisitHelper &visitor, const UserData *ud, LocalData *ld)
{
    visitor.TransformMethod(method, [&](AbckitFile *file, AbckitCoreFunction *method) {
        auto ctxG = g_implI->createGraphFromFunction(method);
        AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(ctxG);
        std::vector<AbckitBasicBlock *> succBBs;
        g_implG->bbVisitSuccBlocks(startBB, &succBBs, [](AbckitBasicBlock *succBasicBlock, void *d) {
            auto *succs = reinterpret_cast<std::vector<AbckitBasicBlock *> *>(d);
            succs->emplace_back(succBasicBlock);
            return true;
        });
        AbckitInst *createEmptyArray = helpers::FindFirstInst(ctxG, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYARRAY);

        const auto &routerInfo = ud->routerInfo;
        std::string fullPath =
            std::string(visitor.GetString(routerInfo.scheme)) + std::string(visitor.GetString(routerInfo.path));
        auto arr = std::vector<AbckitLiteral *>();
        AbckitLiteral *str = g_implM->createLiteralString(file, fullPath.data(), fullPath.size());
        arr.emplace_back(str);

        auto *litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());
        auto *createArrayWithBuffer = g_dynG->iCreateCreatearraywithbuffer(ctxG, litArr);

        if (ld->insertAfterInst == nullptr) {
            ld->insertAfterInst = createEmptyArray;
        }

        auto *ldExternal = g_dynG->iCreateLdexternalmodulevar(ctxG, ld->coreImport);

        auto *classThrow =
            g_dynG->iCreateThrowUndefinedifholewithname(ctxG, ldExternal, g_implI->classGetName(routerInfo.owner));

        auto *newObj = g_dynG->iCreateNewobjrange(ctxG, 1, ldExternal);

        auto *stownByIndex1 = g_dynG->iCreateStownbyindex(ctxG, newObj, createArrayWithBuffer, 1);

        auto *stownByIndex2 = g_dynG->iCreateStownbyindex(ctxG, createArrayWithBuffer, createEmptyArray, ld->idx++);

        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        g_implG->iInsertAfter(createArrayWithBuffer, ld->insertAfterInst);
        g_implG->iInsertAfter(ldExternal, createArrayWithBuffer);
        g_implG->iInsertAfter(classThrow, ldExternal);
        g_implG->iInsertAfter(newObj, classThrow);
        g_implG->iInsertAfter(stownByIndex1, newObj);
        g_implG->iInsertAfter(stownByIndex2, stownByIndex1);

        ld->insertAfterInst = stownByIndex2;

        g_implM->functionSetGraph(method, ctxG);
        g_impl->destroyGraph(ctxG);
    });
}

static void ModifyRouterTable(AbckitCoreFunction *method, VisitHelper &visitor,
                              const std::vector<UserData> &udContainer)
{
    LocalData ld {};
    ld.idx = 0;
    ld.insertAfterInst = nullptr;
    for (const auto &ud : udContainer) {
        const auto &className = visitor.GetString(ud.classStr);
        AbckitArktsImportFromDynamicModuleCreateParams params {};
        params.name = className.data();
        params.alias = className.data();

        const auto &moduleName = visitor.GetString(ud.moduleStr);
        helpers::ModuleByNameContext ctxFinder = {nullptr, moduleName.data()};
        g_implI->fileEnumerateModules(g_implI->functionGetFile(method), &ctxFinder, helpers::ModuleByNameFinder);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(ctxFinder.module, nullptr);
        auto *newImport = g_implArkM->moduleAddImportFromArktsV1ToArktsV1(
            g_implArkI->coreModuleToArktsModule(g_implI->functionGetModule(method)),
            g_implArkI->coreModuleToArktsModule(ctxFinder.module), &params);
        ld.coreImport = g_implArkI->arktsImportDescriptorToCoreImportDescriptor(newImport);
        TransformMethod(method, visitor, &ud, &ld);
    }
}

static AbckitCoreFunction *FindMethodWithRouterTable(VisitHelper &visitor)
{
    AbckitCoreFunction *routerFunc = nullptr;
    visitor.EnumerateModules([&](AbckitCoreModule *mod) {
        auto moduleName = visitor.GetString(g_implI->moduleGetName(mod));
        if (moduleName != ROUTER_MAP_FILE_MODULE_NAME) {
            return;
        }
        visitor.EnumerateModuleTopLevelFunctions(mod, [&](AbckitCoreFunction *func) {
            auto funcName = visitor.GetString(g_implI->functionGetName(func));
            if (funcName != FUNC_MAIN_0) {
                return;
            }
            routerFunc = func;
        });
    });
    return routerFunc;
}

static void RemoveAnnotations(const std::vector<UserData> &udContainer)
{
    for (const auto &ud : udContainer) {
        const auto &routerInfo = ud.routerInfo;
        g_implArkM->classRemoveAnnotation(g_implArkI->coreClassToArktsClass(routerInfo.owner),
                                          g_implArkI->coreAnnotationToArktsAnnotation(routerInfo.ptrToAnno));
    }
}

static void CollectClasseInfo(VisitHelper &visitor, std::vector<UserData> &udContainer, AbckitCoreModule *mod,
                              AbckitCoreClass *klass)
{
    visitor.EnumerateClassAnnotations(klass, [&](AbckitCoreAnnotation *anno) {
        auto *annoClass = g_implI->annotationGetInterface(anno);
        auto annoName = visitor.GetString(g_implI->annotationInterfaceGetName(annoClass));
        if (annoName != ROUTER_ANNOTATION_NAME) {
            return;
        }
        UserData ud {};
        ud.classStr = g_implI->classGetName(klass);
        ud.moduleStr = g_implI->moduleGetName(mod);
        auto &routerInfo = ud.routerInfo;
        routerInfo.ptrToAnno = anno;
        routerInfo.owner = klass;
        visitor.EnumerateAnnotationElements(anno, [&](AbckitCoreAnnotationElement *annoElem) {
            auto annoElemName = visitor.GetString(g_implI->annotationElementGetName(annoElem));
            auto *value = g_implI->annotationElementGetValue(annoElem);
            if (std::string_view(annoElemName) == "scheme") {
                routerInfo.scheme = g_implI->valueGetString(value);
            }
            if (std::string_view(annoElemName) == "path") {
                routerInfo.path = g_implI->valueGetString(value);
            }
        });
        udContainer.push_back(ud);
    });
}

static void CollectClassesInfo(VisitHelper &visitor, std::vector<UserData> &udContainer)
{
    visitor.EnumerateModules([&](AbckitCoreModule *mod) {
        visitor.EnumerateModuleClasses(
            mod, [&](AbckitCoreClass *klass) { CollectClasseInfo(visitor, udContainer, mod, klass); });
    });
}

static bool ClassHasAnnotation(VisitHelper &visitor, const UserData &ud)
{
    bool found = false;

    auto classAnnotationsEnumCb = [&](AbckitCoreAnnotation *anno) {
        auto *annoClass = g_implI->annotationGetInterface(anno);
        auto annoName = visitor.GetString(g_implI->annotationInterfaceGetName(annoClass));
        if (annoName == ROUTER_ANNOTATION_NAME) {
            found = true;
        }
    };

    visitor.EnumerateModules([&](AbckitCoreModule *mod) {
        if (visitor.GetString(g_implI->moduleGetName(mod)) != visitor.GetString(ud.moduleStr)) {
            return;
        }
        visitor.EnumerateModuleClasses(mod, [&](AbckitCoreClass *klass) {
            if (visitor.GetString(g_implI->classGetName(klass)) != visitor.GetString(ud.classStr)) {
                return;
            }
            visitor.EnumerateClassAnnotations(klass, classAnnotationsEnumCb);
        });
    });
    return found;
}

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(AbckitScenarioTest, LibAbcKitTestRouterTable)
{
    std::string inputPath = ABCKIT_ABC_DIR "scenarios/router_table/dynamic/router_table.abc";
    std::string outputPath = ABCKIT_ABC_DIR "scenarios/router_table/dynamic/router_table_modified.abc";

    auto output = helpers::ExecuteDynamicAbc(inputPath, "router_table");
    EXPECT_TRUE(helpers::Match(output, ""));

    AbckitFile *ctxI = g_impl->openAbc(inputPath.c_str(), inputPath.size());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto visitor = VisitHelper(ctxI, g_impl, g_implI, g_implG, g_dynG);

    std::vector<UserData> userData;

    CollectClassesInfo(visitor, userData);
    RemoveAnnotations(userData);
    auto *method = FindMethodWithRouterTable(visitor);
    ModifyRouterTable(method, visitor, userData);

    g_impl->writeAbc(ctxI, outputPath.c_str(), outputPath.size());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    for (const auto &ud : userData) {
        ASSERT_FALSE(ClassHasAnnotation(visitor, ud));
    }

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "scenarios/router_table/dynamic/router_table_modified.abc",
                                        "router_table");

    g_impl->closeFile(ctxI);

    EXPECT_TRUE(helpers::Match(output,
                               "xxxHandler.handle was called\n"
                               "handle1/xxx\n"));
}

}  // namespace libabckit::test
