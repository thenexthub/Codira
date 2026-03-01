/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_JSON_HELPER
#define PANDA_PLUGINS_ETS_RUNTIME_JSON_HELPER

#include "types/ets_string.h"

namespace ark::ets::intrinsics::helpers {
class JSONStringifier {
public:
    explicit JSONStringifier() = default;

    EtsString *Stringify(EtsHandle<EtsObject> &value);

private:
    static inline bool IsHighSurrogate(uint16_t ch)
    {
        return ch >= utf::DECODE_LEAD_LOW && ch <= utf::DECODE_LEAD_HIGH;
    }

    static inline bool IsLowSurrogate(uint16_t ch)
    {
        return ch >= utf::DECODE_TRAIL_LOW && ch <= utf::DECODE_TRAIL_HIGH;
    }

    inline void AppendUnicodeEscape(uint32_t ch)
    {
        buffer_ += "\\u";
        buffer_ += HEX_DIGIT[(ch >> HEX_SHIFT_THREE) & HEX_DIGIT_MASK];
        buffer_ += HEX_DIGIT[(ch >> HEX_SHIFT_TWO) & HEX_DIGIT_MASK];
        buffer_ += HEX_DIGIT[(ch >> HEX_SHIFT_ONE) & HEX_DIGIT_MASK];
        buffer_ += HEX_DIGIT[ch & HEX_DIGIT_MASK];
    }

private:
    bool AppendJSONString(EtsHandle<EtsObject> &value, bool hasContent);
    void AppendJSONPrimitive(const PandaString &value, bool hasContent);

    bool SerializeFields(EtsCoroutine *coro, EtsHandle<EtsObject> &value, bool &hasContent);
    bool SerializeGetters(EtsCoroutine *coro, EtsHandle<EtsObject> &value, bool &hasContent);
    bool SerializeInterfaceList(EtsCoroutine *coro, EtsHandle<EtsObject> &value, bool &hasContent);

    // handling of types
    bool SerializeJSONObject(EtsHandle<EtsObject> &value);
    bool SerializeJSONObjectArray(EtsHandle<EtsObject> &value);

    void AppendUtf16ToQuotedString(const Span<const uint16_t> &sp);
    void AppendUtf8ToQuotedString(const Span<const uint8_t> &sp);
    uint32_t AppendUtf8Character(const Span<const uint8_t> &sp, uint32_t index);
    void DealUtf16Character(uint16_t ch0, uint16_t ch1);
    bool SerializeJSONString(EtsHandle<EtsObject> &value);
    bool DealSpecialCharacter(uint8_t ch);

    bool SerializeJSONBoxedBoolean(EtsHandle<EtsObject> &value);
    bool SerializeJSONBoxedDouble(EtsHandle<EtsObject> &value);
    bool SerializeJSONBoxedFloat(EtsHandle<EtsObject> &value);
    bool SerializeJSONBoxedLong(EtsHandle<EtsObject> &value);

    template <typename T>
    bool SerializeJSONBoxedPrimitiveNoCache(EtsHandle<EtsObject> &value);

    bool SerializeEmptyObject();
    bool SerializeJSONNullValue();
    bool SerializeJSONRecord(EtsHandle<EtsObject> &value);
    bool SerializeJsonSerializable(EtsHandle<EtsObject> &value);

    bool PushValue(EtsHandle<EtsObject> &value);
    void PopValue(EtsHandle<EtsObject> &value);

    PandaString ResolveDisplayName(const EtsField *field);

    bool SerializeObject(EtsHandle<EtsObject> &value);
    bool HandleField(EtsCoroutine *coro, EtsHandle<EtsObject> &obj, EtsField *field, bool &hasContent,
                     PandaUnorderedSet<PandaString> &keys);
    bool HandleRecordKey(EtsHandle<EtsObject> &key);
    template <typename T>
    PandaString HandleNumeric(EtsHandle<EtsObject> &value);

    bool CheckUnsupportedAnnotation(EtsField *field);

private:
    PandaString buffer_;
    PandaString key_;
    PandaUnorderedSet<uint32_t> path_;

    const bool modify_ = false;
    static constexpr uint8_t CODE_SPACE = 0x20;
    static constexpr unsigned HEX_DIGIT_MASK = 0xF;
    static constexpr unsigned HEX_SHIFT_THREE = 12;
    static constexpr unsigned HEX_SHIFT_TWO = 8;
    static constexpr unsigned HEX_SHIFT_ONE = 4;

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    static constexpr char HEX_DIGIT[] = "0123456789abcdef";
};
}  // namespace ark::ets::intrinsics::helpers
#endif  // PANDA_PLUGINS_ETS_RUNTIME_JSON_HELPER
