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

#ifndef PANDA_RUNTIME_CORETYPES_COMMON_STRING_FLATTEN_H_
#define PANDA_RUNTIME_CORETYPES_COMMON_STRING_FLATTEN_H_

#include "runtime/include/coretypes/string.h"

namespace ark::coretypes {
class FlatStringInfo {
public:
    /**
     * @brief flow flatten string to line string
     * @param [in] str : string to be flatten
     * @return the flattened string
     */
    static String *SlowFlatten(VMHandle<String> &str, const LanguageContext &ctx);

    /**
     * @brief flow flatten string to line string with native memory
     * @param [in] str : string to be flatten
     * @return the flattened string
     */
    static String *SlowFlattenWithNativeMemory(VMHandle<String> &str, const LanguageContext &ctx);

    /**
     * @brief flatten tree string to line string
     * @param [in] treeStr : tree string to be flatten
     * @param [in] withNativeMemory : default is false , with gc managed memory
     *        in intrinsic Compare method , we will use native memory , as there is no stackmap in
     *        intrinsic method , so can not happen gc in intrinsic method
     * @return the flattened string
     */
    static FlatStringInfo FlattenTreeString(VMHandle<String> &treeStr, const LanguageContext &ctx,
                                            bool withNativeMemory = false);

    /**
     * @brief flatten sliced string to line string
     * @param [in] slicedStr : sliced string to be flatten
     * @return the flattened string
     */
    static FlatStringInfo FlattenSlicedString(VMHandle<String> &slicedStr);

    /**
     * @brief flatten str to line string , in order to use it comfortably
     *        LineString --> LineString
     *        SlicedString --> LineString
     *        TreeString --> flatten every node in the tree to a LineString
     * @param [in] str : str to be flatten
     * @param [in] withNativeMemory : default is false , with gc managed memory
     *        in intrinsic Compare method , we will use native memory , as there is no stackmap in
     *        intrinsic method , so can not happen gc in intrinsic method
     * @return the flattened string
     */
    static FlatStringInfo FlattenAllString(VMHandle<String> &str, const LanguageContext &ctx,
                                           bool withNativeMemory = false);

    FlatStringInfo(String *string, uint32_t startIndex, uint32_t length, bool withNativeMemory = false)
        : string_(string), startIndex_(startIndex), length_(length), withNativeMemory_(withNativeMemory)
    {
    }

    NO_COPY_SEMANTIC(FlatStringInfo);

    FlatStringInfo(FlatStringInfo &&info) noexcept;
    FlatStringInfo &operator=(FlatStringInfo &&info) noexcept;

    ~FlatStringInfo();

    bool IsUtf8() const
    {
        return string_->IsMUtf8();
    }

    bool IsUtf16() const
    {
        return string_->IsUtf16();
    }

    String *GetString() const
    {
        return string_;
    }

    void SetString(String *string)
    {
        string_ = string;
    }

    uint32_t GetStartIndex() const
    {
        return startIndex_;
    }

    void SetStartIndex(uint32_t index)
    {
        startIndex_ = index;
    }

    uint32_t GetLength() const
    {
        return length_;
    }

    const uint8_t *GetDataUtf8() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return string_->GetDataUtf8() + startIndex_;
    }

    const uint16_t *GetDataUtf16() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic, clang-analyzer-core.CallAndMessage)
        return string_->GetDataUtf16() + startIndex_;  // NOLINT(clang-analyzer-core.NullDereference)
    }

    uint8_t *GetDataUtf8Writable() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return string_->GetDataUtf8Writable() + startIndex_;
    }

    uint16_t *GetDataUtf16Writable() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return string_->GetDataUtf16Writable() + startIndex_;
    }

    std::u16string ToU16String(uint32_t len = 0)
    {
        uint32_t length = len > 0 ? len : GetLength();
        std::u16string result;
        if (IsUtf16()) {
            const uint16_t *data = this->GetDataUtf16();
            result = common::Utf16ToU16String(data, length);
        } else {
            const uint8_t *data = this->GetDataUtf8();
            result = common::Utf8ToU16String(data, length);
        }
        return result;
    }

private:
    String *string_ {nullptr};
    uint32_t startIndex_ {0};
    uint32_t length_ {0};
    bool withNativeMemory_ {false};
};

}  // namespace ark::coretypes

#endif  // PANDA_RUNTIME_CORETYPES_COMMON_STRING_FLATTEN_H_