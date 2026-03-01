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
 * This file is the main entry of compiler.
 */

#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/Driver/Driver.h"
#include "Codira/Driver/TempFileManager.h"
#include "Codira/FrontendTool/FrontendTool.h"
#include "Codira/Utils/FileUtil.h"

#include <memory>
#include <exception>
#include <string>
#ifdef _WIN32
#include <algorithm>
#include <libloaderapi.h>
#endif

#include "Codira/Basic/Print.h"
#include "Codira/Utils/Signal.h"

namespace {

#if (defined RELEASE)
void RegisterSignalHandler()
{
#if (defined __unix__)
    Codira::CreateAltSignalStack();
#elif (defined _WIN32)
    Codira::RegisterCrashExceptionHandler();
#endif
    Codira::RegisterCrashSignalHandler();
}
#endif

const int EXIT_CODE_SUCCESS = 0;
const int EXIT_CODE_ERROR = 1; // Normal compiler error

} // namespace

using namespace Codira;

int main(int argc, const char** argv, const char** envp)
{
    try {
#if (defined RELEASE)
        RegisterSignalHandler();
#endif
        RegisterCrtlCSignalHandler();
        // Convert all arguments to string list.
        std::vector<std::string> args = Utils::StringifyArgumentVector(argc, argv);
        std::unordered_map<std::string, std::string> environmentVars = Utils::StringifyEnvironmentPointer(envp);
        SourceManager sm;
        DiagnosticEngine diag;
        diag.SetSourceManager(&sm);
#ifdef _WIN32
        auto maybeExePath = Utils::GetApplicationPath();
#else
        auto maybeExePath = Utils::GetApplicationPath(args[0], environmentVars);
#endif
        if (!maybeExePath.has_value()) {
            return EXIT_CODE_ERROR;
        }
        std::string exePath = maybeExePath.value();
        std::string exeName = FileUtil::GetFileName(args[0]);
        // The program is executed by the symbolic link `codec-frontend`. Run in Frontend mode instead of Driver mode.
        if (exeName == "codec-frontend" || exeName == "codec-frontend.exe") {
            auto ret = ExecuteFrontend(exePath, args, environmentVars);
            RuntimeInit::GetInstance().CloseRuntime();
            TempFileManager::Instance().DeleteTempFiles();
            return ret;
        }
#ifdef SIGNAL_TEST
        // The interrupt signal triggers the function. In normal cases, this function does not take effect.
        Codira::SignalTest::ExecuteSignalTestCallbackFunc(Codira::SignalTest::TriggerPointer::MAIN_POINTER);
#endif
        std::unique_ptr<Driver> driver = std::make_unique<Driver>(args, diag, exePath);
        driver->EnvironmentSetup(environmentVars);
        if (!driver->ParseArgs()) {
            // Driver should have printed error messages,
            // but if driver didn't, users may be confused since codec did neither compilation
            // nor error reporting. Therefore, we add an error message (and also a help message) here.
            WriteError("Invalid options. Try: 'codec --help' for more information.\n");
            return EXIT_CODE_ERROR;
        }
        auto res = driver->ExecuteCompilation();
        TempFileManager::Instance().DeleteTempFiles();
        if (!res) {
            RuntimeInit::GetInstance().CloseRuntime();
            return EXIT_CODE_ERROR;
        }
        RuntimeInit::GetInstance().CloseRuntime();
#ifndef CODIRA_ENABLE_GCOV
    } catch (const NullPointerException& nullPointerException) {
        Codira::ICE::TriggerPointSetter iceSetter(nullPointerException.GetTriggerPoint());
#else
    } catch (const std::exception& nullPointerException) {
#endif
        InternalError("null pointer");
    }
    return EXIT_CODE_SUCCESS;
}
