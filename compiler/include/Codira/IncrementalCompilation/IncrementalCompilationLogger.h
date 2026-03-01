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

#ifndef CODIRA_INCREMENTAL_COMPILATION_LOGER_H
#define CODIRA_INCREMENTAL_COMPILATION_LOGER_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "Codira/Utils/FileUtil.h"
#include "Codira/Utils/ConstantsUtils.h"

/// Log states of incremental compilation into log file
class IncrementalCompilationLogger {
public:
    static IncrementalCompilationLogger& GetInstance()
    {
        static IncrementalCompilationLogger logger = IncrementalCompilationLogger();
        return logger;
    }
    void SetDebugPrint(bool flag)
    {
        debugPrint = flag;
    }
    void InitLogFile(const std::string& logFilePath)
    {
        if (logFilePath.empty()) {
            return;
        }
        if (!Codira::FileUtil::HasExtension(logFilePath, "log")) {
            return;
        }
        auto realDirPath = Codira::FileUtil::GetAbsPath(Codira::FileUtil::GetDirPath(logFilePath));
        if (!realDirPath.has_value()) {
            return;
        }
        auto fileNameWithExt = Codira::FileUtil::GetFileName(logFilePath);
        auto ret = Codira::FileUtil::JoinPath(realDirPath.value(), fileNameWithExt);
        fileStream.open(ret, std::ofstream::out);
        if (fileStream.is_open()) {
            writeKind = WriteKind::FILE;
            saveLogFile = true;
        }
    }
    void LogLn(const std::string& input)
    {
        if (debugPrint) {
            std::cout << input << std::endl;
        }
        if (writeKind == WriteKind::FILE) {
            fileStream << input << std::endl;
        } else {
            strStream << input << std::endl;
        }
    }
    void Log(const std::string& input)
    {
        if (debugPrint) {
            std::cout << input;
        }
        if (writeKind == WriteKind::FILE) {
            fileStream << input;
        } else {
            strStream << input;
        }
    }
    bool IsEnable() const
    {
        return debugPrint || saveLogFile;
    }
    enum class WriteKind : uint8_t { BUFF, FILE };
    void SetWriteKind(WriteKind kind)
    {
        writeKind = kind;
    }
    void WriteBuffToFile()
    {
        if (writeKind == WriteKind::FILE) {
            if (fileStream.is_open()) {
                fileStream << strStream.str();
                writeKind = WriteKind::FILE;
            }
        }
    }

private:
    IncrementalCompilationLogger() {}
    ~IncrementalCompilationLogger() noexcept
    {
#ifndef CODIRA_ENABLE_GCOV
        try {
#endif
            if (fileStream.is_open()) {
                fileStream.close();
            }
#ifndef CODIRA_ENABLE_GCOV
        } catch (const std::exception& e) {
            // do nothing for noexcept
        } catch (...) {
            // do nothing for noexcept
        }
#endif
    }
    bool debugPrint{false};
    bool saveLogFile{false};
    std::ostringstream strStream;
    std::ofstream fileStream;
    WriteKind writeKind{WriteKind::BUFF};
};

#endif
