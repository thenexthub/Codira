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

#include "ets_string_helpers.h"

#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"

namespace ark::ets::intrinsics::helpers {

EtsString *CreateNewStringFromCharCode(EtsObjectArray *charCodes, size_t actualLength)
{
    if (actualLength == 0) {
        return EtsString::CreateNewEmptyString();
    }

    // allocator may trig gc and move the 'charCodes' array, need to hold it
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    EtsHandle arrayHandle(coroutine, charCodes);

    auto isCompressed = [actualLength](EtsObjectArray *codes) -> bool {
        if (!Runtime::GetOptions().IsRuntimeCompressedStringsEnabled()) {
            return false;
        }

        for (size_t i = 0; i < actualLength; ++i) {
            if (!coretypes::String::IsASCIICharacter(
                    // CC-OFFNXT(G.FMT.06-CPP) project code style
                    EtsString::CodeToChar(EtsBoxPrimitive<EtsDouble>::FromCoreType(codes->Get(i))->GetValue()))) {
                return false;
            }
        }
        return true;
    };

    auto copyCharsIntoString = [actualLength](EtsObjectArray *codes, auto *dstData) {
        Span<std::remove_pointer_t<decltype(dstData)>> to(dstData, actualLength);
        for (size_t i = 0; i < actualLength; ++i) {
            to[i] = EtsString::CodeToChar(EtsBoxPrimitive<EtsDouble>::FromCoreType(codes->Get(i))->GetValue());
        }
    };

    bool compress = isCompressed(arrayHandle.GetPtr());
    EtsString *s = EtsString::AllocateNonInitializedString(actualLength, compress);
    if (compress) {
        copyCharsIntoString(arrayHandle.GetPtr(), s->GetDataMUtf8());
    } else {
        copyCharsIntoString(arrayHandle.GetPtr(), s->GetDataUtf16());
    }
    return s;
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP, G.FUD.05) solid logic
EtsString *CreateNewStringFromCharCode(EtsEscompatArray *charCodes)
{
    ASSERT(charCodes != nullptr);

    auto *coro = EtsCoroutine::GetCurrent();
    if (LIKELY(charCodes->IsExactlyEscompatArray(coro))) {
        // Fast path, `charCodes` is exactly `std.core.Array`
        return CreateNewStringFromCharCode(charCodes->GetDataFromEscompatArray(),
                                           charCodes->GetActualLengthFromEscompatArray());
    }

    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle arrayHandle(coro, charCodes);

    EtsInt uncheckedActualLength = 0;
    if (UNLIKELY(!arrayHandle->GetLength(coro, &uncheckedActualLength))) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    if (UNLIKELY(uncheckedActualLength == 0)) {
        return EtsString::CreateNewEmptyString();
    }

    PandaVector<uint16_t> copiedChars;
    auto actualLength = static_cast<size_t>(uncheckedActualLength);
    copiedChars.reserve(actualLength);

    // Returns `true` if there are no errors being thrown, `false` otherwise
    auto charCodesExtractor = [coro, actualLength, &arrayHandle](bool &isCompressed,
                                                                 PandaVector<uint16_t> &outputChars) {
        if (!Runtime::GetOptions().IsRuntimeCompressedStringsEnabled()) {
            isCompressed = false;
            return true;
        }

        // By default string is considered to be compressed
        isCompressed = true;
        for (size_t i = 0; i < actualLength; ++i) {
            auto optElement = arrayHandle->GetRef(coro, i);
            if (UNLIKELY(!optElement)) {
                ASSERT(coro->HasPendingException());
                return false;
            }
            if (UNLIKELY(*optElement == nullptr)) {
                PandaStringStream ss;
                ss << "element at index " << i << " is undefined";
                ThrowEtsException(coro, panda_file_items::class_descriptors::NULL_POINTER_ERROR, ss.str());
                return false;
            }
            uint16_t codeValue =
                EtsString::CodeToChar(EtsBoxPrimitive<EtsDouble>::FromCoreType(*optElement)->GetValue());
            outputChars.emplace_back(codeValue);
            if (isCompressed && !coretypes::String::IsASCIICharacter(codeValue)) {
                isCompressed = false;
            }
        }
        return true;
    };

    auto copyCharsIntoString = [](const PandaVector<uint16_t> &inputChars, auto dstData) {
        ASSERT(inputChars.size() == dstData.size());
        for (size_t i = 0; i < dstData.size(); ++i) {
            dstData[i] = inputChars[i];
        }
    };

    bool compress = false;
    if (UNLIKELY(!charCodesExtractor(compress, copiedChars))) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }

    EtsString *s = EtsString::AllocateNonInitializedString(actualLength, compress);
    if (compress) {
        copyCharsIntoString(copiedChars, Span<uint8_t>(s->GetDataMUtf8(), actualLength));
    } else {
        copyCharsIntoString(copiedChars, Span<uint16_t>(s->GetDataUtf16(), actualLength));
    }
    return s;
}

}  // namespace ark::ets::intrinsics::helpers
