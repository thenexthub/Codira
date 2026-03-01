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

#include "libabckit/c/statuses.h"
#include "libabckit/src/macros.h"
#include "libabckit/src/helpers_common.h"
#include "libabckit/src/statuses_impl.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/ir_impl.h"
#include "libabckit/src/adapter_dynamic/abckit_dynamic.h"
#include "libabckit/src/adapter_static/abckit_static.h"
#include "libabckit/src/mem_manager/mem_manager.h"
#include "libabckit/src/logger.h"
#include "libabckit/src/abckit_options.h"
#include "scoped_timer.h"

#include <cstddef>
#include <iostream>

libabckit::Logger *libabckit::Logger::logger_ = nullptr;
thread_local libabckit::NullBuffer libabckit::g_nB {};
thread_local std::ostream libabckit::g_nullStream {&g_nB};

namespace libabckit {
libabckit::Options g_abckitOptions("");

extern "C" AbckitStatus GetAbckitLastError()
{
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    return statuses::GetLastError();
}

extern "C" AbckitFile *OpenAbc(const char *path, size_t len)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(path, nullptr);
    LIBABCKIT_ZERO_ARGUMENT(len, nullptr);

    auto spath = std::string(path, len);
    LIBABCKIT_LOG(DEBUG) << spath << '\n';

    MemManager::Initialize(g_abckitOptions.GetMemorySizeLimit());

    AbckitFile *file = nullptr;

    file = OpenAbcStatic(path, len);
    if (file != nullptr) {
        return file;
    }

    LIBABCKIT_LOG(DEBUG) << (std::string() + "Load file with path '" + spath +
                             "' failed for static mode, trying dynamic mode\n");

    file = OpenAbcDynamic(path, len);
    if (file != nullptr) {
        return file;
    }

    LIBABCKIT_LOG(DEBUG) << (std::string() + "Load file with path '" + spath + "' failed for both modes\n");

    // Input abc file is neither for dynamic vm nor static vm
    statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);

    return nullptr;
}

extern "C" void WriteAbc(AbckitFile *file, const char *path, size_t len)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(file)
    LIBABCKIT_BAD_ARGUMENT_VOID(path)
    LIBABCKIT_ZERO_ARGUMENT_VOID(len)

    switch (file->frontend) {
        case Mode::DYNAMIC:
            return WriteAbcDynamic(file, path, len);
        case Mode::STATIC:
            return WriteAbcStatic(file, path, len);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" void CloseFile(AbckitFile *file)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(file);

    switch (file->frontend) {
        case Mode::DYNAMIC:
            return CloseFileDynamic(file);
        case Mode::STATIC:
            return CloseFileStatic(file);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" void DestroyGraph(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(graph);

    if (IsDynamic(graph->function->owningModule->target)) {
        return DestroyGraphDynamic(graph);
    }
    DestroyGraphStatic(graph);
}

AbckitApi g_impl = {
    // ========================================
    // Common API
    // ========================================

    ABCKIT_VERSION_RELEASE_1_0_0,
    GetAbckitLastError,

    // ========================================
    // Inspection API entrypoints
    // ========================================

    OpenAbc,
    WriteAbc,
    CloseFile,

    // ========================================
    // IR API entrypoints
    // ========================================

    DestroyGraph,
};

}  // namespace libabckit

#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
#include "./mock/abckit_mock.h"
#endif

extern "C" AbckitApi const *AbckitGetApiImpl(AbckitApiVersion version)
{
#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
    return AbckitGetMockApiImpl(version);
#endif
    switch (version) {
        case ABCKIT_VERSION_RELEASE_1_0_0:
            return &libabckit::g_impl;
        default:
            libabckit::statuses::SetLastError(ABCKIT_STATUS_UNKNOWN_API_VERSION);
            return nullptr;
    }
}
