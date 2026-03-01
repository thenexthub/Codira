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

#include "intrinsics.h"
#include "libarkbase/utils/logger.h"
#include "runtime/include/class.h"
#include "runtime/include/exceptions.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_string_builder.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "libarkbase/utils/utf.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "runtime/arch/memory_helpers.h"
#include <unistd.h>

#include "plugins/ets/runtime/intrinsics/helpers/ets_to_string_cache.h"

namespace ark::ets::intrinsics {

static inline constexpr size_t NULL_BYTES_NUM = 5;

EtsString *GetNullString()
{
    auto ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    auto vm = Runtime::GetCurrent()->GetPandaVM();

    std::array<uint8_t, NULL_BYTES_NUM> nullBytes = {'n', 'u', 'l', 'l', '\0'};

    return EtsString::FromCoreType(
        vm->GetStringTable()->GetOrInternString(nullBytes.data(), nullBytes.size() - 1, ctx));
}

EtsString *StdCoreToStringBoolean(EtsBoolean i)
{
    std::string s = i == 1 ? "true" : "false";
    return EtsString::CreateFromMUtf8(s.c_str());
}

EtsString *StdCoreToStringByte(EtsByte i)
{
    std::string s = std::to_string(i);
    return EtsString::CreateFromMUtf8(s.c_str());
}

EtsString *StdCoreToStringChar(EtsChar i)
{
    return EtsString::CreateFromUtf16(&i, 1);
}

EtsString *StdCoreToStringShort(EtsShort i)
{
    return StdCoreToStringLong(i);
}

EtsString *StdCoreToStringInt(EtsInt i)
{
    return StdCoreToStringLong(i);
}

EtsString *StdCoreToStringLong(EtsLong i)
{
    auto *cache = PandaEtsVM::GetCurrent()->GetLongToStringCache();
    if (UNLIKELY(cache == nullptr)) {
        return LongToStringCache::GetNoCache(i);
    }
    return cache->GetOrCache(EtsCoroutine::GetCurrent(), i);
}

ObjectHeader *StdCoreStringBuilderAppendString(ObjectHeader *sb, EtsString *str)
{
    return StringBuilderAppendString(sb, str);
}

ObjectHeader *StdCoreStringBuilderAppendString2(ObjectHeader *sb, EtsString *str0, EtsString *str1)
{
    return StringBuilderAppendStrings(sb, str0, str1);
}

ObjectHeader *StdCoreStringBuilderAppendString3(ObjectHeader *sb, EtsString *str0, EtsString *str1, EtsString *str2)
{
    return StringBuilderAppendStrings(sb, str0, str1, str2);
}

ObjectHeader *StdCoreStringBuilderAppendString4(ObjectHeader *sb, EtsString *str0, EtsString *str1, EtsString *str2,
                                                EtsString *str3)
{
    return StringBuilderAppendStrings(sb, str0, str1, str2, str3);
}

ObjectHeader *StdCoreStringBuilderAppendBool(ObjectHeader *sb, EtsBoolean v)
{
    return StringBuilderAppendBool(sb, v);
}

ObjectHeader *StdCoreStringBuilderAppendChar(ObjectHeader *sb, EtsChar v)
{
    return StringBuilderAppendChar(sb, v);
}

ObjectHeader *StdCoreStringBuilderAppendByte(ObjectHeader *sb, EtsByte v)
{
    return StringBuilderAppendLong(sb, static_cast<EtsLong>(v));
}

ObjectHeader *StdCoreStringBuilderAppendShort(ObjectHeader *sb, EtsShort v)
{
    return StringBuilderAppendLong(sb, static_cast<EtsLong>(v));
}

ObjectHeader *StdCoreStringBuilderAppendInt(ObjectHeader *sb, EtsInt v)
{
    return StringBuilderAppendLong(sb, static_cast<EtsLong>(v));
}

ObjectHeader *StdCoreStringBuilderAppendLong(ObjectHeader *sb, EtsLong v)
{
    return StringBuilderAppendLong(sb, v);
}

ObjectHeader *StdCoreStringBuilderAppendFloat(ObjectHeader *sb, EtsFloat v)
{
    return StringBuilderAppendFloat(sb, v);
}

ObjectHeader *StdCoreStringBuilderAppendDouble(ObjectHeader *sb, EtsDouble v)
{
    return StringBuilderAppendDouble(sb, v);
}

EtsString *StdCoreStringBuilderToString(ObjectHeader *sb)
{
    return StringBuilderToString(sb);
}

}  // namespace ark::ets::intrinsics
