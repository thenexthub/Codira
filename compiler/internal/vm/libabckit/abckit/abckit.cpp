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

#include "libabckit/c/abckit.h"
#include "utils/pandargs.h"
#include "libabckit/src/abckit_options.h"

#include <dlfcn.h>
#include <iostream>

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

static void PrintHelp(panda::PandArgParser &paParser)
{
    std::cerr << "Usage:" << std::endl;
    std::cerr << "abckit --plugin-path <libabckitplugin.so> --input-file <input.abc> [--output-file <output.abc>]"
              << std::endl;
    std::cerr << "Supported options:" << std::endl;
    std::cerr << paParser.GetHelpString() << std::endl;
}

static bool ProcessArgs(panda::PandArgParser &paParser, int /*argc*/, const char ** /*argv*/)
{
    if (libabckit::g_abckitOptions.GetInputFile().empty()) {
        std::cerr << "ERROR: --input-file is required\n";
        PrintHelp(paParser);
        return false;
    }

    if (!fs::exists(libabckit::g_abckitOptions.GetInputFile())) {
        std::cerr << "ERROR: file doesn't exist: " << libabckit::g_abckitOptions.GetInputFile() << '\n';
        PrintHelp(paParser);
        return false;
    }

    if (libabckit::g_abckitOptions.GetPluginPath().empty()) {
        std::cerr << "ERROR: --plugin-path is required\n";
        PrintHelp(paParser);
        return false;
    }

    if (!fs::exists(libabckit::g_abckitOptions.GetPluginPath())) {
        std::cerr << "ERROR: file doesn't exist: " << libabckit::g_abckitOptions.GetPluginPath() << '\n';
        PrintHelp(paParser);
        return false;
    }

    return true;
}

int main(int argc, const char **argv)
{
    panda::PandArgParser paParser;
    libabckit::g_abckitOptions.AddOptions(&paParser);
    if (!paParser.Parse(argc, argv)) {
        std::cerr << "Error: " << paParser.GetErrorString() << "\n";
        PrintHelp(paParser);
        return 1;
    }

    if (!ProcessArgs(paParser, argc, argv)) {
        return 1;
    }

    if (libabckit::g_abckitOptions.IsHelp()) {
        PrintHelp(paParser);
        return 0;
    }

    void *handler = dlopen(libabckit::g_abckitOptions.GetPluginPath().c_str(), RTLD_LAZY);
    if (handler == nullptr) {
        std::cerr << "ERROR: failed load plugin: " << libabckit::g_abckitOptions.GetPluginPath() << '\n';
        return 1;
    }
    auto *entry = reinterpret_cast<int (*)(AbckitFile *)>(dlsym(handler, "Entry"));
    if (entry == nullptr) {
        std::cerr << "ERROR: failed find symbol 'Entry'\n";
        return 1;
    }

    auto inputPath = libabckit::g_abckitOptions.GetInputFile();
    AbckitFile *file = g_impl->openAbc(inputPath.c_str(), inputPath.size());
    if (g_impl->getLastError() != ABCKIT_STATUS_NO_ERROR || file == nullptr) {
        std::cerr << "ERROR: failed to open: " << inputPath << '\n';
        return 1;
    }

    int ret = entry(file);
    if (ret != 0) {
        std::cerr << "ERROR: plugin returned non-zero return code";
        return ret;
    }

    auto outputPath = libabckit::g_abckitOptions.GetOutputFile();
    if (!outputPath.empty()) {
        g_impl->writeAbc(file, outputPath.c_str(), outputPath.size());
        if (g_impl->getLastError() != ABCKIT_STATUS_NO_ERROR || file == nullptr) {
            std::cerr << "ERROR: failed to write: " << outputPath << '\n';
            return 1;
        }
        g_impl->closeFile(file);
    }

    return 0;
}
