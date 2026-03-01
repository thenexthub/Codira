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

#ifndef COMPILER_TESTS_UNIT_TEST_H
#define COMPILER_TESTS_UNIT_TEST_H

#include "libarkbase/macros.h"
#include "optimizer/ir/ir_constructor.h"
#include "gtest/gtest.h"
#include "libarkbase/mem/arena_allocator.h"
#include "libarkbase/mem/pool_manager.h"
#include <numeric>
#include <unordered_map>
#include "compiler.h"
#include "compiler_logger.h"
#include "graph_comparator.h"
#include "include/runtime.h"
#include "libarkfile/file_item_container.h"

namespace ark::compiler {
struct RuntimeInterfaceMock : public compiler::RuntimeInterface {
    // Only one exact CALLEE may exist in fake runtime
    MethodPtr GetMethodById(MethodPtr /* unused */, MethodId id) const override
    {
        savedCalleeId_ = id;
        return reinterpret_cast<MethodPtr>(CALLEE);
    }

    MethodId GetMethodId(MethodPtr method) const override
    {
        return (reinterpret_cast<uintptr_t>(method) == CALLEE) ? savedCalleeId_ : 0;
    }

    uint64_t GetUniqMethodId(MethodPtr method) const override
    {
        return (reinterpret_cast<uintptr_t>(method) == CALLEE) ? savedCalleeId_ : 0;
    }

    BinaryFilePtr GetBinaryFileForMethod(MethodPtr /* unused */) const override
    {
        static constexpr uintptr_t FAKE_FILE = 0xdeadf;
        return reinterpret_cast<void *>(FAKE_FILE);
    }

    DataType::Type GetMethodReturnType(MethodPtr /* unused */) const override
    {
        return returnType_;
    }

    DataType::Type GetMethodReturnType(MethodPtr /* unused */, MethodId /* unused */) const override
    {
        return returnType_;
    }

    DataType::Type GetMethodTotalArgumentType(MethodPtr /* unused */, size_t index) const override
    {
        if (argTypes_ == nullptr || index >= argTypes_->size()) {
            return DataType::NO_TYPE;
        }
        return argTypes_->at(index);
    }

    size_t GetMethodTotalArgumentsCount(MethodPtr /* unused */) const override
    {
        if (argTypes_ == nullptr) {
            return argsCount_;
        }
        return argTypes_->size();
    }

    size_t GetMethodArgumentsCount(MethodPtr /* unused */) const override
    {
        return argsCount_;
    }

    size_t GetMethodRegistersCount(MethodPtr /* unused */) const override
    {
        return vregsCount_;
    }

    DataType::Type GetFieldType(PandaRuntimeInterface::FieldPtr field) const override
    {
        return fieldTypes_ == nullptr ? DataType::NO_TYPE : (*fieldTypes_)[field];
    }

    DataType::Type GetArrayComponentType(PandaRuntimeInterface::ClassPtr klass) const override
    {
        return arrayComponentTypes_ == nullptr ? DataType::NO_TYPE : (*arrayComponentTypes_)[klass];
    }

    PandaRuntimeInterface::ClassPtr GetClass([[maybe_unused]] MethodPtr method,
                                             [[maybe_unused]] IdType id) const override
    {
        return classes_ == nullptr ? nullptr : (*classes_)[id];
    }

    bool CanUseStringFlatCheck() const override
    {
        return true;
    }

private:
    static constexpr uintptr_t METHOD = 0xdead;
    static constexpr uintptr_t CALLEE = 0xdeadc;

    size_t argsCount_ {0};
    size_t vregsCount_ {0};
    mutable unsigned savedCalleeId_ {0};
    DataType::Type returnType_ {DataType::NO_TYPE};
    ArenaVector<DataType::Type> *argTypes_ {nullptr};
    ArenaUnorderedMap<PandaRuntimeInterface::FieldPtr, DataType::Type> *fieldTypes_ {nullptr};
    ArenaUnorderedMap<PandaRuntimeInterface::ClassPtr, DataType::Type> *arrayComponentTypes_ {nullptr};
    ArenaUnorderedMap<IdType, PandaRuntimeInterface::ClassPtr> *classes_ {nullptr};

