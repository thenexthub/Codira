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
 * This file is the main entry of chir-dis.
 */

#include <string>
#include <unordered_set>
#include <exception>

#include "Codira/Basic/Print.h"
#include "Codira/Basic/Version.h"
#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/CHIR/CHIRPrinter.h"
#include "Codira/CHIR/Serializer/CHIRDeserializer.h"
#include "Codira/Utils/FileUtil.h"
#include "Codira/Utils/Signal.h"
#include "Codira/Utils/Utils.h"

using namespace Codira;

namespace {
const int EXIT_CODE_SUCCESS = 0;
const int EXIT_CODE_ERROR = 1; // Normal compiler error

const std::string_view CHIR_DIS_USAGE = R"(A tool used to deserialize and dump CHIR.

Overview: chir-dis xxx.chir -> xxx.chirtxt

Usage:
  chir-dis [option] file

Options:
  -v                      print compiler version information.
  -h                      print this help.)";

struct ActionInfo {
    bool printHelp{false};
    bool printVersion{false};
    std::string inputFilePath{};
};

void RegisterSignalHandler()
{
#if (defined RELEASE) && (!defined CODIRA_ENABLE_CODECFUZZ)
#if (defined __unix__)
    CreateAltSignalStack();
#elif (defined _WIN32)
    RegisterCrashExceptionHandler();
#endif
    RegisterCrashSignalHandler();
#endif
    RegisterCrtlCSignalHandler();
}

std::string GetOptionName(const std::string& arg)
{
    size_t pos = arg.find_first_of('=');
    if (pos != std::string::npos) {
        return arg.substr(0, pos);
    }
    return arg;
}

bool ParseArgs(const std::vector<std::string>& args, ActionInfo& info)
{
    const std::string_view helpOption = "-h";
    const std::string_view versionOption = "-v";
    bool multiInput{false};

    if (args.size() == 1) {
        Errorln("expected one serialization file of CHIR.");
        Println(CHIR_DIS_USAGE);
        return false;
    }
    for (size_t i = 1; i < args.size(); i++) {
        if (args[i] == helpOption) {
            info.printHelp = true;
            continue;
        }
        if (args[i] == versionOption) {
            info.printVersion = true;
            continue;
        }
        if (args[i].front() == '-') {
            Errorln("invalid option: '", GetOptionName(args[i]), "'");
            Println(CHIR_DIS_USAGE);
            return false;
        }
        if (!info.inputFilePath.empty()) {
            multiInput = true;
            continue;
        }
        info.inputFilePath = args[i];
    }
    if (info.printHelp) {
        return true;
    }
    if (multiInput) {
        Errorln("Only one file can be entered at a time");
        Println(CHIR_DIS_USAGE);
        return false;
    }
    return true;
}

bool DeserializeInputCHIR(const std::string& path)
{
    auto access = FileUtil::AccessWithResult(".", FileUtil::FileMode::FM_WRITE);
    switch (access) {
        case FileUtil::AccessResultType::NOT_EXIST:
            CODEC_ABORT();
            return false;
        case FileUtil::AccessResultType::NO_PERMISSION:
            Errorln("can't access current directory to write .chirtxt due no permisson");
            return false;
        case FileUtil::AccessResultType::FAILED_WITH_UNKNOWN_REASON:
            Errorln("can't access current directory to write .chirtxt for unknow reason");
            return false;
        case FileUtil::AccessResultType::OK:
            break;
    }
    std::unordered_map<unsigned int, std::string> fileNameMap;
    CHIR::CHIRContext cctx(&fileNameMap);
    CHIR::CHIRBuilder chirBuilder(cctx);
    CHIR::ToCHIR::Phase phase;
    if (!CHIR::CHIRDeserializer::Deserialize(path, chirBuilder, phase)) {
        return false;
    }
    std::string outputFilePath = FileUtil::GetFileNameWithoutExtension(path) + CHIR_READABLE_FILE_EXTENSION;
    // print serialize extension info which just for serialization not necessary for chir nodes
    CHIR::CHIRPrinter::PrintCHIRSerializeInfo(phase, outputFilePath);
    // print package
    CHIR::CHIRPrinter::PrintPackage(*cctx.GetCurPackage(), outputFilePath);
    return true;
}
} // namespace

int main(int argc, const char** argv)
{
    try {
        RegisterSignalHandler();
        auto args = Utils::StringifyArgumentVector(argc, argv);
        ActionInfo info;
        if (!ParseArgs(args, info)) {
            return EXIT_CODE_ERROR;
        }
        if (info.printHelp) {
            Println(CHIR_DIS_USAGE);
            return EXIT_CODE_SUCCESS;
        }
        if (info.printVersion) {
            Codira::PrintVersion();
            return EXIT_CODE_SUCCESS;
        }
        if (!DeserializeInputCHIR(info.inputFilePath)) {
            return EXIT_CODE_ERROR;
        }
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
