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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_OPTIONS_PARSER_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_OPTIONS_PARSER_H

#include "ani.h"

#include "libarkbase/panda_gen_options/generated/logger_options.h"
#include "libarkbase/utils/pandargs.h"
#include "libarkbase/utils/span.h"
#include "plugins/ets/runtime/ani/ani_options.h"
#include "runtime/include/runtime_options.h"

namespace ark::ets::ani {

class OptionsParser final {
public:
    using ErrorMsg = std::optional<std::string>;

    OptionsParser();
    ~OptionsParser() = default;

    ErrorMsg Parse(const ani_options *options);

    const ANIOptions &GetANIOptions()
    {
        return aniOptions_;
    }

    const logger::Options &GetLoggerOptions()
    {
        return loggerOptions_;
    }

    const RuntimeOptions &GetRuntimeOptions()
    {
        return runtimeOptions_;
    }

private:
    struct Option {
        std::string_view fullName;
        std::string_view name;
        std::string_view value;
        const ani_option *opt;
    };

    using OptionsMap = std::map<std::string_view, std::unique_ptr<Option>>;

    ErrorMsg GetParserDependentArgs(const PandArgParser &parser, const std::set<std::string> &productOptions,
                                    OptionsMap &extOptionsMap, std::vector<std::string> &outArgs);
    ErrorMsg RunOptionsParser(PandArgParser &parser, const std::vector<std::string> &args);
    ErrorMsg RunLoggerOptionsParser(const std::vector<std::string> &args);
    ErrorMsg RunRuntimeOptionsParser(const std::vector<std::string> &args);
    ErrorMsg RunCompilerOptionsParser(const std::vector<std::string> &args);
    std::unique_ptr<Option> ParseExtendedOption(std::string_view name, std::string_view value, const ani_option *opt);
    ErrorMsg ParseExtOptions(OptionsMap extOptionsMap);
    void PrepareEtsVmOptions();

    ANIOptions aniOptions_;

    PandArgParser loggerOptionsParser_;
    logger::Options loggerOptions_ {""};

    PandArgParser runtimeOptionsParser_;
    RuntimeOptions runtimeOptions_;

    PandArgParser compilerOptionsParser_;

    NO_COPY_SEMANTIC(OptionsParser);
    NO_MOVE_SEMANTIC(OptionsParser);
};

}  // namespace ark::ets::ani

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_OPTIONS_PARSER_H
