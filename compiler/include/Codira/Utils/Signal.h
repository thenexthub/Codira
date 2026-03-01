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
 * This file declares crash signal handler related functions.
 */

#ifndef CODIRA_UTILS_SIGNAL_H
#define CODIRA_UTILS_SIGNAL_H

#if (defined RELEASE)
#include "Codira/Utils/ICEUtil.h"
#include "Codira/Utils/FileUtil.h"


#ifdef __unix__
#include <csignal>
#include <functional>
#elif __APPLE__
#include <signal.h>
#elif _WIN32
#include <signal.h>
#include <windows.h>
#endif

namespace Codira {

#if defined(__unix__) || defined(__APPLE__)
const std::string SIGNAL_MSG_PART_ONE = "Interrupt signal (";
/* Create alternate signal stack. */
void CreateAltSignalStack();

#elif _WIN32
const std::string SIGNAL_MSG_PART_ONE = "Windows unexpected exception code (";
void RegisterCrashExceptionHandler();
#endif
const std::string SIGNAL_MSG_PART_TWO = ") received.";

/* Register signal handler for crash signals. */
void RegisterCrashSignalHandler();

#ifdef CODIRA_BUILD_TESTS
#define SIGNAL_TEST
namespace SignalTest {
using SignalTestCallbackFuncType = void (*)(void);

enum TriggerPointer {
    NON_POINTER,    // The test callback function is not executed.
    MAIN_POINTER,   // Execute the test callback function inserted in the main func.
    DRIVER_POINTER, // Execute the test callback function inserted in the Driver module.
    PARSER_POINTER, // Execute the test callback function inserted in the Parser module.
    SEMA_POINTER,   // Execute the test callback function inserted in the Sema module.
    CHIR_POINTER,   // Execute the test callback function inserted in the CHIR module.
    CODEGEN_POINTER // Execute the test callback function inserted in the CodeGen module.
};
void SetSignalTestCallbackFunc(SignalTestCallbackFuncType fp, TriggerPointer pointerType, int errorCodeOffset);
void ExecuteSignalTestCallbackFunc(TriggerPointer executionPoint);
} // namespace SignalTest
#endif

} // namespace Codira
#endif
namespace Codira {
/* Register signal handler for Crtl C signal. */
void RegisterCrtlCSignalHandler();
} // namespace Codira

#endif // CODIRA_UTILS_SIGNAL_H
