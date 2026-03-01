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
 * This file declares the TempFileInfo and TempFileKind.
 */

#ifndef CODIRA_DRIVER_TEMP_FILE_INFO_H
#define CODIRA_DRIVER_TEMP_FILE_INFO_H

#include <string>

namespace Codira {
// A struct for passing output file info between Driver and Frontend.
struct TempFileInfo {
    std::string fileName;           // Record file name without suffix
    std::string filePath;           // Record the absolute file path.
    std::string rawPath{""};        // Record the original path of the file.
    bool isFrontendOutput{false};   // Record file is output by the frontend
    bool isForeignInput{false};     // Record whether it is a pre-compiled file (.bc/.o) provided by users.
};

enum class TempFileKind {
    O_CODEO,         // output .codeo file
    O_FULL_BCHIR, // output .full.bchir file
    O_BCHIR,      // output .bchir file
    T_BC,          // temp .bc(bitcode) file
    O_BC,          // output .bc(bitcode) file
    O_EXE,         // output executable file
    O_DYLIB,       // output dynamic library file
    O_STATICLIB,   // output static library file
    O_MACRO,       // output dynamic library file for macro
    O_CHIR,        // output CHIR serialization file
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    T_OPT_BC, // temp .opt.bc(optimized bitcode) file
    O_OPT_BC, // output .opt.bc(optimized bitcode) file
    T_ASM,    // temp .s(assembled) file
#endif
    T_OBJ,         // temp .o(binary object) file
    T_EXE_MAC,     // temp executable file for macro strip
    T_DYLIB_MAC,   // temp dynamic library file for macro strip
};

} // namespace Codira

#endif // CODIRA_DRIVER_TEMP_FILE_INFO_H
