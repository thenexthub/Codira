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
#ifndef LIBABCKIT_TESTS_HELPERS
#define LIBABCKIT_TESTS_HELPERS

#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/isa/isa_static.h"
#include "libabckit/c/abckit.h"
#include "libabckit/cpp/abckit_cpp.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/src/logger.h"

#include <string>
#include <type_traits>
#include <vector>
#include <functional>
#include <optional>

#include <gtest/gtest.h>

#ifndef ABCKIT_ABC_DIR
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ABCKIT_ABC_DIR ""
#endif

#ifndef ABCKIT_TEST_DIR
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ABCKIT_TEST_DIR ""
#endif

namespace libabckit::test::helpers {

std::vector<AbckitBasicBlock *> BBgetPredBlocks(AbckitBasicBlock *bb);
std::vector<AbckitBasicBlock *> BBgetSuccBlocks(AbckitBasicBlock *bb);
std::vector<AbckitInst *> BBgetAllInsts(AbckitBasicBlock *bb);

template <class T>
// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct InstSchema {
    InstSchema(size_t id, T o, std::vector<size_t> i) : id(id), opcode(o), inputs(std::move(i)) {}
    size_t id;
    T opcode;
    std::vector<size_t> inputs;
};
// NOLINTEND(misc-non-private-member-variables-in-classes)
template struct InstSchema<AbckitIsaApiStaticOpcode>;
template struct InstSchema<AbckitIsaApiDynamicOpcode>;

template <class T>
// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct BBSchema {
    BBSchema(std::vector<size_t> p, std::vector<size_t> s, std::vector<InstSchema<T>> i)
        : preds(std::move(p)), succs(std::move(s)), instSchemas(std::move(i))
    {
    }
    std::vector<size_t> preds;
    std::vector<size_t> succs;
    std::vector<InstSchema<T>> instSchemas;
};
// NOLINTEND(misc-non-private-member-variables-in-classes)
template struct BBSchema<AbckitIsaApiStaticOpcode>;
template struct BBSchema<AbckitIsaApiDynamicOpcode>;

inline static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
inline static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
inline static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
inline static auto g_gStat = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);
inline static auto g_gDyn = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);
inline static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

template <class T>
static void VerifyBBPreds(AbckitBasicBlock *bb, const std::unordered_map<AbckitBasicBlock *, size_t> &bbToIdx,
                          const BBSchema<T> &bbSchema)
{
    // Verify bb predecessors
    auto preds = BBgetPredBlocks(bb);
    ASSERT_EQ(preds.size(), bbSchema.preds.size());
    for (size_t predIdx = 0; predIdx < preds.size(); predIdx++) {
        ASSERT_EQ(bbToIdx.at(preds[predIdx]), bbSchema.preds[predIdx]);
    }
}

template <class T>
static void VerifyBBSuccs(AbckitBasicBlock *bb, const std::unordered_map<AbckitBasicBlock *, size_t> &bbToIdx,
                          const BBSchema<T> &bbSchema)
{
    // Verify bb successors
    auto succs = BBgetSuccBlocks(bb);
    ASSERT_EQ(succs.size(), bbSchema.succs.size());
    for (size_t succIdx = 0; succIdx < succs.size(); succIdx++) {
        ASSERT_EQ(bbToIdx.at(succs[succIdx]), bbSchema.succs[succIdx]);
    }
}

template <typename T>
static void VerifyGraphInputs(uint32_t inputCount, const InstSchema<T> &instSchema,
                              const std::unordered_map<size_t, size_t> &schemaIdToId, AbckitInst *inst)
{
    // Verify inputs
    ASSERT_EQ(inputCount, instSchema.inputs.size());
    for (size_t inputNumber = 0; inputNumber < inputCount; inputNumber++) {
        auto input = g_implG->iGetInput(inst, inputNumber);
        auto inputId = g_implG->iGetId(input);
        auto inputIdx = instSchema.inputs[inputNumber];
        ASSERT_NE(schemaIdToId.find(inputIdx), schemaIdToId.end());
        ASSERT_EQ(schemaIdToId.at(inputIdx), inputId);
    }
}

template <typename T>
static void VerifyInstSchema(const std::vector<AbckitBasicBlock *> &bbs, const std::vector<BBSchema<T>> &bbSchemas,
                             const std::unordered_map<AbckitBasicBlock *, size_t> &bbToIdx)
{
    std::unordered_map<size_t, size_t> schemaIdToId;  // Map from inst schema id to actual inst id
    for (size_t bbIdx = 0; bbIdx < bbs.size(); bbIdx++) {
        auto *bb = bbs[bbIdx];
        auto &bbSchema = bbSchemas[bbIdx];

        VerifyBBPreds<T>(bb, bbToIdx, bbSchema);

        VerifyBBSuccs<T>(bb, bbToIdx, bbSchema);

        // Collect instructions
        std::vector<AbckitInst *> insts = BBgetAllInsts(bb);

        // Verify instructions
        auto &instSchemas = bbSchema.instSchemas;
        std::unordered_map<size_t, AbckitInst *> idToInst;
        ASSERT_EQ(insts.size(), instSchemas.size());
        for (size_t instIdx = 0; instIdx < insts.size(); instIdx++) {
            auto &instSchema = instSchemas[instIdx];
            auto *inst = insts[instIdx];

            if constexpr (std::is_same<T, AbckitIsaApiDynamicOpcode>()) {
                ASSERT_EQ(g_gDyn->iGetOpcode(inst), instSchema.opcode);
            } else if constexpr (std::is_same<T, AbckitIsaApiStaticOpcode>()) {
                ASSERT_EQ(g_gStat->iGetOpcode(inst), instSchema.opcode);
            } else {
                LIBABCKIT_UNREACHABLE;
            }

            auto instId = g_implG->iGetId(inst);
            auto instSchemaId = instSchema.id;
            schemaIdToId.insert({instSchemaId, instId});
            idToInst.insert({instId, inst});

            VerifyGraphInputs(g_implG->iGetInputCount(inst), instSchema, schemaIdToId, inst);
        }
    }
}

