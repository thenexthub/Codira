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

#include "plugins/ets/runtime/ani/ani_options_parser.h"

#include "compiler/compiler_options.h"
#include "plugins/ets/compiler/product_options.h"
#include "plugins/ets/runtime/ets_vm_options.h"
#include "plugins/ets/runtime/product_options.h"

namespace ark::ets::ani {

#ifdef PANDA_PRODUCT_BUILD
static constexpr bool ANI_SUPPORTS_ONLY_PRODUCT_OPTIONS = true;
#else
static constexpr bool ANI_SUPPORTS_ONLY_PRODUCT_OPTIONS = false;
#endif  // PANDA_PRODUCT_BUILD

// clang-format off
static constexpr std::array AVAILABLE_LOGGER_OPTIONS_IN_PRODUCT = {
    "log-components",
    "log-level",
};
// clang-format on

OptionsParser::OptionsParser()
{
    loggerOptions_.AddOptions(&loggerOptionsParser_);
    runtimeOptions_.AddOptions(&runtimeOptionsParser_);
    compiler::g_options.AddOptions(&compilerOptionsParser_);

    // Set default runtime options for 'ets' language
    runtimeOptions_.SetBootIntrinsicSpaces({"ets"});
    runtimeOptions_.SetBootClassSpaces({"ets"});
    runtimeOptions_.SetRuntimeType("ets");
    runtimeOptions_.SetLoadRuntimes({"ets"});
}

OptionsParser::ErrorMsg OptionsParser::GetParserDependentArgs(const PandArgParser &parser,
                                                              const std::set<std::string> &productOptions,
                                                              OptionsMap &extOptionsMap,
                                                              std::vector<std::string> &outArgs)
{
    auto names = parser.GetNamesOfParsedArgs();
    for (auto &name : names) {
        if (ANI_SUPPORTS_ONLY_PRODUCT_OPTIONS && productOptions.find(name) == productOptions.end()) {
            continue;  // skip unsupported options
        }

        auto it = extOptionsMap.find(name);
        if (it != extOptionsMap.end()) {
            if (it->second->opt->extra != nullptr) {
                std::stringstream ss;
                ss << "Extended '" << it->second->fullName
                   << "' option doesn't support 'extra' field, 'extra' must be NULL";
                return ss.str();
            }
            std::string_view value = it->second->value;
            std::stringstream ss;
            ss << "--" << it->second->name;
            if (!value.empty()) {
                ss << '=' << value;
            }
            outArgs.emplace_back(ss.str());
            extOptionsMap.erase(it);
        }
    }
    return {};
}

OptionsParser::ErrorMsg OptionsParser::Parse(const ani_options *options)
{
    if (options == nullptr) {
        return {};
    }

    struct ExtOpt {
        const ani_option *opt;
        std::string_view name;
        std::string_view value;
    };

    std::vector<ExtOpt> extOptions;
    Span<const ani_option> opts(options->options, options->nr_options);
    for (const ani_option &opt : opts) {
        std::string_view option = opt.option;
        std::string_view name = option;
        std::string_view value;
        size_t pos = option.find('=');
        if (pos != std::string_view::npos) {
            name = option.substr(0, pos);
            value = option.substr(pos + 1);
        }

        auto isAddedOption = aniOptions_.SetOption(name, value, opt.extra);
        if (!isAddedOption.HasValue()) {
            return isAddedOption.Error();
        }
        if (!isAddedOption.Value()) {
            extOptions.push_back(ExtOpt {&opt, name, value});
        }
    }

    OptionsMap extOptionsMap;
    for (const ExtOpt &opt : extOptions) {
        std::unique_ptr<Option> op = ParseExtendedOption(opt.name, opt.value, opt.opt);
        if (!op) {
            std::stringstream ss;
            ss << "'" << opt.name << "' option is not supported";
            return ss.str();
        }
        extOptionsMap[op->name] = std::move(op);
    }
    ErrorMsg errorMsg = ParseExtOptions(std::move(extOptionsMap));
    if (errorMsg) {
        return errorMsg;
    }
    PrepareEtsVmOptions();
    return {};
}

OptionsParser::ErrorMsg OptionsParser::ParseExtOptions(OptionsMap extOptionsMap)
{
    std::set<std::string> loggerProductOptions(AVAILABLE_LOGGER_OPTIONS_IN_PRODUCT.cbegin(),
                                               AVAILABLE_LOGGER_OPTIONS_IN_PRODUCT.cend());
    std::vector<std::string> loggerArgs;
    ErrorMsg errorMsg = GetParserDependentArgs(loggerOptionsParser_, loggerProductOptions, extOptionsMap, loggerArgs);
    if (errorMsg) {
        return errorMsg;
    }
    std::set<std::string> runtimeProductOptions(AVAILABLE_RUNTIME_OPTIONS_IN_PRODUCT.cbegin(),
                                                AVAILABLE_RUNTIME_OPTIONS_IN_PRODUCT.cend());
    std::vector<std::string> runtimeArgs;
    errorMsg = GetParserDependentArgs(runtimeOptionsParser_, runtimeProductOptions, extOptionsMap, runtimeArgs);
    if (errorMsg) {
        return errorMsg;
    }
    std::set<std::string> compilerProductOptions(AVAILABLE_COMPILER_OPTIONS_IN_PRODUCT.cbegin(),
                                                 AVAILABLE_COMPILER_OPTIONS_IN_PRODUCT.cend());
    std::vector<std::string> compilerArgs;
    errorMsg = GetParserDependentArgs(compilerOptionsParser_, compilerProductOptions, extOptionsMap, compilerArgs);
    if (errorMsg) {
        return errorMsg;
    }
    if (!extOptionsMap.empty()) {
        std::stringstream ss;
        ss << "'" << extOptionsMap.begin()->second->fullName << "' option is not supported";
        return ss.str();
    }

    errorMsg = RunLoggerOptionsParser(loggerArgs);
    if (errorMsg) {
        return errorMsg;
    }
    errorMsg = RunRuntimeOptionsParser(runtimeArgs);
    if (errorMsg) {
        return errorMsg;
    }
    return RunCompilerOptionsParser(compilerArgs);
}

std::unique_ptr<OptionsParser::Option> OptionsParser::ParseExtendedOption(std::string_view name, std::string_view value,
                                                                          const ani_option *opt)
{
    const std::string_view depricatedExtPrefix = "--ext:--";
    const std::string_view extPrefix = "--ext:";

    if (name.substr(0, depricatedExtPrefix.size()) == depricatedExtPrefix) {
        return std::make_unique<Option>(Option {name, name.substr(depricatedExtPrefix.size()), value, opt});
    }
    if (name.substr(0, extPrefix.size()) == extPrefix) {
        return std::make_unique<Option>(Option {name, name.substr(extPrefix.size()), value, opt});
    }
    return {};
}

OptionsParser::ErrorMsg OptionsParser::RunLoggerOptionsParser(const std::vector<std::string> &args)
{
    auto errorMsg = RunOptionsParser(loggerOptionsParser_, args);
    if (errorMsg) {
        return errorMsg;
    }
    auto error = loggerOptions_.Validate();
    if (error) {
        // Set default options
        loggerOptions_ = logger::Options("");
        return error->GetMessage();
    }
    return {};
}

OptionsParser::ErrorMsg OptionsParser::RunRuntimeOptionsParser(const std::vector<std::string> &args)
{
    auto errorMsg = RunOptionsParser(runtimeOptionsParser_, args);
    if (errorMsg) {
        return errorMsg;
    }
    auto error = runtimeOptions_.Validate();
    if (error) {
        return error->GetMessage();
    }
    return {};
}

OptionsParser::ErrorMsg OptionsParser::RunCompilerOptionsParser(const std::vector<std::string> &args)
{
    auto errorMsg = RunOptionsParser(compilerOptionsParser_, args);
    if (errorMsg) {
        return errorMsg;
    }
    auto error = compiler::g_options.Validate();
    if (error) {
        return error->GetMessage();
    }
    return {};
}

OptionsParser::ErrorMsg OptionsParser::RunOptionsParser(PandArgParser &parser, const std::vector<std::string> &args)
{
    if (!parser.Parse(args)) {
        std::string errorMessage = parser.GetErrorString();
        if (!errorMessage.empty() && errorMessage.back() == '\n') {
            // Trim new line
            errorMessage = std::string(errorMessage.c_str(), errorMessage.length() - 1);
        }
        return errorMessage;
    }
    return {};
}

void OptionsParser::PrepareEtsVmOptions()
{
    SetEtsVmOptions(runtimeOptions_, std::make_unique<EtsVmOptions>(aniOptions_.IsVerifyANI()));
}

}  // namespace ark::ets::ani