    friend class GraphTest;
    friend class GraphCreator;
    friend class InstGenerator;
};

class CommonTest : public ::testing::Test {
public:
    CommonTest()
    {
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_32)
        // We have issue with QEMU - so reduce memory heap
        ark::mem::MemConfig::Initialize(32_MB, 64_MB, 200_MB, 32_MB, 0, 0);  // NOLINT(readability-magic-numbers)
#else
        ark::mem::MemConfig::Initialize(32_MB, 64_MB, 256_MB, 32_MB, 0, 0);  // NOLINT(readability-magic-numbers)
#endif
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        objectAllocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_OBJECT);
        localAllocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        builder_ = new IrConstructor();
    }
    ~CommonTest() override;

    NO_COPY_SEMANTIC(CommonTest);
    NO_MOVE_SEMANTIC(CommonTest);

    ArenaAllocator *GetAllocator() const
    {
        return allocator_;
    }

    ArenaAllocator *GetObjectAllocator() const
    {
        return objectAllocator_;
    }

    ArenaAllocator *GetLocalAllocator() const
    {
        return localAllocator_;
    }

    Arch GetArch() const
    {
        return arch_;
    }

    Graph *CreateEmptyGraph(bool isOsr = false) const
    {
        return GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_, isOsr);
    }

    Graph *CreateEmptyGraph(Arch arch) const
    {
        return GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch, false);
    }

    Graph *CreateOsrGraph() const
    {
        return GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_, true);
    }

    Graph *CreateGraphStartEndBlocks(bool isDynamic = false) const
    {
        auto graph = GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_, isDynamic, false);
        graph->CreateStartBlock();
        graph->CreateEndBlock();
        return graph;
    }
    Graph *CreateDynEmptyGraph() const
    {
        return GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_, true, false);
    }
    Graph *CreateEmptyBytecodeGraph() const
    {
        return GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), Arch::NONE, false, true);
    }
    Graph *CreateEmptyFastpathGraph(Arch arch) const
    {
        auto graph = GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch, false);
        graph->SetMode(GraphMode::FastPath());
        return graph;
    }

    BasicBlock *CreateEmptyBlock(Graph *graph) const
    {
        auto block = graph->GetAllocator()->New<BasicBlock>(graph);
        graph->AddBlock(block);
        return block;
    }

    ArenaVector<BasicBlock *> GetBlocksById(Graph *graph, std::vector<size_t> &&ids) const
    {
        ArenaVector<BasicBlock *> blocks(graph->GetAllocator()->Adapter());
        for (auto id : ids) {
            blocks.push_back(&BB(id));
        }
        return blocks;
    }

    bool CheckInputs(Inst &inst, std::initializer_list<size_t> list) const
    {
        if (inst.GetInputs().Size() != list.size()) {
            return false;
        }
        auto it2 = list.begin();
        for (auto it = inst.GetInputs().begin(); it != inst.GetInputs().end(); ++it, ++it2) {
            if (it->GetInst() != &INS(*it2)) {
                return false;
            }
        }
        return true;
    }

    bool CheckUsers(Inst &inst, std::initializer_list<int> list) const
    {
        std::unordered_map<int, size_t> usersMap;
        for (auto l : list) {
            ++usersMap[l];
        }
        for (auto &user : inst.GetUsers()) {
            EXPECT_EQ(user.GetInst()->GetInput(user.GetIndex()).GetInst(), &inst);
            if (usersMap[user.GetInst()->GetId()]-- == 0) {
                return false;
            }
        }
        auto rest = std::accumulate(usersMap.begin(), usersMap.end(), 0, [](int a, auto &x) { return a + x.second; });
        EXPECT_EQ(rest, 0);
        return rest == 0;
    }

protected:
    IrConstructor *builder_;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    ArenaAllocator *allocator_;
    ArenaAllocator *objectAllocator_;
    ArenaAllocator *localAllocator_;
#ifdef PANDA_TARGET_ARM32
    Arch arch_ {Arch::AARCH32};
#else
    Arch arch_ {Arch::AARCH64};
#endif
};

