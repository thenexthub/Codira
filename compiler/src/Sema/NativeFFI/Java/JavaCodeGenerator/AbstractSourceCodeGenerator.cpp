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

/**
 * @file
 *
 * This file implements Java class generation.
 */

#include "AbstractSourceCodeGenerator.h"

namespace Codira::Interop {
AbstractSourceCodeGenerator::AbstractSourceCodeGenerator(const std::string& outputFilePath)
    : outputFilePath(outputFilePath)
{
}

AbstractSourceCodeGenerator::AbstractSourceCodeGenerator(
    const std::string& outputFolderPath, const std::string& outputFileName)
    : outputFilePath(FileUtil::JoinPath(outputFolderPath, outputFileName))
{
}

void AbstractSourceCodeGenerator::Generate()
{
    ConstructResult();
    WriteToOutputFile();
}

bool AbstractSourceCodeGenerator::WriteToOutputFile()
{
    return FileUtil::WriteToFile(outputFilePath, res);
}

void AbstractSourceCodeGenerator::AddWithIndent(const std::string& indent, const std::string& s)
{
    res += indent;
    res += s;
    res += "\n";
}

const std::string AbstractSourceCodeGenerator::TAB = "    ";
const std::string AbstractSourceCodeGenerator::TAB2 =
    AbstractSourceCodeGenerator::TAB + AbstractSourceCodeGenerator::TAB;

} // namespace Codira::Interop
