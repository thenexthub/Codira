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

#ifndef LSPSERVER_OPTION_H
#define LSPSERVER_OPTION_H

#include <map>
#include <string>

namespace ark {
constexpr int LONG_ARG_LEN = 2;

struct Environment {
    std::string cangjieHome;
    std::string cangjiePath;
    std::string runtimePath;
};

/**
 * @class Option
 * @brief This implementation assumes that if a parameter requires a value, the value should either
 * directly follow the parameter (for short parameters), be connected by an equal sign (for long
 * parameters), or be placed as the next parameter (if the next parameter does not start with a
 * hyphen).
 */
class Options {
public:
    // Delete copy constructor and assignment operator to ensure singleton
    Options(const Options &) = delete;
    Options &operator=(const Options &) = delete;

    static Options &GetInstance()
    {
        static Options instance;
        return instance;
    }

    void Parse(const int argc, const char *argv[])
    {
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg[0] == '-' && arg[1] != '-') {
                // deal short parameters
                if (arg.length() > LONG_ARG_LEN) {
                    // Assume that the short parameter is followed by its value.
                    shortOptions[arg[1]] = arg.substr(LONG_ARG_LEN);
                } else if (i + 1 < argc && argv[i + 1][0] != '-') {
                    // The next parameter is the value of this short parameter
                    shortOptions[arg[1]] = argv[++i];
                } else {
                    // Short parameter with no value
                    shortOptions[arg[1]] = "";
                }
            } else if (arg[0] == '-' && arg[1] == '-') {
                // deal long parameters
                std::string::size_type equalPos = arg.find('=');
                if (equalPos != std::string::npos) {
                    // The parameter specifies the value by an equal sign
                    longOptions[arg.substr(LONG_ARG_LEN, equalPos - LONG_ARG_LEN)] = arg.substr(equalPos + 1);
                } else if (i + 1 < argc && argv[i + 1][0] != '-') {
                    // The next parameter is the value of this long parameter
                    longOptions[arg.substr(LONG_ARG_LEN)] = argv[++i];
                } else {
                    // Long parameter with no value
                    longOptions[arg.substr(LONG_ARG_LEN)] = "";
                }
            }
        }
    }

    bool IsOptionSet(const char shortOption) { return shortOptions.find(shortOption) != shortOptions.end(); }

    bool IsOptionSet(const std::string &longOption) { return longOptions.find(longOption) != longOptions.end(); }

    std::optional<std::string> GetShortOption(char option)
    {
        if (shortOptions.find(option) != shortOptions.end()) {
            return shortOptions[option];
        }
        return std::nullopt;
    }

    std::optional<std::string> GetLongOption(const std::string &option)
    {
        if (longOptions.find(option) != longOptions.end()) {
            return longOptions[option];
        }
        return std::nullopt;
    }

    void SetLSPFlag(const std::string &flag)
    {
        (void)internalFLag.insert_or_assign(flag, true);
    }

    std::optional<bool> GetLSPFlag(const std::string &flag)
    {
        if (internalFLag.find(flag) != internalFLag.end()) {
            return internalFLag[flag];
        }
        return std::nullopt;
    }

private:
    Options() { DefineOptions(); }
    ~Options() = default;

    std::map<char, std::string> shortOptions;
    std::map<std::string, std::string> longOptions;
    std::map<std::string, std::string> optionDescriptions;

    // Flag               Descriptions                                Default
    // enableParallel     Enable compiler package by thread pools     false
    std::map<std::string, bool> internalFLag;

    void DefineOptions()
    {
        optionDescriptions["-h, --help"] = "Display this help information";
        optionDescriptions["-V, --verbose"] = "Record the crash logs when the codepls crashes";
        optionDescriptions["--test"] = "For the execution of UT";
        // Add more options and their description here
        // ...
        // Add intenral flag for code language server
        internalFLag.emplace("enableParallel", false);
    }
};
} // namespace ark

#endif // LSPSERVER_OPTION_H
