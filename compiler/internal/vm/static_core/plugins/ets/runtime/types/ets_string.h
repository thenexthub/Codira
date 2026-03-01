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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_STRING_H_
#define PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_STRING_H_

#include <cmath>
#include "common_interfaces/objects/string/base_string-inl.h"
#include "libarkbase/utils/utf.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_box_primitive.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "runtime/include/runtime.h"
#include "runtime/include/coretypes/string.h"
#include "plugins/ets/runtime/types/ets_primitives.h"

namespace ark::ets {

// Private inheritance, because need to disallow implicit conversion to core type
class EtsString : private coretypes::String {
public:
    static EtsString *CreateFromMUtf8(const char *mutf8);

    static EtsString *CreateFromMUtf8(const char *mutf8, uint32_t utf16Length);

    static EtsString *CreateFromUtf8(const char *utf8, uint32_t length);

    static EtsString *CreateFromAscii(const char *str, uint32_t length);

    static EtsString *CreateFromUtf16(const uint16_t *utf16, uint32_t length);

    static EtsString *CreateNewStringFromCharCode(EtsDouble charCode);

    static EtsString *CreateNewStringFromChars(uint32_t offset, uint32_t length, EtsArray *chararray);

    static EtsString *CreateNewStringFromBytes(EtsArray *bytearray, EtsInt high, EtsInt offset, EtsInt length);

    static EtsString *CreateNewStringFromString(EtsString *etsString);

    static EtsString *CreateNewEmptyString();

    static EtsString *Resolve(const uint8_t *mutf8, uint32_t length)
    {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        return reinterpret_cast<EtsString *>(
            Runtime::GetCurrent()->ResolveString(Runtime::GetCurrent()->GetPandaVM(), mutf8, length, ctx));
    }

    static EtsString *Concat(EtsString *etsString1, EtsString *etsString2)
    {
        ASSERT(etsString1 != nullptr);
        ASSERT(etsString2 != nullptr);
        ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
        ASSERT(etsString1 != nullptr);
        ASSERT(etsString2 != nullptr);
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        coretypes::String *string3 = coretypes::String::Concat(etsString1->GetCoreType(), etsString2->GetCoreType(),
                                                               ctx, Runtime::GetCurrent()->GetPandaVM());
        return reinterpret_cast<EtsString *>(string3);
    }

    EtsString *TrimLeft();

    EtsString *TrimRight();

    EtsString *Trim();

    EtsBoolean StartsWith(EtsString *prefix, EtsInt fromIndex);

    EtsBoolean EndsWith(EtsString *suffix, EtsInt endIndex);

    int32_t Compare(EtsString *rhs)
    {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        return GetCoreType()->Compare(rhs->GetCoreType(), ctx);
    }

    uint16_t At(int32_t index)
    {
        return GetCoreType()->At(index);
    }

    static EtsString *DoReplace(EtsString *src, EtsChar oldC, EtsChar newC);

    static EtsString *FastSubString(EtsString *src, uint32_t start, uint32_t length)
    {
        ASSERT(src != nullptr);
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        coretypes::String *string1 = src->GetCoreType();
        coretypes::String *string2 =
            coretypes::String::FastSubString(string1, start, length, ctx, Runtime::GetCurrent()->GetPandaVM());
        return reinterpret_cast<EtsString *>(string2);
    }

    EtsArray *ToCharArray()
    {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        auto *ret = GetCoreType()->ToCharArray(ctx);
        if (UNLIKELY(ret == nullptr)) {
            return nullptr;
        }
        return reinterpret_cast<EtsArray *>(ret);
    }

    bool IsUtf16()
    {
        return GetCoreType()->IsUtf16();
    }

    bool IsUtf8()
    {
        return IsMUtf8();
    }

    bool IsMUtf8()
    {
        return !IsUtf16();
    }

    EtsInt GetLength()
    {
        return GetCoreType()->GetLength();
    }

    bool IsLineString()
    {
        return GetCoreType()->IsLineString();
    }

    bool IsTreeString()
    {
        return GetCoreType()->IsTreeString();
    }

    size_t CopyDataMUtf8(void *buf, size_t maxLength, bool isCString)
    {
        return GetCoreType()->CopyDataMUtf8(reinterpret_cast<uint8_t *>(buf), maxLength, isCString);
    }

    size_t CopyDataRegionUtf8(void *buf, size_t start, size_t length, size_t maxLength)
    {
        return GetCoreType()->CopyDataRegionUtf8(reinterpret_cast<uint8_t *>(buf), start, length, maxLength);
    }

    size_t CopyDataRegionMUtf8(void *buf, size_t start, size_t length, size_t maxLength)
    {
        return GetCoreType()->CopyDataRegionMUtf8(reinterpret_cast<uint8_t *>(buf), start, length, maxLength);
    }

    uint32_t CopyDataUtf16(void *buf, uint32_t maxLength)
    {
        return GetCoreType()->CopyDataUtf16(reinterpret_cast<uint16_t *>(buf), maxLength);
    }

    uint32_t CopyDataRegionUtf16(void *buf, uint32_t start, uint32_t length, uint32_t maxLength)
    {
        return GetCoreType()->CopyDataRegionUtf16(reinterpret_cast<uint16_t *>(buf), start, length, maxLength);
    }

    std::string_view ConvertToStringView([[maybe_unused]] PandaVector<uint8_t> *buf);

    uint32_t GetMUtf8Length()
    {
        return GetCoreType()->GetMUtf8Length();
    }

    uint32_t GetUtf16Length()
    {
        return GetCoreType()->GetUtf16Length();
    }

    uint32_t GetUtf8Length()
    {
        return GetCoreType()->GetUtf8Length();
    }

