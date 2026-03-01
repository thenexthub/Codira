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
 * This file declares the map of standard library names.
 */

#ifndef CODIRA_DIRVIER_STDLIBMAP_H
#define CODIRA_DIRVIER_STDLIBMAP_H

#include <string>
#include <unordered_map>

namespace Codira {
const std::string GET_COMMAND_LINE_ARGS = "getCommandLineArgs";
const std::string MODULE_SPLIT = "/";

const std::unordered_map<std::string, std::string> STANDARD_LIBS = {
#define STDLIB(NAME, MODULE, SUB_PACKAGE) {MODULE "." SUB_PACKAGE, SUB_PACKAGE},
#define STDLIB_ROOTPKG(NAME, ROOT_PACKAGE) {ROOT_PACKAGE, ROOT_PACKAGE},
#define STDLIB_TOPPKG(NAME, TOP_PACKAGE) {TOP_PACKAGE, TOP_PACKAGE},
#include "Codira/Driver/Stdlib.inc"
#undef STDLIB_TOPPKG
#undef STDLIB_ROOTPKG
#undef STDLIB
};
} // namespace Codira
#endif
