/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_TOOLS_SAMPLER_ARGS_PARSER_H
#define PANDA_TOOLS_SAMPLER_ARGS_PARSER_H

#include "tools/sampler/panda_gen_options/generated/aspt_converter_options.h"

namespace ark::tooling::sampler {

class ArgsParser {
public:
    bool Parse(ark::Span<const char *> args)
    {
        options_.AddOptions(&parser_);

        if (!parser_.Parse(args.Size(), args.Data())) {
            std::cerr << parser_.GetErrorString();
            return false;
        }

        auto err = options_.Validate();
        if (err) {
            std::cerr << err.value().GetMessage() << std::endl;
            return false;
        }

        if (options_.GetInput().empty()) {
            std::cerr << "[Error] Input file is not set." << std::endl;
            return false;
        }

        if (options_.GetOutput().empty()) {
            std::cerr << "[Error] Output file is not set." << std::endl;
            return false;
        }

        if (!ark::os::IsFileExists(options_.GetInput())) {
            std::cerr << "[Error] File \"" << options_.GetInput() << "\" not found." << std::endl;
            return false;
        }

        return true;
    }

    const Options &GetOptions() const
    {
        return options_;
    }

    void Help() const
    {
        std::cerr << "AsptConverter usage: " << std::endl;
        std::cerr << "${BUILD_DIR}/bin/aspt_converter "
                  << "--input=<input_name.aspt> "
                  << "--output=<output_name.csv> "
                  << "[OPTIONS]" << std::endl;
        std::cerr << "optional arguments:" << std::endl;
        std::cerr << parser_.GetHelpString() << std::endl;
    }

private:
    PandArgParser parser_;
    Options options_ {""};
};

}  // namespace ark::tooling::sampler

#endif  // PANDA_TOOLS_SAMPLER_ARGS_PARSER_H