class GraphTest : public CommonTest {
public:
    GraphTest() : graph_(CreateEmptyGraph())
    {
        graph_->SetRuntime(&runtime_);
        runtime_.fieldTypes_ =
            graph_->GetAllocator()->New<ArenaUnorderedMap<PandaRuntimeInterface::FieldPtr, DataType::Type>>(
                graph_->GetAllocator()->Adapter());

        runtime_.arrayComponentTypes_ =
            graph_->GetAllocator()->New<ArenaUnorderedMap<PandaRuntimeInterface::ClassPtr, DataType::Type>>(
                graph_->GetAllocator()->Adapter());

        runtime_.classes_ =
            graph_->GetAllocator()
                ->New<ArenaUnorderedMap<PandaRuntimeInterface::IdType, PandaRuntimeInterface::ClassPtr>>(
                    graph_->GetAllocator()->Adapter());  // CC-OFF(G.FMT.06-CPP) project code style
    }
    ~GraphTest() override = default;

    NO_MOVE_SEMANTIC(GraphTest);
    NO_COPY_SEMANTIC(GraphTest);

    Graph *GetGraph() const
    {
        return graph_;
    }

    void SetGraphArch(Arch arch)
    {
        graph_ = CreateEmptyGraph(arch);
        graph_->SetRuntime(&runtime_);
    }

    void ResetGraph()
    {
        graph_ = CreateEmptyGraph();
        graph_->SetRuntime(&runtime_);
    }

    void SetNumVirtRegs(size_t num)
    {
        runtime_.vregsCount_ = num;
        graph_->SetVRegsCount(std::max(graph_->GetVRegsCount(), runtime_.vregsCount_ + runtime_.argsCount_ + 1));
    }

    void SetNumArgs(size_t num)
    {
        runtime_.argsCount_ = num;
        graph_->SetVRegsCount(std::max(graph_->GetVRegsCount(), runtime_.vregsCount_ + runtime_.argsCount_ + 1));
    }

    void RegisterFieldType(PandaRuntimeInterface::FieldPtr field, DataType::Type type)
    {
        (*runtime_.fieldTypes_)[field] = type;
    }

    void RegisterArrayComponentType(PandaRuntimeInterface::ClassPtr klass, DataType::Type type)
    {
        (*runtime_.arrayComponentTypes_)[klass] = type;
    }

    void RegisterClass(PandaRuntimeInterface::IdType id, PandaRuntimeInterface::ClassPtr klass)
    {
        (*runtime_.classes_)[id] = klass;
    }

protected:
    RuntimeInterfaceMock runtime_;  // NOLINT(misc-non-private-member-variables-in-classes)
    Graph *graph_ {nullptr};        // NOLINT(misc-non-private-member-variables-in-classes)
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PandaRuntimeTest : public ::testing::Test, public PandaRuntimeInterface {
public:
    PandaRuntimeTest();
    ~PandaRuntimeTest() override;
    NO_MOVE_SEMANTIC(PandaRuntimeTest);
    NO_COPY_SEMANTIC(PandaRuntimeTest);

    static void Initialize(int argc, char **argv);

    Graph *CreateGraph()
    {
        auto graph = GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_);
        graph->SetRuntime(this);
        return graph;
    }

    Graph *CreateGraphOsr()
    {
        Graph *graph = GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_, true);
        graph->SetRuntime(this);
        return graph;
    }

    // this method is needed to create a graph with a working dump
    Graph *CreateGraphWithDefaultRuntime()
    {
        auto *graph = GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_);
        graph->SetRuntime(GetDefaultRuntime());
        return graph;
    }

    Graph *CreateGraphDynWithDefaultRuntime()
    {
        auto *graph = GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_);
        graph->SetRuntime(GetDefaultRuntime());
        graph->SetDynamicMethod();
#ifndef NDEBUG
        graph->SetDynUnitTestFlag();
#endif
        return graph;
    }

    Graph *CreateGraphDynStubWithDefaultRuntime()
    {
        auto *graph = GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_);
        graph->SetRuntime(GetDefaultRuntime());
        graph->SetDynamicMethod();
        graph->SetDynamicStub();
#ifndef NDEBUG
        graph->SetDynUnitTestFlag();
