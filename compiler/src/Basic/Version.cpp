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
 * This file implements compiler's version apis.
 */

#include "Codira/Basic/Print.h"
#include "Codira/Basic/Version.h"

namespace Codira {
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
const std::string CODIRA_VERSION = CODE_SDK_VERSION;

#ifndef VERSION_TAIL
const std::string CODIRA_COMPILER_VERSION = "Codira Compiler: " + CODIRA_VERSION;
#else
const std::string CODIRA_COMPILER_VERSION = "Codira Compiler: " + CODIRA_VERSION + VERSION_TAIL;
#endif
#endif
void PrintVersion()
{
    Codira::Println(CODIRA_COMPILER_VERSION);
}
} // namespace Codira
