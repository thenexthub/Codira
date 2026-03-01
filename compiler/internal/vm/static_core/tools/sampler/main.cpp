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

#include "libarkbase/utils/logger.h"
#include "tools/sampler/aspt_converter.h"

namespace ark::tooling::sampler {

int Main(int argc, const char **argv)
{
    ark::Span<const char *> sp(argv, argc);

    ArgsParser parser;
    if (!parser.Parse(sp)) {
        parser.Help();
        return 1;
    }
    const Options &cliOptions = parser.GetOptions();

    Logger::InitializeStdLogging(Logger::Level::INFO, Logger::ComponentMaskFromString("profiler"));

    AsptConverter asptConv(cliOptions.GetInput().c_str());
    if (!asptConv.RunWithOptions(cliOptions)) {
        return 1;
    }

    return 0;
}

}  // namespace ark::tooling::sampler

int main(int argc, const char **argv)
{
    return ark::tooling::sampler::Main(argc, argv);
}
