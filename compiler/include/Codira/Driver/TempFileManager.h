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
 * This file declares the TempFileManager class.
 */

#ifndef CODIRA_DRIVER_TEMP_FILES_UTIL_H
#define CODIRA_DRIVER_TEMP_FILES_UTIL_H

#include <atomic>
#include <string>
#include <unordered_map>
#include <vector>

#include "Codira/Driver/TempFileInfo.h"
#include "Codira/Option/Option.h"

namespace Codira {
class TempFileManager {
public:
    /**
     * @brief Disable the copy constructor of class TempFileManager.
     */
    TempFileManager(TempFileManager const&) = delete;

    /**
     * @brief Disable the copy assignment operator of class TempFileManager.
     */
    TempFileManager& operator=(TempFileManager const&) = delete;

    /**
     * @brief Obtains the globally unique TempFileManager instance.
     *
     * @return TempFileManager The globally unique TempFileManager instance.
     */
    static TempFileManager& Instance()
    {
        static TempFileManager manager{};
        return manager;
    }

    /**
     * @brief Initialize the constructed TempFileManager.
     *
     * @param options GlobalOptions Instance.
     * @param isFrontend It is codec-frontend or codec being executed.
     * @return bool Return true if Initialization succeeded.
     */
    bool Init(const GlobalOptions& options, bool isFrontend);

    /**
     * @brief Create a new TempFileInfo whose type is kind.
     *
     * @param info Old TempFileInfo, It may only have the fileName field.
     * @param kind Type of TempFileInfo to be created.
     * @return TempFileInfo The new TempFileInfo.
     */
    TempFileInfo CreateNewFileInfo(const TempFileInfo& info, TempFileKind kind);

    /**
     * @brief Get the path of the temporary folder.
     * It may be a generated temporary directory or a user specified location.
     *
     * @return std::string The path of the temporary folder.
     */
    std::string GetTempFolder();

    /**
     * @brief Delete all temporary files.
     *
     * @param isSignalSafe Whether temporary files are safe.
     */
    void DeleteTempFiles(bool isSignalSafe = false);

    /**
     * @brief Check whether temporary files are deleted.
     *
     * @return bool Return true If temporary files are deleted.
     */
    bool IsDeleted() const;

private:
    bool isCodecFrontend{false};
    GlobalOptions opts{};
    std::string tempDir{};
    std::string outputDir{};
    std::string outputName{};
    std::vector<std::string> deletedFiles{};
    std::atomic<uint8_t> isDeleted{0}; // 0: not deleted, 1: deleting, 2: deleted
    TempFileManager(){};
    bool InitOutPutDir();
    bool InitTempDir();
    TempFileInfo CreateIntermediateFileInfo(const TempFileInfo& info, TempFileKind kind);
    TempFileInfo CreateTempBcFileInfo(const TempFileInfo& info, TempFileKind kind);
    TempFileInfo CreateOutputFileInfo(const TempFileInfo& info, TempFileKind kind);
    std::unordered_map<TempFileKind, std::function<std::string()>> fileSuffixMap;
    TempFileInfo CreateTempFileInfo(const TempFileInfo& info, TempFileKind kind);
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    TempFileInfo CreateLinuxLLVMOptOutputBcFileInfo(const TempFileInfo& info, TempFileKind kind);
#endif
    std::string GetDylibSuffix() const;
};

} // namespace Codira

#endif // CODIRA_DRIVER_TEMP_FILES_UTIL_H
