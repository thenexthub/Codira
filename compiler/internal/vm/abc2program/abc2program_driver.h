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

namespace panda::abc2program {

class Abc2ProgramDriver {
public:
    int Run(int argc, const char **argv);
    bool Run(const std::string &input_file_path, const std::string &output_file_path);
    bool Compile(const std::string &input_file_path);
    bool Dump(const std::string &output_file_path);
    const pandasm::Program &GetProgram() const;

private:
    Abc2ProgramCompiler compiler_;
    pandasm::Program program_;
    std::string input_file_path_;
}; // class Abc2ProgramDriver

} // namespace panda::abc2program

#endif // ABC2PROGRAM_ABC2PROGRAM_DRIVER_H