template <class T>
void VerifyGraph(AbckitGraph *graph, const std::vector<BBSchema<T>> &bbSchemas)
{
    LIBABCKIT_LOG(DEBUG) << " Graph:\n";
    LIBABCKIT_LOG_DUMP(g_implG->gDump(graph, 2U), DEBUG);
    std::vector<AbckitBasicBlock *> bbs;

    // Collect basic blocks
    g_implG->gVisitBlocksRpo(graph, &bbs, [](AbckitBasicBlock *bb, void *data) {
        reinterpret_cast<std::vector<AbckitBasicBlock *> *>(data)->emplace_back(bb);
        return true;
    });

    // Construct maps bb->idx
    std::unordered_map<AbckitBasicBlock *, size_t> bbToIdx;
    ASSERT_EQ(bbs.size(), bbSchemas.size());
    for (size_t bbIdx = 0; bbIdx < bbs.size(); bbIdx++) {
        auto *bb = bbs[bbIdx];
        bbToIdx.insert({bb, bbIdx});
    }

    VerifyInstSchema(bbs, bbSchemas, bbToIdx);
}

bool Match(const std::string &actual, const std::string &expected);
void TransformMethod(AbckitFile *file, const std::string &methodSignature,
                     const std::function<void(AbckitFile *, AbckitCoreFunction *, AbckitGraph *)> &userTransformer);
void TransformMethod(const std::string &inputPath, const std::string &outputPath, const std::string &methodSignature,
                     const std::function<void(AbckitFile *, AbckitCoreFunction *, AbckitGraph *)> &userTransformer);
void TransformMethod(const std::string &inputPath, const std::string &outputPath, const std::string &methodSignature,
                     const std::function<void(AbckitFile *, AbckitCoreFunction *, AbckitGraph *)> &userTransformer,
                     const std::function<void(AbckitGraph *)> &validateResult);
void TransformMethod(AbckitCoreFunction *method,
                     const std::function<void(AbckitFile *, AbckitCoreFunction *, AbckitGraph *)> &userTransformer);

void InspectMethod(AbckitFile *file, const std::string &methodSignature,
                   const std::function<void(AbckitFile *, AbckitCoreFunction *, AbckitGraph *)> &userInspector);
void InspectMethod(const std::string &inputPath, const std::string &methodSignature,
                   const std::function<void(AbckitFile *, AbckitCoreFunction *, AbckitGraph *)> &userInspector);

AbckitInst *FindFirstInst(
    AbckitGraph *graph, AbckitIsaApiStaticOpcode opcode,
    const std::function<bool(AbckitInst *)> &findIf = [](AbckitInst *) { return true; });
AbckitInst *FindFirstInst(
    AbckitGraph *graph, AbckitIsaApiDynamicOpcode opcode,
    const std::function<bool(AbckitInst *)> &findIf = [](AbckitInst *) { return true; });
AbckitInst *FindLastInst(
    AbckitGraph *graph, AbckitIsaApiDynamicOpcode opcode,
    const std::function<bool(AbckitInst *)> &findIf = [](AbckitInst *) { return true; });
AbckitInst *FindLastInst(
    AbckitGraph *graph, AbckitIsaApiStaticOpcode opcode,
    const std::function<bool(AbckitInst *)> &findIf = [](AbckitInst *) { return true; });
void ReplaceInst(AbckitInst *what, AbckitInst *with);
void EnumerateAllMethods(AbckitFile *file, const std::function<void(AbckitCoreFunction *)> &cb);

struct ModuleByNameContext {
    AbckitCoreModule *module;
    const char *name;
};

struct ImportByAliasContext {
    AbckitCoreImportDescriptor *id;
    const char *name;
};

struct ExportByAliasContext {
    AbckitCoreExportDescriptor *ed;
    const char *name;
};

struct ClassByNameContext {
    AbckitCoreClass *klass;
    const char *name;
};

struct EnumByNameContext {
    AbckitCoreEnum *enm;
    const char *name;
};

struct MethodByNameContext {
    AbckitCoreFunction *method = nullptr;
    const char *name = "";
    bool fullSign = false;
};

