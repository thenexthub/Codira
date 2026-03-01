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

#include "include/runtime.h"
#include "include/thread.h"
#include "include/thread_scopes.h"
#include "runtime/include/locks.h"
#include "runtime/include/method-inl.h"
#include "runtime/include/class.h"
#include "libarkbase/utils/pandargs.h"
#include "compiler/compiler_options.h"
#include "compiler/compiler_logger.h"
#include "compiler_events_gen.h"
#include "mem/mem_stats.h"
#include "libarkbase/os/mutex.h"
#include "libarkbase/os/native_stack.h"
#include "libarkbase/panda_gen_options/generated/logger_options.h"

#include "libarkbase/generated/ark_version.h"

#include "libarkbase/utils/span.h"

#include "libarkbase/utils/logger.h"

#include <limits>
#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <csignal>

#ifdef ARK_HYBRID
#include <node_api.h>
#endif

namespace ark {
const panda_file::File *GetPandaFile(const ClassLinker &classLinker, std::string_view fileName)
{
    const panda_file::File *res = nullptr;
    classLinker.EnumerateBootPandaFiles([&res, fileName](const panda_file::File &pf) {
        if (pf.GetFilename() == fileName) {
            res = &pf;
            return false;
        }
        return true;
    });
    return res;
}

static void PrintHelp(const ark::PandArgParser &paParser, bool showFlags = false)
{
    if (!paParser.GetErrorString().empty()) {
        std::cerr << paParser.GetErrorString();
        std::cerr << "Use ark --help to get more information\n";
        return;
    }

    std::cerr << "Usage: ark [OPTIONS] file entrypoint -- [arguments]\n\n";
    if (showFlags) {
        std::cerr << "optional arguments:" << std::endl;
        std::cerr << paParser.GetHelpString() << std::endl;
    }

    std::cerr << paParser.GetTailArgs() << std::endl;
    if (!showFlags) {
        std::cerr << "Use ark --help to get more information\n";
    }
}

static bool PrepareArguments(ark::PandArgParser *paParser, const RuntimeOptions &runtimeOptions,
                             const ark::PandArg<std::string> &file, const ark::PandArg<std::string> &entrypoint,
                             const ark::PandArg<bool> &help)
{
    if (runtimeOptions.IsVersion()) {
        PrintPandaVersion();
        return false;
    }

    if (file.GetValue().empty() || entrypoint.GetValue().empty() || help.GetValue()) {
        PrintHelp(*paParser, help.GetValue());
        return false;
    }

    auto runtimeOptionsErr = runtimeOptions.Validate();
    if (runtimeOptionsErr) {
        std::cerr << "Error: " << runtimeOptionsErr.value().GetMessage() << std::endl;
        return false;
    }

    auto compilerOptionsErr = compiler::g_options.Validate();
    if (compilerOptionsErr) {
        std::cerr << "Error: " << compilerOptionsErr.value().GetMessage() << std::endl;
        return false;
    }

    return true;
}

static void SetPandaFiles(RuntimeOptions &runtimeOptions, ark::PandArg<std::string> &file)
{
    const std::string &fileName = file.GetValue();
    auto bootPandaFiles = runtimeOptions.GetBootPandaFiles();
    auto pandaFiles = runtimeOptions.GetPandaFiles();
    auto bootFoundIter = std::find(bootPandaFiles.begin(), bootPandaFiles.end(), fileName);
    if (runtimeOptions.IsLoadInBoot() && bootFoundIter == bootPandaFiles.end()) {
        bootPandaFiles.push_back(fileName);
        runtimeOptions.SetBootPandaFiles(bootPandaFiles);
        return;
    }

    if (pandaFiles.empty() && bootFoundIter == bootPandaFiles.end()) {
        pandaFiles.push_back(fileName);
        runtimeOptions.SetPandaFiles(pandaFiles);
        return;
    }

    auto pandaFoundIter = std::find(pandaFiles.begin(), pandaFiles.end(), fileName);
    if (pandaFoundIter == pandaFiles.end()) {
        pandaFiles.push_back(fileName);
        runtimeOptions.SetPandaFiles(pandaFiles);
    }
}

static ark::PandArgParser GetPandArgParser(ark::PandArg<bool> &help, ark::PandArg<bool> &options,
                                           ark::PandArg<std::string> &file, ark::PandArg<std::string> &entrypoint)
{
    ark::PandArgParser paParser;

    paParser.Add(&help);
    paParser.Add(&options);
    paParser.PushBackTail(&file);
    paParser.PushBackTail(&entrypoint);
    paParser.EnableTail();
    paParser.EnableRemainder();

    return paParser;
}

static int ExecutePandaFile(Runtime &runtime, const std::string &fileName, const std::string &entry,
                            const arg_list_t &arguments)
{
    auto res = runtime.ExecutePandaFile(fileName, entry, arguments);
    if (!res) {
        std::cerr << "Cannot execute panda file '" << fileName << "' with entry '" << entry << "'" << std::endl;
        return -1;
    }

    return res.Value();
}

static void PrintStatistics(RuntimeOptions &runtimeOptions, Runtime &runtime)
{
    if (runtimeOptions.IsPrintMemoryStatistics()) {
        std::cout << runtime.GetMemoryStatistics();
    }
    if (runtimeOptions.IsPrintGcStatistics()) {
        std::cout << runtime.GetFinalStatistics();
    }
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
int Main(int argc, const char **argv)
{
    Span<const char *> sp(argv, argc);
    RuntimeOptions runtimeOptions(sp[0]);
    logger::Options loggerOptions(sp[0]);

    ark::PandArg<bool> help("help", false, "Print this message and exit");
    ark::PandArg<bool> options("options", false, "Print compiler and runtime options");
    // tail arguments
    ark::PandArg<std::string> file("file", "", "path to pandafile");
    ark::PandArg<std::string> entrypoint("entrypoint", "", "full name of entrypoint function or method");

    ark::PandArgParser paParser = GetPandArgParser(help, options, file, entrypoint);

    runtimeOptions.AddOptions(&paParser);
    loggerOptions.AddOptions(&paParser);
    compiler::g_options.AddOptions(&paParser);

    auto startTime =
        std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();

    if (!paParser.Parse(argc, argv)) {
        PrintHelp(paParser);
        return 1;
    }

    if (runtimeOptions.IsStartupTime()) {
        // clang-format off
        std::cout << "\n" << "Startup start time: " << startTime << std::endl;
        // clang-format on
    }

    if (!ark::PrepareArguments(&paParser, runtimeOptions, file, entrypoint, help)) {
        return 1;
    }

    compiler::g_options.AdjustCpuFeatures(false);

    Logger::Initialize(loggerOptions);

    ark::compiler::CompilerLogger::SetComponents(ark::compiler::g_options.GetCompilerLog());
    if (compiler::g_options.IsCompilerEnableEvents()) {
        ark::compiler::EventWriter::Init(ark::compiler::g_options.GetCompilerEventsPath());
    }

    SetPandaFiles(runtimeOptions, file);

#ifdef ARK_HYBRID
    // This workaround is needed to define weak symbols of napi in hybrid libarkruntime.so
    // It will be removed after #26269 fix
    volatile bool initNapi = false;
    if (initNapi) {
        napi_module_register(nullptr);
    }
#endif

    if (!Runtime::Create(runtimeOptions)) {
        std::cerr << "Error: cannot create runtime" << std::endl;
        return -1;
    }

    if (options.GetValue()) {
        std::cout << paParser.GetRegularArgs() << std::endl;
    }

    auto &runtime = *Runtime::GetCurrent();

    int ret = ExecutePandaFile(runtime, file.GetValue(), entrypoint.GetValue(), paParser.GetRemainder());
    PrintStatistics(runtimeOptions, runtime);

    if (!Runtime::Destroy()) {
        std::cerr << "Error: cannot destroy runtime" << std::endl;
        return -1;
    }
    paParser.DisableTail();
    return ret;
}
}  // namespace ark

int main(int argc, const char **argv)
{
#ifdef ARK_HYBRID
    common::BaseRuntime::GetInstance()->Init();
#endif
    int res = ark::Main(argc, argv);
#ifdef ARK_HYBRID
    common::BaseRuntime::GetInstance()->Fini();
    common::BaseRuntime::DestroyInstance();
#endif
    return res;
}
