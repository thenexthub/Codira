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

#ifndef LSPSERVER_CONSTANTS_H
#define LSPSERVER_CONSTANTS_H

#include <set>
namespace CONSTANTS {
#ifdef __WIN32
    const std::string BLANK = "\r\n";
#else
    const std::string STRING_BLANK = "\\n";
    const std::string BLANK = "\n";
#endif
    const std::string BUILD_FOLDER_RELEASE = "target/release";
    const std::string BUILD_FOLDER_DEBUG = "target/debug";
    const std::string TARGET_LIB = "targetLib";
    const std::string SOURCE_CODE_DIR = "src";
    const std::string MULTI_MODULE_OPTION = "multiModuleOption";
    const std::string CACHE_PATH = "cachePath";
    const std::string CONDITION_COMPILE_OPTION = "conditionCompileOption";
    const std::string MODULE_CONDITION_COMPILE_OPTION = "moduleConditionCompileOption";
    const std::string SINGLE_CONDITION_COMPILE_OPTION = "singleConditionCompileOption";
    const std::string CONDITION_COMPILE_PATHS = "conditionCompilePaths";
    const std::string MODULES_HOME_OPTION = "modulesHomeOption";
    const std::string STD_LIB_PATH_OPTION = "stdLibPathOption";
    const std::string STD_CODED_PATH_OPTION = "stdCodedPathOption";
    const std::string OHOS_CODED_PATH_OPTION = "ohosCodedPathOption";
    const std::string CODED_CACHE_PATH_OPTION = "codedCachePathOption";
    const std::string SLASH = "/";
    const std::string DOT = ".";
    const std::string FILE_SEPARATOR = "/\\";
    // key of module.json
    const std::string MODULE_JSON_NAME = "name";
    const std::string REQUIRES = "requires";
    const std::string MODULE_JSON_PATH = "path";
    const std::string PACKAGES_REQUIRES = "package_requires";
    const std::string PACKAGE_OPTION = "package_option";
    const std::string PATH_OPTION = "path_option";
    const std::string DEFAULT_ROOT_PACKAGE = "default";
    const std::string SRC_PATH = "src_path";
    const std::string COMBINED = "combined";
    const std::string IMPORT = "import";
    const std::string AS = "as";
    const std::string WHITE_SPACE = " ";
    const std::string COMMA = ":";
    const std::set<std::string> BUILTIN_OPERATORS = {"@", ".", "[]", "()", "++", "--", "?", "!", "-", "**", "*", "/",
                                                     "%", "+", "<<", ">>", "<", "<=", ">", ">=", "is", "as", "==",
                                                     "!=", "&", "^", "|", "..", "..=", "&&", "||", "\?\?",
                                                     "~>", "=", "**=", "*=", "/=", "%/", "+=", "-=", "<<=", ">>=",
                                                     "&=", "^=", "|="};
    // CODIRA file extension
    const std::string CODIRA_FILE_EXTENSION = "code";
    // codec auto create file about macro expand
    const std::string CODIRA_MACRO_FILE_EXTENSION = "macrocall";
    const unsigned int AD_OFFSET = 2;
    constexpr size_t MAC_THREAD_STACK_SIZE = 1024 * 1024 * 8;
    const unsigned int SORT_TEXT_SIZE = 6;
    const float ROUND_NUM = 0.5;
    const std::string DOUBLE_COLON = "::";
} // namespace CONSTANTS

#endif // LSPSERVER_CONSTANTS_H