struct NamespaceByNameContext {
    AbckitCoreNamespace *n;
    const char *name;
};

struct NamespaceFieldByNameContext {
    AbckitCoreNamespaceField *cnf;
    const char *name;
};

struct AnnotationInterfaceByNameContext {
    AbckitCoreAnnotationInterface *ai;
    const char *name;
};

struct AnnotationInterfaceFieldByNameContext {
    AbckitCoreAnnotationInterfaceField *aif;
    const char *name;
};

struct AnnotationByNameContext {
    AbckitCoreAnnotation *anno;
    const char *name;
};

struct InterfaceByNameContext {
    AbckitCoreInterface *iface;
    const char *name;
};

struct FieldByNameContext {
    AbckitCoreInterfaceField *field;
    const char *name;
};
struct CoreClassField {
    AbckitCoreClassField *filed;
    const char *name;
};

struct ModuleFieldByNameContext {
    AbckitCoreModuleField *cmf;
    const char *name;
};

struct InterfaceFieldByNameContext {
    AbckitCoreInterfaceField *cif;
    const char *name;
};

struct EnumFieldByNameContext {
    AbckitCoreEnumField *cef;
    const char *name;
};

struct ClassFieldByNameContext {
    AbckitCoreClassField *ccf;
    const char *name;
};

AbckitCoreFunction *FindMethodByName(AbckitFile *file, const std::string &name);
AbckitCoreNamespace *FindNamespaceByName(AbckitFile *file, const std::string &name);
bool ModuleByNameFinder(AbckitCoreModule *module, void *data);
bool ImportByAliasFinder(AbckitCoreImportDescriptor *id, void *data);
bool ExportByAliasFinder(AbckitCoreExportDescriptor *ed, void *data);
bool ClassByNameFinder(AbckitCoreClass *klass, void *data);
bool EnumByNameFinder(AbckitCoreEnum *enm, void *data);
bool InterfaceByNameFinder(AbckitCoreInterface *iface, void *data);
bool NamespaceByNameFinder(AbckitCoreNamespace *n, void *data);
bool MethodByNameFinder(AbckitCoreFunction *method, void *data);
bool AnnotationInterfaceByNameFinder(AbckitCoreAnnotationInterface *ai, void *data);
bool AnnotationByNameFinder(AbckitCoreAnnotation *anno, void *data);
bool NameToModuleCollector(AbckitCoreModule *module, void *data);
bool ModuleImportsCollector(AbckitCoreImportDescriptor *id, void *data);
bool ModuleExportsCollector(AbckitCoreExportDescriptor *ed, void *data);
bool ModuleFieldByNameFinder(AbckitCoreModuleField *mf, void *data);
bool InterfaceFieldByNameFinder(AbckitCoreInterfaceField *cif, void *data);
bool NamespaceFieldByNameFinder(AbckitCoreNamespaceField *cnf, void *data);
bool EnumByNameFinder(AbckitCoreEnum *ce, void *data);
bool EnumFieldByNameFinder(AbckitCoreEnumField *cef, void *data);
bool ClassFieldByNameFinder(AbckitCoreClassField *ccf, void *data);
bool AnnotationInterfaceFieldByNameFinder(AbckitCoreAnnotationInterfaceField *aif, void *data);

void AssertModuleVisitor(AbckitCoreModule *module, void *data);
void AssertImportVisitor(AbckitCoreImportDescriptor *id, void *data);
void AssertExportVisitor(AbckitCoreExportDescriptor *ed, void *data);
void AssertClassVisitor(AbckitCoreClass *klass, void *data);
void AssertEnumVisitor(AbckitCoreEnum *enm, void *data);
void AssertMethodVisitor(AbckitCoreFunction *method, void *data);
void AssertOpenAbc(const char *fname, AbckitFile **file);
void AssertModuleFieldVisitor(AbckitCoreModuleField *cmf, void *data);
void AssertInterfaceVisitor(AbckitCoreInterface *ci, void *data);
void AssertInterfaceFieldVisitor(AbckitCoreInterfaceField *cif, void *data);
void AssertEnumFieldVisitor(AbckitCoreEnumField *cef, void *data);
void AssertClassFieldVisitor(AbckitCoreClassField *ccf, void *data);
std::string_view AbckitStringToString(AbckitString *str);
std::string GetCropFuncName(const std::string &fullSig);
std::optional<abckit::core::Class> GetClassByName(const abckit::core::Module &module, const std::string &name);
std::optional<abckit::core::Interface> GetInterfaceByName(const abckit::core::Module &module, const std::string &name);
std::optional<abckit::core::Enum> GetEnumByName(const abckit::core::Module &module, const std::string &name);
std::optional<abckit::core::Namespace> GetNamespaceByName(const abckit::core::Module &module, const std::string &name);
std::optional<abckit::core::Function> GetFunctionByName(const abckit::core::Module &module, const std::string &name);

bool FieldByNameFinder(AbckitCoreInterfaceField *field, void *data);
bool ClassFieldFinder(AbckitCoreClassField *field, void *data);
}  // namespace libabckit::test::helpers

#endif  // LIBABCKIT_TESTS_HELPERS
