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
 * This file declares Tool Options functions.
 */

#ifndef CODIRA_DRIVER_TOOL_OPTIONS_H
#define CODIRA_DRIVER_TOOL_OPTIONS_H

#include <functional>
#include <string>
#include <vector>

#include "Codira/Driver/DriverOptions.h"

namespace Codira {
namespace ToolOptions {

using SetFuncType = const std::function<void(std::string)>;
using ToolOptionType = std::function<void(SetFuncType, const DriverOptions&)>;

namespace OPT {
/**
 * @brief Set the new password manager opt options.
 *
 * @param setOptionHandler The function to remove the initial hyphen in the options.
 * @param driverOptions The data structure is obtained through parsing the compilation options.
 */
void SetNewPassManagerOptions(SetFuncType setOptionHandler, const DriverOptions& driverOptions);

/**
 * @brief Set code obfuscation opt options.
 *
 * @param setOptionHandler The function to set composite option.
 * @param driverOptions The data structure is obtained through parsing the compilation options.
 */
void SetCodeObfuscationOptions(SetFuncType setOptionHandler, const DriverOptions& driverOptions);

/**
 * @brief Set LTO opt options.
 *
 * @param setOptionHandler The function to set LTO lld options.
 * @param driverOptions The data structure is obtained through parsing the compilation options.
 */
void SetLTOOptions(SetFuncType setOptionHandler, const DriverOptions& driverOptions);

/**
 * @brief Set optimization level opt options.
 *
 * @param setOptionHandler The function to set optimization level.
 * @param driverOptions The data structure is obtained through parsing the compilation options.
 */
void SetOptimizationLevelOptions(SetFuncType setOptionHandler, const DriverOptions& driverOptions);

/**
 * @brief set opt options.
 *
 * @param setOptionHandler The function to set opt options.
 * @param driverOptions The data structure is obtained through parsing the compilation options.
 */
void SetOptions(SetFuncType setOptionHandler, const DriverOptions& driverOptions);

/**
 * @brief Set verify opt options.
 *
 * @param setOptionHandler The function to set 'only-verify-out' option.
 * @param driverOptions The data structure is obtained through parsing the compilation options.
 */
void SetVerifyOptions(SetFuncType setOptionHandler, const DriverOptions& driverOptions);

/**
 * @brief Set triple opt options.
 *
 * @param setOptionHandler The function to set 'mtriple' option.
 * @param driverOptions The data structure is obtained through parsing the compilation options.
 */
void SetTripleOptions(SetFuncType setOptionHandler, const DriverOptions& driverOptions);

/**
 * @brief Set transparent opt options.
 *
 * @param setOptionHandler The function to set transparent options.
 * @param driverOptions The data structure is obtained through parsing the compilation options.
 */
void SetTransparentOptions(SetFuncType setOptionHandler, const DriverOptions& driverOptions);

/**
 * @brief Set Pgo opt options.
 *
 * @param setOptionHandler The function to set Pgo options.
 * @param driverOptions The data structure is obtained through parsing the compilation options.
 */
void SetPgoOptions(SetFuncType setOptionHandler, const DriverOptions& driverOptions);
} // namespace OPT

namespace LLC {
/**
 * @brief Set optimization level llc options.
 *
 * @param setOptionHandler The function to llc optimization level options.
 * @param driverOptions The data structure is obtained through parsing the compilation options.
 */
void SetOptimizationLevelOptions(SetFuncType setOptionHandler, const DriverOptions& driverOptions);

/**
 * @brief Set llc options.
 *
 * @param setOptionHandler The function to set llc options.
 * @param driverOptions The data structure is obtained through parsing the compilation options.
 */
void SetOptions(SetFuncType setOptionHandler, const DriverOptions& driverOptions);

/**
 * @brief Set triple llc options.
 *
 * @param setOptionHandler The function to set 'mtriple' option.
 * @param driverOptions The data structure is obtained through parsing the compilation options.
 */
void SetTripleOptions(SetFuncType setOptionHandler, const DriverOptions& driverOptions);

/**
 * @brief Set transparent llc options.
 *
 * @param setOptionHandler The function to set args may contain multiple opt arguments separated by spaces.
 * @param driverOptions The data structure is obtained through parsing the compilation options.
 */
void SetTransparentOptions(SetFuncType setOptionHandler, const DriverOptions& driverOptions);
} // namespace LLC

namespace LLD {
/**
 * @brief Set LTO optimization level lld options.
 *
 * @param setOptionHandler The function to set LTO lld options.
 * @param driverOptions The data structure is obtained through parsing the compilation options.
 */
void SetLTOOptimizationLevelOptions(SetFuncType setOptionHandler, const DriverOptions& driverOptions);

/**
 * @brief Set LTO lld options.
 *
 * @param setOptionHandler The function to LTO lld options.
 * @param driverOptions The data structure is obtained through parsing the compilation options.
 */
void SetLTOOptions(SetFuncType setOptionHandler, const DriverOptions& driverOptions);

/**
 * @brief Set Pgo lld options.
 *
 * @param setOptionHandler The function to.
 * @param driverOptions The data structure is obtained through parsing the compilation options.
 */
void SetPgoOptions(SetFuncType setOptionHandler, const DriverOptions& driverOptions);
} // namespace LLD

/**
 * @brief Call function object to set options.
 *
 * @param setOptionHandler The function to set options.
 * @param driverOptions The data structure is obtained through parsing the compilation options.
 * @param typeFunc a function object.
 */
void SetOptions(SetFuncType setOptionHandler, const DriverOptions& driverOptions, const ToolOptionType typeFunc);

/**
 * @brief Call function objects to set options.
 *
 * @param setOptionHandler The function to set options.
 * @param driverOptions The data structure is obtained through parsing the compilation options.
 * @param typeFuncs a function object vector.
 */
void SetOptions(
    SetFuncType setOptionHandler, const DriverOptions& driverOptions, const std::vector<ToolOptionType>& typeFuncs);
} // namespace ToolOptions
} // namespace Codira

#endif // CODIRA_DRIVER_TOOL_OPTIONS_H
