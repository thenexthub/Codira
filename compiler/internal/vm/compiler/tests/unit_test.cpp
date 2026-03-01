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

#include "macros.h"
#if defined(PANDA_TARGET_MOBILE)
#elif defined(USE_STD_FILESYSTEM)
#include <filesystem>
#else
#include <experimental/filesystem>
#endif
#include "optimizer/ir_builder/ir_builder.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/code_generator/encode.h"
#include "include/class_linker.h"
#include "assembly-parser.h"
#include "include/runtime.h"
#include "compiler.h"
#include "utils/expected.h"
#include "compiler_options.h"
#include "unit_test.h"

#include "utils/utf.h"

namespace panda::compiler {
void PandaRuntimeTest::Initialize([[maybe_unused]] int argc, char **argv)
{
    ASSERT(argc > 0);
    exec_path_ = argv[0];
}

PandaRuntimeTest::PandaRuntimeTest()
{
    ASSERT(exec_path_ != nullptr);
#if !defined(PANDA_TARGET_MOBILE)
#if defined(USE_STD_FILESYSTEM)
    std::filesystem::path exec_name(exec_path_);
#else
    std::experimental::filesystem::path exec_name(exec_path_);
#endif  // defined(USE_STD_FILESYSTEM)
    std::string pandastdlib_path = exec_name.parent_path() / "../pandastdlib/arkstdlib.abc";
#else
    std::string exec_name = "compiler_unit_tests";
    std::string pandastdlib_path = "../pandastdlib/arkstdlib.abc";
#endif
    panda::RuntimeOptions runtime_options(exec_name);
    runtime_options.SetBootPandaFiles({pandastdlib_path});
    runtime_options.SetLoadRuntimes({"core"});
    runtime_options.SetHeapSizeLimit(50_MB);
    runtime_options.SetEnableAn(true);
    runtime_options.SetGcType("epsilon");
    Logger::InitializeDummyLogging();
    EXPECT_TRUE(panda::Runtime::Create(runtime_options));

    allocator_ = new ArenaAllocator(panda::SpaceType::SPACE_TYPE_INTERNAL);
    local_allocator_ = new ArenaAllocator(panda::SpaceType::SPACE_TYPE_INTERNAL);
    builder_ = new IrConstructor();

    graph_ = CreateGraph();
}

PandaRuntimeTest::~PandaRuntimeTest()
{
    delete builder_;
    delete allocator_;
    delete local_allocator_;
    panda::Runtime::Destroy();
}

RuntimeInterface *PandaRuntimeTest::GetDefaultRuntime()
{
    return Graph::GetDefaultRuntime();
}

std::unique_ptr<const panda_file::File> AsmTest::ParseToFile(const char *source, const char *file_name)
{
    panda::pandasm::Parser parser;
    auto res = parser.Parse(source, file_name);
    if (parser.ShowError().err != pandasm::Error::ErrorType::ERR_NONE) {
        std::cerr << "Parse failed: " << parser.ShowError().message << std::endl
                  << parser.ShowError().whole_line << std::endl;
        ADD_FAILURE();
        return nullptr;
    }
    return pandasm::AsmEmitter::Emit(res.Value());
}

bool AsmTest::Parse(const char *source, const char *file_name)
{
    auto pfile = ParseToFile(source, file_name);
    if (pfile == nullptr) {
        ADD_FAILURE();
        return false;
    }
    GetClassLinker()->AddPandaFile(std::move(pfile));
    return true;
}

Graph *AsmTest::BuildGraph(const char *method_name, Graph *graph)
{
    auto loader = GetClassLinker();
    PandaString storage;
    auto extension = loader->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *thread = MTManagedThread::GetCurrent();
    thread->ManagedCodeBegin();
    auto klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &storage));
    thread->ManagedCodeEnd();

    auto method = klass->GetDirectMethod(utf::CStringAsMutf8(method_name));
    if (method == nullptr) {
        ADD_FAILURE();
        return nullptr;
    }
    if (graph == nullptr) {
        graph = CreateGraph();
    }
    graph->SetMethod(method);
    if (!graph->RunPass<IrBuilder>()) {
        ADD_FAILURE();
        return nullptr;
    }
    return graph;
}

void AsmTest::CleanUp(Graph *graph)
{
    graph->RunPass<Cleanup>();
}

CommonTest::~CommonTest()
{
    // Look at examples in encoder_constructors tests.
    // Used for destroy MasmHolder.
    Encoder *encoder = Encoder::Create(allocator_, arch_, false);
    if (encoder != nullptr) {
        encoder->~Encoder();
    }
    delete builder_;
    delete allocator_;
    delete object_allocator_;
    delete local_allocator_;
    PoolManager::Finalize();

    panda::mem::MemConfig::Finalize();
}
}  // namespace panda::compiler

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    panda::compiler::PandaRuntimeTest::Initialize(argc, argv);
    panda::compiler::options.SetCompilerUseSafepoint(false);
    return RUN_ALL_TESTS();
}
