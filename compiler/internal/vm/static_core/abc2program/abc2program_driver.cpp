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
#include "program_dump.h"
#include "abc2program_options.h"

namespace ark::abc2program {

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

bool Abc2ProgramDriver::Run(const std::string &inputFilePath, const std::string &outputFilePath)
{
    return (Compile(inputFilePath) && Dump(outputFilePath));
}

bool Abc2ProgramDriver::Compile(const std::string &inputFilePath)
{
    return Compile(inputFilePath, program_);
}

bool Abc2ProgramDriver::Compile(const std::string &inputFilePath, pandasm::Program &program)
{
    // abc file compile logic
    if (!compiler_.OpenAbcFile(inputFilePath)) {
        return false;
    }
    return compiler_.FillProgramData(program);
}

bool Abc2ProgramDriver::Dump(const std::string &outputFilePath)
{
    // program dump logic
    std::ofstream ofs;
    ofs.open(outputFilePath, std::ios::trunc | std::ios::out);
    PandasmProgramDumper dumper(compiler_.GetAbcFile(), compiler_.GetAbcStringTable());
    dumper.Dump(ofs, program_);
    ofs.close();
    return true;
}

const pandasm::Program &Abc2ProgramDriver::GetProgram() const
{
    return program_;
}

pandasm::Program &Abc2ProgramDriver::GetProgram()
{
    return program_;
}

}  // namespace ark::abc2program