#endif
        return graph;
    }

    // this method is needed to create a graph with a working dump
    Graph *CreateGraphOsrWithDefaultRuntime()
    {
        auto *graph = GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_, true);
        graph->SetRuntime(GetDefaultRuntime());
        return graph;
    }

    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }

    ArenaAllocator *GetLocalAllocator()
    {
        return localAllocator_;
    }

    virtual Graph *GetGraph()
    {
        return graph_;
    }

    auto GetClassLinker()
    {
        return ark::Runtime::GetCurrent()->GetClassLinker();
    }

    void EnableLogs(Logger::Level level = Logger::Level::DEBUG)
    {
        Logger::EnableComponent(Logger::Component::COMPILER);
        Logger::SetLevel(level);
    }
    const char *GetExecPath() const
    {
        return execPath_;
    }
    static RuntimeInterface *GetDefaultRuntime();

protected:
    IrConstructor *builder_;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Graph *graph_ {nullptr};
    ArenaAllocator *allocator_ {nullptr};
    ArenaAllocator *localAllocator_ {nullptr};
    static inline const char *execPath_ {nullptr};
    Arch arch_ {RUNTIME_ARCH};
};

// NOLINTNEXTLINE(fuchsia-multia-inheritance,cppcoreguidelines-special-member-functions)
class AsmTest : public PandaRuntimeTest {
public:
    std::unique_ptr<const panda_file::File> ParseToFile(const char *source, const char *fileName = "test.pb");
    bool Parse(const char *source, const char *fileName = "test.pb");
    Graph *BuildGraph(const char *methodName, Graph *graph = nullptr);
    void CleanUp(Graph *graph);

    AsmTest() = default;
    ~AsmTest() override = default;
    NO_MOVE_SEMANTIC(AsmTest);
    NO_COPY_SEMANTIC(AsmTest);

    template <bool WITH_CLEANUP = false>
    bool ParseToGraph(const char *source, const char *methodName, Graph *graph = nullptr)
    {
        if (!Parse(source)) {
            return false;
        }
        if (graph == nullptr) {
            graph = GetGraph();
        }
        if (BuildGraph(methodName, graph) == nullptr) {
            return false;
        }
        if constexpr (WITH_CLEANUP) {
            CleanUp(graph);
        }
        return true;
    }
};

struct TmpFile {
    explicit TmpFile(const char *fileName) : fileName_(fileName) {}
    ~TmpFile()
    {
        ASSERT(fileName_ != nullptr);
        std::remove(fileName_);
    }

    NO_MOVE_SEMANTIC(TmpFile);
    NO_COPY_SEMANTIC(TmpFile);

    const char *GetFileName() const
    {
        return fileName_;
    }

private:
    const char *fileName_ {nullptr};
};
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define TEST_GRAPH(ns, testName, ...) /* CC-OFF(G.PRE.06) solid logic */              \
    namespace ns {                                                                    \
    class testName {                                                                  \
    public:                                                                           \
        template <typename... Args>                                                   \
        static void Create(IrConstructor *builder, Args &&...args)                    \
        {                                                                             \
            testName(builder, std::forward<Args>(args)...);                           \
        }                                                                             \
                                                                                      \
    private:                                                                          \
        template <typename... Args>                                                   \
        explicit testName(IrConstructor *builder, Args &&...args) : builder_(builder) \
        {                                                                             \
            BuildGraph(std::forward<Args>(args)...);                                  \
        }                                                                             \
        void BuildGraph(__VA_ARGS__);                                                 \
                                                                                      \
        auto GetBuilder()                                                             \
        {                                                                             \
            /* CC-OFFNXT(G.PRE.05) function gen */                                    \
            return builder_;                                                          \
        }                                                                             \
                                                                                      \
    private:                                                                          \
        IrConstructor *builder_ {};                                                   \
    };                                                                                \
    }                                                                                 \
    void ns::testName::BuildGraph(__VA_ARGS__)

#define CREATE(...) Create(builder_, __VA_ARGS__)
#define SRC_GRAPH(testName, ...) TEST_GRAPH(src_graph, testName, __VA_ARGS__)
#define OUT_GRAPH(testName, ...) TEST_GRAPH(out_graph, testName, __VA_ARGS__)
// NOLINTEND(cppcoreguidelines-macro-usage)

}  // namespace ark::compiler

#endif  // COMPILER_TESTS_UNIT_TEST_H
