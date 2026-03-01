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


#include <dlfcn.h>

#include "SanitizerInterface.h"

#include "Base/Log.h"
#include "Base/Macros.h"

namespace MapleRuntime {
namespace Sanitizer {
using namespace std;
#define SANITIZER_SYMBOL_DECL(ret_type, func, argc, ...) \
    typedef ret_type (*FUNC_TYPE(func))(ARG_MAP(argc, ARG_TYPE, __VA_ARGS__)); \
    FUNC_TYPE(func) PTR_TO_REAL(func) = nullptr // default to nullptr
#include "SymbolList.def"
#undef SANITIZER_SYMBOL_DECL

__attribute__((constructor)) ATTR_UNUSED void InitSanitizer()
{
#define SANITIZER_SYMBOL_DECL(ret_type, func, argc, ...) \
    REAL(func) = reinterpret_cast<SANITIZER_FUNC_TYPE(func)>(dlsym(RTLD_DEFAULT, #func)); \
    CHECK_DETAIL(REAL(func) != nullptr, "sanitizer function " #func "not found")
#include "SymbolList.def"
#undef SANITIZER_SYMBOL_DECL
}
} // namespace Sanitizer
} // namespace MapleRuntime
