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

#ifndef LIBLLVMBACKEND_COMPILER_INTERFACE_H
#define LIBLLVMBACKEND_COMPILER_INTERFACE_H

#include <string>
#include <vector>

#include "libarkbase/utils/expected.h"

namespace ark::compiler {
class Graph;
}  // namespace ark::compiler

namespace ark::llvmbackend {

struct CompiledCode {
    const uint8_t *code;
    size_t size;
};

struct CompilerInterface {
    CompilerInterface() = default;
    virtual ~CompilerInterface() = default;

    virtual Expected<bool, std::string> TryAddGraph(ark::compiler::Graph *graph) = 0;
    virtual void FinishCompile() = 0;
    virtual bool HasCompiledCode() = 0;
    virtual bool IsIrFailed() = 0;

    CompilerInterface(const CompilerInterface &) = delete;
    void operator=(const CompilerInterface &) = delete;
    CompilerInterface(CompilerInterface &&) = delete;
    CompilerInterface &operator=(CompilerInterface &&) = delete;
};

struct IrtocCompilerInterface : public CompilerInterface {
    IrtocCompilerInterface() = default;
    ~IrtocCompilerInterface() override = default;

    virtual void WriteObjectFile(std::string_view output) = 0;
    virtual CompiledCode GetCompiledCode(std::string_view functionName) = 0;
    virtual size_t GetObjectFileSize() = 0;
    virtual bool IsEmpty() = 0;

    IrtocCompilerInterface(const IrtocCompilerInterface &) = delete;
    void operator=(const IrtocCompilerInterface &) = delete;
    IrtocCompilerInterface(IrtocCompilerInterface &&) = delete;
    IrtocCompilerInterface &operator=(IrtocCompilerInterface &&) = delete;
};

}  // namespace ark::llvmbackend

#endif  // LIBLLVMBACKEND_COMPILER_INTERFACE_H
