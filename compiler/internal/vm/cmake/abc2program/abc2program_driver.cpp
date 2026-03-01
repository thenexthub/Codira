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

#include "abc2program_driver.h"
#include "abc2program_options.h"
#include "program_dump.h"

namespace panda::abc2program {

int Abc2ProgramDriver::Run(int argc, const char **argv)
{
    Abc2ProgramOptions options;
    if (!options.Parse(argc, argv)) {
        return 1;
    }
    if (Run(options.GetInputFilePath(), options.GetOutputFilePath())) {
        return 0;
    }
    return 1;
}

bool Abc2ProgramDriver::Run(const std::string &input_file_path, const std::string &output_file_path)
{
    return (Compile(input_file_path) && Dump(output_file_path));
}

bool Abc2ProgramDriver::Compile(const std::string &input_file_path)
{
    input_file_path_ = input_file_path;
    if (!compiler_.OpenAbcFile(input_file_path)) {
        return false;
    }
    program_ = std::move(*compiler_.CompileAbcFile());
    return true;
}

bool Abc2ProgramDriver::Dump(const std::string &output_file_path)
{
    // program dump logic
    std::ofstream ofs;
    ofs.open(output_file_path, std::ios::trunc | std::ios::out);
    PandasmProgramDumper dumper;
    dumper.SetAbcFilePath(input_file_path_);
    dumper.Dump(ofs, program_);
    ofs.close();
    return true;
}

const pandasm::Program &Abc2ProgramDriver::GetProgram() const
{
    return program_;
}

}  // namespace panda::abc2program