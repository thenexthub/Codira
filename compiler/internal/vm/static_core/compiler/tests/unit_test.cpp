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

#include "libarkbase/macros.h"
#if defined(PANDA_TARGET_MOBILE)
#elif defined(USE_STD_FILESYSTEM)
#include <filesystem>
#else
#include <experimental/filesystem>
#endif
#include "unit_test.h"
#include "optimizer/ir_builder/ir_builder.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/code_generator/encode.h"
#include "include/class_linker.h"
#include "assembly-parser.h"
#include "include/runtime.h"
#include "compiler.h"
#include "libarkbase/utils/expected.h"
#include "compiler_options.h"

#include "libarkbase/utils/utf.h"

namespace ark::compiler {
void PandaRuntimeTest::Initialize([[maybe_unused]] int argc, char **argv)
{
    ASSERT(argc > 0);
    execPath_ = argv[0];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

PandaRuntimeTest::PandaRuntimeTest()
{
    ASSERT(execPath_ != nullptr);
#if !defined(PANDA_TARGET_MOBILE)
#if defined(USE_STD_FILESYSTEM)
    std::filesystem::path execName(execPath_);
#else
    std::experimental::filesystem::path execName(execPath_);
#endif  // defined(USE_STD_FILESYSTEM)
    std::string pandastdlibPath = execName.parent_path() / "../pandastdlib/arkstdlib.abc";
#else
    std::string execName = "compiler_unit_tests";
    std::string pandastdlibPath = "../pandastdlib/arkstdlib.abc";
#endif
    ark::RuntimeOptions runtimeOptions(execName);
    runtimeOptions.SetBootPandaFiles({pandastdlibPath});
    runtimeOptions.SetLoadRuntimes({"core"});
    runtimeOptions.SetHeapSizeLimit(50_MB);  // NOLINT(readability-magic-numbers)
    runtimeOptions.SetEnableAn(true);
    runtimeOptions.SetGcType("epsilon");
    Logger::InitializeDummyLogging();
    EXPECT_TRUE(ark::Runtime::Create(runtimeOptions));

    allocator_ = new ArenaAllocator(ark::SpaceType::SPACE_TYPE_INTERNAL);
    localAllocator_ = new ArenaAllocator(ark::SpaceType::SPACE_TYPE_INTERNAL);
    builder_ = new IrConstructor();

    graph_ = CreateGraph();
}

PandaRuntimeTest::~PandaRuntimeTest()
{
    delete builder_;
    delete allocator_;
    delete localAllocator_;
    ark::Runtime::Destroy();
}

RuntimeInterface *PandaRuntimeTest::GetDefaultRuntime()
{
    return Graph::GetDefaultRuntime();
}

std::unique_ptr<const panda_file::File> AsmTest::ParseToFile(const char *source, const char *fileName)
{
    ark::pandasm::Parser parser;
    auto res = parser.Parse(source, fileName);
    if (parser.ShowError().err != pandasm::Error::ErrorType::ERR_NONE) {
        std::cerr << "Parse failed: " << parser.ShowError().message << std::endl
                  << parser.ShowError().wholeLine << std::endl;
        ADD_FAILURE();
        return nullptr;
    }
    return pandasm::AsmEmitter::Emit(res.Value());
}

bool AsmTest::Parse(const char *source, const char *fileName)
{
    auto pfile = ParseToFile(source, fileName);
    if (pfile == nullptr) {
        ADD_FAILURE();
        return false;
    }
    GetClassLinker()->AddPandaFile(std::move(pfile));
    return true;
}

Graph *AsmTest::BuildGraph(const char *methodName, Graph *graph)
{
    auto loader = GetClassLinker();
    PandaString storage;
    auto extension = loader->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *thread = MTManagedThread::GetCurrent();
    thread->ManagedCodeBegin();
    auto klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &storage));
    thread->ManagedCodeEnd();

    auto method = klass->GetDirectMethod(utf::CStringAsMutf8(methodName));
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
    delete objectAllocator_;
    delete localAllocator_;
    PoolManager::Finalize();

    ark::mem::MemConfig::Finalize();
}
}  // namespace ark::compiler

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    ark::compiler::PandaRuntimeTest::Initialize(argc, argv);
    ark::compiler::g_options.SetCompilerUseSafepoint(false);
    return RUN_ALL_TESTS();
}
