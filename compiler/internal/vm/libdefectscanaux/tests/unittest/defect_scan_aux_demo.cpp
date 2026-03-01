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

#include "defect_scan_aux_api.h"

int main()
{
    std::cout << "===== [libark_defect_scan_aux] Running start =====" << std::endl;
    {
        std::string_view abc_filename = "test.abc";
        auto abc_file = panda::defect_scan_aux::AbcFile::Open(abc_filename);
        if (abc_file == nullptr) {
            std::cerr << "  --- abc file obj is null ---" << std::endl;
            std::cerr << "  --- please add the file to be detected to the executable file path ---" << std::endl;
            return 0;
        }
        size_t def_func_cnt = abc_file->GetDefinedFunctionCount();
        std::cout << "  --- function count: " << def_func_cnt << " ---" << std::endl;
        size_t def_class_cnt = abc_file->GetDefinedClassCount();
        std::cout << "  --- class count: " << def_class_cnt << " ---" << std::endl;
    }
    std::cout << "===== [libark_defect_scan_aux] end =====" << std::endl;

    return 0;
}
