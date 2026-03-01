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

#ifndef GUARD_ARGS_PARSER_H
#define GUARD_ARGS_PARSER_H

#include <string>

namespace ark::guard {

/**
 * @brief ArgsParser
 */
class ArgsParser {
public:
    /**
     * @brief parse args
     * @return bool true: success, false: failed
     */
    bool Parse(int argc, const char **argv);

    /**
     * @brief Get config file path
     * @return std::string config file path
     */
    [[nodiscard]] const std::string &GetConfigFilePath() const;

    /**
     * @brief is Debug, if debug mode, log level will set debug, otherwise log level set error
     * @return bool true: debug mode, false: release mode
     */
    [[nodiscard]] bool IsDebug() const;

    /**
     * @brief is Help, if help mode, print help info, otherwise not print help info
     * @return bool true: help mode, false: not help mode
     */
    [[nodiscard]] bool IsHelp() const;

    /**
     * @brief Get debug file path
     * @return std::string if empty, will print to standard output, otherwise will print to the debug file
     */
    [[nodiscard]] const std::string &GetDebugFile() const;

private:
    std::string configFilePath_;

    bool isHelp_ = false;

    bool isDebug_ = false;

    std::string debugFile_;
};

}  // namespace ark::guard

#endif  // GUARD_ARGS_PARSER_H