    bool IsEqual(const char *str)
    {
        auto *mutf8Str = utf::CStringAsMutf8(str);
        return coretypes::String::StringsAreEqualMUtf8(GetCoreType(), mutf8Str, utf::MUtf8ToUtf16Size(mutf8Str));
    }

    PandaString GetMutf8()
    {
        size_t len = GetMUtf8Length();
        PandaString out;
        out.resize(len - 1);
        if (!IsUtf16()) {
            // CopyData expects one byte preserved for zero at the end. For utf16 it will preserve itself
            --len;
        }
        CopyDataMUtf8(out.data(), len, false);
        return out;
    }

    PandaString GetUtf8()
    {
        size_t len = GetUtf8Length();
        PandaString out;
        out.resize(len);
        CopyDataRegionUtf8(out.data(), 0, GetCoreType()->GetLength(), len);
        return out;
    }

    uint8_t *GetDataUtf8()
    {
        return GetCoreType()->GetDataUtf8();
    }

    uint8_t *GetTreeStringDataUtf8(PandaVector<uint8_t> &buf)
    {
        return GetCoreType()->GetTreeStringDataUtf8(buf);
    }

    uint8_t *GetDataMUtf8()
    {
        return GetCoreType()->GetDataMUtf8();
    }

    uint8_t *GetTreeStringDataMUtf8(PandaVector<uint8_t> &buf)
    {
        return GetCoreType()->GetTreeStringDataMUtf8(buf);
    }

    uint16_t *GetDataUtf16()
    {
        return GetCoreType()->GetDataUtf16();
    }

    uint16_t *GetTreeStringDataUtf16(PandaVector<uint16_t> &buf)
    {
        return GetCoreType()->GetTreeStringDataUtf16(buf);
    }

    bool IsEmpty()
    {
        return GetCoreType()->IsEmpty();
    }

    uint32_t GetHashcode()
    {
        return GetCoreType()->GetHashcode();
    }

    /**
     * @brief read data from src to dest
     * @param [in] dest : dest string
     * @param [in] src : src string
     * @param [in] start : write to dest positioned at start offset
     * @param [in] destSize :  dest max size
     * @param [in] length : how many chars to copy
     */
    static void ReadData(EtsString *dest, EtsString *src, uint32_t start, uint32_t destSize, uint32_t length)
    {
        dest->WriteData(src, start, destSize, length);
    }

    /**
     * @brief copy data from src to dest , dest is specified by this string
     * @param [in] src : original data
     * @param [in] start : write to dest positioned at start offset
     * @param [in] destSize :  dest max size
     * @param [in] length : how many chars to copy
     */
    void WriteData(EtsString *src, uint32_t start, uint32_t destSize, uint32_t length)
    {
        GetCoreType()->WriteData(src->GetCoreType(), start, destSize, length);
    }

    bool StringsAreEqual(EtsObject *obj)
    {
        return coretypes::String::StringsAreEqual(GetCoreType(), FromEtsObject(obj)->GetCoreType());
    }

    coretypes::String *GetCoreType()
    {
        ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
        return reinterpret_cast<coretypes::String *>(this);
    }

    ObjectHeader *AsObjectHeader()
    {
        return reinterpret_cast<ObjectHeader *>(this);
    }

    EtsObject *AsObject()
    {
        return reinterpret_cast<EtsObject *>(this);
    }

    const EtsObject *AsObject() const
    {
        return reinterpret_cast<const EtsObject *>(this);
    }

    static EtsString *FromCoreType(coretypes::String *str)
    {
        return reinterpret_cast<EtsString *>(str);
    }

    static EtsString *FromCoreType(coretypes::LineString *str)
    {
        return reinterpret_cast<EtsString *>(str);
    }

    static EtsString *FromEtsObject(EtsObject *obj)
    {
        [[maybe_unused]] Class *cls = obj->GetClass()->GetRuntimeClass();
        [[maybe_unused]] Class *lineStrCls = Runtime::GetCurrent()
                                                 ->GetClassLinker()
                                                 ->GetExtension(panda_file::SourceLang::ETS)
                                                 ->GetClassRoot(ClassRoot::LINE_STRING);
        [[maybe_unused]] Class *slicedStrCls = Runtime::GetCurrent()
                                                   ->GetClassLinker()
                                                   ->GetExtension(panda_file::SourceLang::ETS)
                                                   ->GetClassRoot(ClassRoot::SLICED_STRING);
        [[maybe_unused]] Class *treeStrCls = Runtime::GetCurrent()
                                                 ->GetClassLinker()
                                                 ->GetExtension(panda_file::SourceLang::ETS)
                                                 ->GetClassRoot(ClassRoot::TREE_STRING);
        ASSERT(cls == lineStrCls || cls == slicedStrCls || cls == treeStrCls);
        return reinterpret_cast<EtsString *>(obj);
    }

    static EtsString *AllocateNonInitializedString(uint32_t length, bool compressed)
    {
        ASSERT(length != 0);
        ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        return reinterpret_cast<EtsString *>(
            coretypes::LineString::AllocLineStringObject(length, compressed, ctx, Runtime::GetCurrent()->GetPandaVM()));
    }

    static uint16_t CodeToChar(double code)
    {
        if (std::isnan(code) || std::isinf(code)) {
            return 0;
        }
        constexpr double UTF16_CHAR_DIVIDER = 0x10000;
        return static_cast<uint16_t>(static_cast<int64_t>(std::fmod(code, UTF16_CHAR_DIVIDER)));
    }

    EtsString() = delete;
    ~EtsString() = delete;

    NO_COPY_SEMANTIC(EtsString);
    NO_MOVE_SEMANTIC(EtsString);

private:
    friend EtsString *StringBuilderToString(ObjectHeader *sb);
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_STRING_H_
