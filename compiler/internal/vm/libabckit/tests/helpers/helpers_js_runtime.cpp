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

#include "helpers_runtime.h"
#include "logger.h"
#include "ecmascript/napi/include/jsnapi.h"

#include <iostream>
#include <sstream>

namespace libabckit::test::helpers {

std::string ExecuteDynamicAbc(const std::string &abcPath, const std::string &entry)
{
    LIBABCKIT_LOG(DEBUG) << "ExecuteDynamicAbc: " << abcPath << ' ' << entry << '\n';

    panda::RuntimeOption option;
    option.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::ERROR);
    option.SetGcType(panda::RuntimeOption::GC_TYPE::STW);
    const size_t pause = 1000U;
    option.SetLongPauseTime(pause);
    panda::ecmascript::EcmaVM *vm = panda::JSNApi::CreateJSVM(option);

    std::stringstream ss;
    {
        CoutRedirect coutRedirect(ss.rdbuf());
        panda::JSNApi::Execute(vm, abcPath, entry, true);
    }
    panda::JSNApi::DestroyJSVM(vm);
    LIBABCKIT_LOG(DEBUG) << "Output:\n" << ss.str() << '\n';
    return ss.str();
}

}  // namespace libabckit::test::helpers
