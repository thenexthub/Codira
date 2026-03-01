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
 * This file declares the Backend related classes, which provides versatile compile utility.
 */

#ifndef CODIRA_DRIVER_BACKEND_H
#define CODIRA_DRIVER_BACKEND_H

#include "Codira/Driver/DriverOptions.h"
#include "Codira/Driver/Toolchains/ToolChain.h"
#include "Codira/Option/Option.h"

namespace Codira {
class Job;

/**
 * Backend is an interface. An implementation of Backend generates one or more
 * tools. A tool contains a tool name (e.g. llc, javac), some arguments (e.g. -O2, -static, -L.), and
 * some necessary environment settings for generating an actual executable command. A tool represents
 * a well-structured command. Backend itself does not execute any commands.
 */
class Backend {
public:
    /**
     * @brief The constructor of class Backend.
     *
     * @param job The compilation job.
     * @param driverOptions The data structure is obtained through parsing the compilation options.
     * @param driver It is the object that triggers the compiler's compilation process.
     * @return Backend The instance of Backend.
     */
    explicit Backend(Job& job, const DriverOptions& driverOptions, const Driver& driver)
        : driver(driver), driverOptions(driverOptions), ownerJob(job)  {}

    /**
     * @brief The destructor of class Backend.
     */
    virtual ~Backend() = default;

    /**
     * @brief Generate toolchain, assembly tools.
     *
     * @return bool Return true If generate success.
     */
    bool Generate();

    const std::vector<ToolBatch>& GetBackendCmds()
    {
        return backendCmds;
    }

protected:
    const Driver& driver;
    const DriverOptions& driverOptions;
    std::unique_ptr<ToolChain> TC;
    Job& ownerJob;
    std::vector<ToolBatch> backendCmds;

    virtual bool GenerateToolChain() = 0;
    virtual bool ProcessGeneration() = 0;

    /**
     * Check whether tools exist (and get their paths if required)
     * If some dependencies are not available, this function should return false so the generation won't proceed.
     */
    virtual bool PrepareDependencyPath() = 0;
};
} // namespace Codira
#endif // CODIRA_DRIVER_BACKEND_H
