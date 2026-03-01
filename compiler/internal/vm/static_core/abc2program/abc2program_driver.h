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

#ifndef ABC2PROGRAM_ABC2PROGRAM_DRIVER_H
#define ABC2PROGRAM_ABC2PROGRAM_DRIVER_H

#include <string>
#include <assembly-program.h>
#include "abc2program_compiler.h"

namespace ark::abc2program {

class Abc2ProgramDriver {
public:
    PANDA_PUBLIC_API int Run(int argc, const char **argv);
    PANDA_PUBLIC_API bool Run(const std::string &inputFilePath, const std::string &outputFilePath);
    PANDA_PUBLIC_API bool Compile(const std::string &inputFilePath);
    PANDA_PUBLIC_API const pandasm::Program &GetProgram() const;
    PANDA_PUBLIC_API pandasm::Program &GetProgram();

private:
    bool Dump(const std::string &outputFilePath);
    bool Compile(const std::string &inputFilePath, pandasm::Program &program);
    Abc2ProgramCompiler compiler_;
    pandasm::Program program_;
};  // class Abc2ProgramDriver

}  // namespace ark::abc2program

#endif  // ABC2PROGRAM_ABC2PROGRAM_DRIVER_H
