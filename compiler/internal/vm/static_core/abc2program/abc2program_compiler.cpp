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

#include "abc2program_compiler.h"
#include <iostream>
#include "abc_file_processor.h"

namespace ark::abc2program {

bool Abc2ProgramCompiler::OpenAbcFile(const std::string &filePath)
{
    file_ = panda_file::File::Open(filePath);
    if (file_ == nullptr) {
        std::cerr << "Unable to open specified abc file " << filePath << std::endl;
        return false;
    }
    stringTable_ = std::make_unique<AbcStringTable>(*file_);
    return true;
}

const panda_file::File &Abc2ProgramCompiler::GetAbcFile() const
{
    return *file_;
}

AbcStringTable &Abc2ProgramCompiler::GetAbcStringTable() const
{
    return *stringTable_;
}

bool Abc2ProgramCompiler::FillProgramData(pandasm::Program &program)
{
    keyData_ = std::make_unique<Abc2ProgramKeyData>(*file_, *stringTable_, program);
    AbcFileProcessor fileProcessor(*keyData_);
    bool success = fileProcessor.ProcessFile();
    return success;
}

}  // namespace ark::abc2program
