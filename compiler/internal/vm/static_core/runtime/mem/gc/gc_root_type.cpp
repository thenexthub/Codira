/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "libarkbase/macros.h"
#include "runtime/mem/gc/gc_root_type.h"

namespace ark::mem {
std::ostream &operator<<(std::ostream &os, RootType rootType)
{
    switch (rootType) {
        case RootType::ROOT_UNKNOWN:
            os << "ROOT_UNKNOWN";
            break;
        case RootType::ROOT_CLASS:
            os << "ROOT_CLASS";
            break;
        case RootType::ROOT_FRAME:
            os << "ROOT_FRAME";
            break;
        case RootType::ROOT_THREAD:
            os << "ROOT_THREAD";
            break;
        case RootType::ROOT_CLASS_LINKER:
            os << "ROOT_CLASS_LINKER";
            break;
        case RootType::ROOT_TENURED:
            os << "ROOT_TENURED";
            break;
        case RootType::ROOT_VM:
            os << "ROOT_VM";
            break;
        case RootType::ROOT_NATIVE_GLOBAL:
            os << "ROOT_NATIVE_GLOBAL";
            break;
        case RootType::ROOT_NATIVE_LOCAL:
            os << "ROOT_NATIVE_LOCAL";
            break;
        case RootType::ROOT_PT_LOCAL:
            os << "ROOT_PT_LOCAL";
            break;
        case RootType::ROOT_AOT_STRING_SLOT:
            os << "ROOT_AOT_STRING_SLOT";
            break;
        case RootType::SATB_BUFFER:
            os << "SATB_BUFFER";
            break;
        case RootType::STRING_TABLE:
            os << "STRING_TABLE";
            break;
        case RootType::CARD_TABLE:
            os << "CARD_TABLE";
            break;
        default:
            UNREACHABLE();
    }
    return os;
}
}  // namespace ark::mem
