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
 * This file declares the Driver, who runs the compiler.
 */

#ifndef CODIRA_DRIVER_DRIVER_H
#define CODIRA_DRIVER_DRIVER_H

#include <memory>
#include <string>
#include <vector>

#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/Driver/DriverOptions.h"

namespace Codira {
class Driver {
public:
    /**
     * @brief The constructor of class Driver.
     *
     * @param args The arguments vector.
     * @param diag The main diagnostic processing center.
     * @param exeName The name of exe.
     * @return Driver The instance of Driver.
     */
    Driver(const std::vector<std::string>& args, DiagnosticEngine& diag, const std::string& exeName = "codec");

    /**
     * @brief Parse arguments and setup options specified by user in the command.
     *
     * @return bool Return true If success.
     */
    bool ParseArgs();

    /**
     * @brief Read necessary paths from environment variables and store them in GlobalOptions.
     *
     * @param environmentVars The environment variables.
     */
    void EnvironmentSetup(const std::unordered_map<std::string, std::string>& environmentVars);

    /**
     * @brief The main function of the compilation process.
     *
     * @return bool Return true If success.
     */
    bool ExecuteCompilation() const;

    /**
     * @brief Generate backend and linking commands.
     *
     * @return bool Return true If success.
     */
    bool InvokeCompileToolchain() const;

    std::vector<std::string> args;
    DiagnosticEngine& diag;
    std::unique_ptr<OptionTable> optionTable;
    std::unique_ptr<ArgList> argList;
    std::unique_ptr<DriverOptions> driverOptions;
    std::string executableName;
    std::string codiraHome;
};
} // namespace Codira

#endif // CODIRA_DRIVER_DRIVER_H
