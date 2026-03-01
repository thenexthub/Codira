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

#include <numeric>
#include <unordered_map>
#include "gtest/gtest.h"
#include "optimizer/ir/ir_constructor.h"
#include "mem/arena_allocator.h"
#include "mem/pool_manager.h"
#include "compiler.h"
#include "compiler_logger.h"
#include "graph_comparator.h"
#include "include/runtime.h"

namespace panda::compiler {
struct RuntimeInterfaceMock : public compiler::RuntimeInterface {
    DataType::Type GetMethodReturnType(MethodPtr /* unsuded */) const override
    {
        return return_type;
    }

    DataType::Type GetMethodTotalArgumentType(MethodPtr /* unused */, size_t index) const override
    {
        if (arg_types == nullptr || index >= arg_types->size()) {
            return DataType::NO_TYPE;
        }
        return arg_types->at(index);
    }

    size_t GetMethodTotalArgumentsCount(MethodPtr /* unused */) const override
    {
        if (arg_types == nullptr) {
            return args_count;
        }
        return arg_types->size();
    }
    size_t GetMethodArgumentsCount(MethodPtr /* unused */) const override
    {
        return args_count;
    }

    size_t GetMethodRegistersCount(MethodPtr /* unused */) const override
    {
        return vregs_count;
    }

    size_t args_count {0};
    size_t vregs_count {0};
    DataType::Type return_type {DataType::NO_TYPE};
    ArenaVector<DataType::Type> *arg_types {nullptr};
};

class CommonTest : public ::testing::Test {
public:
    CommonTest()
    {
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_32)
        // We have issue with QEMU - so reduce memory heap
        panda::mem::MemConfig::Initialize(32_MB, 64_MB, 200_MB, 32_MB);
#else
        panda::mem::MemConfig::Initialize(32_MB, 64_MB, 256_MB, 32_MB);
#endif
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        object_allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_OBJECT);
        local_allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        builder_ = new IrConstructor();
    }
    ~CommonTest() override;

    ArenaAllocator *GetAllocator() const
    {
        return allocator_;
    }

    ArenaAllocator *GetObjectAllocator() const
    {
        return object_allocator_;
    }

    ArenaAllocator *GetLocalAllocator() const
    {
        return local_allocator_;
    }

    Arch GetArch() const
    {
        return arch_;
    }

    Graph *CreateEmptyGraph(bool is_osr = false) const
    {
        return GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_, is_osr);
    }

    Graph *CreateEmptyGraph(Arch arch) const
    {
        return GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch, false);
    }

    Graph *CreateGraphStartEndBlocks(bool is_dynamic = false) const
    {
        auto graph = GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_, is_dynamic, false);
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
        std::unordered_map<int, size_t> users_map;
        for (auto l : list)
            ++users_map[l];
        for (auto &user : inst.GetUsers()) {
            EXPECT_EQ(user.GetInst()->GetInput(user.GetIndex()).GetInst(), &inst);
            if (users_map[user.GetInst()->GetId()]-- == 0)
                return false;
        }
        auto rest = std::accumulate(users_map.begin(), users_map.end(), 0, [](int a, auto &x) { return a + x.second; });
        EXPECT_EQ(rest, 0);
        return (rest == 0) ? true : false;
    }

protected:
    IrConstructor *builder_;

private:
    ArenaAllocator *allocator_;
    ArenaAllocator *object_allocator_;
    ArenaAllocator *local_allocator_;
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
    }
    ~GraphTest() override {}

    Graph *GetGraph() const
    {
        return graph_;
    }

    void ResetGraph()
    {
        graph_ = CreateEmptyGraph();
        graph_->SetRuntime(&runtime_);
    }

    void SetNumVirtRegs(size_t num)
    {
        runtime_.vregs_count = num;
        graph_->SetVRegsCount(std::max(graph_->GetVRegsCount(), runtime_.vregs_count + runtime_.args_count + 1));
    }

    void SetNumArgs(size_t num)
    {
        runtime_.args_count = num;
        graph_->SetVRegsCount(std::max(graph_->GetVRegsCount(), runtime_.vregs_count + runtime_.args_count + 1));
    }

protected:
    RuntimeInterfaceMock runtime_;
    Graph *graph_ {nullptr};
};

class PandaRuntimeTest : public ::testing::Test, public PandaRuntimeInterface {
public:
    PandaRuntimeTest();

    ~PandaRuntimeTest() override;

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
        return local_allocator_;
    }

    virtual Graph *GetGraph()
    {
        return graph_;
    }

    auto GetClassLinker()
    {
        return panda::Runtime::GetCurrent()->GetClassLinker();
    }

    void EnableLogs(Logger::Level level = Logger::Level::DEBUG)
    {
        Logger::EnableComponent(Logger::Component::COMPILER);
        Logger::SetLevel(level);
    }

    const char *GetExecPath() const
    {
        return exec_path_;
    }

    static RuntimeInterface *GetDefaultRuntime();

protected:
    IrConstructor *builder_;

private:
    Graph *graph_ {nullptr};
    ArenaAllocator *allocator_ {nullptr};
    ArenaAllocator *local_allocator_ {nullptr};
    static inline const char *exec_path_ {nullptr};
    Arch arch_ {RUNTIME_ARCH};
};

class AsmTest : public PandaRuntimeTest {
public:
    std::unique_ptr<const panda_file::File> ParseToFile(const char *source, const char *file_name = "test.pb");
    bool Parse(const char *source, const char *file_name = "test.pb");
    Graph *BuildGraph(const char *method_name, Graph *graph = nullptr);
    void CleanUp(Graph *graph);

    virtual ~AsmTest() = default;

    template <bool with_cleanup = false>
    bool ParseToGraph(const char *source, const char *method_name, Graph *graph = nullptr)
    {
        if (!Parse(source)) {
            return false;
        }
        if (graph == nullptr) {
            graph = GetGraph();
        }
        if (BuildGraph(method_name, graph) == nullptr) {
            return false;
        }
        if constexpr (with_cleanup) {
            CleanUp(graph);
        }
        return true;
    }
};

struct TmpFile {
    explicit TmpFile(const char *file_name) : file_name_(file_name) {}
    ~TmpFile()
    {
        ASSERT(file_name_ != nullptr);
        remove(file_name_);
    }
    const char *GetFileName() const
    {
        return file_name_;
    }

private:
    const char *file_name_ {nullptr};
};
}  // namespace panda::compiler

#endif  // COMPILER_TESTS_UNIT_TEST_H
