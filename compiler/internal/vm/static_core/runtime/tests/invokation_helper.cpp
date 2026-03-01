/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "invokation_helper.h"

extern "C" void InvokeHelper(...);

namespace ark::test {

const void *GetInvokeHelperImpl()
{
    return reinterpret_cast<const void *>(InvokeHelper);
}

void CountMethodTypes(panda_file::ShortyIterator &it, arch::ArgCounter<RUNTIME_ARCH> counter)
{
    switch ((*it).GetId()) {
        case panda_file::Type::TypeId::U1:
        case panda_file::Type::TypeId::U8:
        case panda_file::Type::TypeId::I8:
        case panda_file::Type::TypeId::I16:
        case panda_file::Type::TypeId::U16:
        case panda_file::Type::TypeId::I32:
        case panda_file::Type::TypeId::U32:
            counter.Count<int32_t>();
            break;
        case panda_file::Type::TypeId::F32:
            counter.Count<float>();
            break;
        case panda_file::Type::TypeId::F64:
            counter.Count<double>();
            break;
        case panda_file::Type::TypeId::I64:
        case panda_file::Type::TypeId::U64:
            counter.Count<int64_t>();
            break;
        case panda_file::Type::TypeId::REFERENCE:
            counter.Count<ObjectHeader *>();
            break;
        case panda_file::Type::TypeId::TAGGED:
            counter.Count<coretypes::TaggedValue>();
            break;
        default:
            UNREACHABLE();
    }
}

}  // namespace ark::test
