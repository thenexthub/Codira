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

#ifndef ANI_TEXTDECODER_H
#define ANI_TEXTDECODER_H
#include <ani.h>
#include <memory.h>
#include <string>
#include <vector>

#include "unicode/ucnv.h"
#include "libarkbase/utils/span.h"
#include "tools/format_logger.h"
namespace ark::ets::sdk::util {
using TransformToolPointer = std::unique_ptr<UConverter, void (*)(UConverter *)>;

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class TextDecoder {
public:
    enum class ConverterFlags {
        FLUSH_FLG = 0x1,
        FATAL_FLG = 0x2,
        IGNORE_BOM_FLG = 0x4,
        UNICODE_FLG = 0x8,
        BOM_SEEN_FLG = 0x10,
        BOM_FLAG = 0xFEFF,
    };

public:
    /**
     * Constructor of textdecoder
     *
     * @param buff Encoding format.
     * @param optionVec There are two attributes of code related option parameters: fatal and ignorebom.
     */
    TextDecoder(std::string &buff, uint32_t flags);
    ~TextDecoder() = default;
    /**
     * Destructor of textencoder.
     *
     * @param env ANI environment parameters.
     * @param src An array that matches the format and needs to be decoded.
     * @param iflag Decoding related option parameters.
     */
    ani_string DecodeToString(ani_env *env, const char *source, int32_t byteOffset, uint32_t length, bool iflag);
    size_t GetMinByteSize() const;
    void Reset() const;
    UConverter *GetConverterPtr() const
    {
        return tranTool_.get();
    }
    bool IsUnicode() const
    {
        uint32_t temp = label_ & static_cast<uint32_t>(ConverterFlags::UNICODE_FLG);
        return temp == static_cast<uint32_t>(ConverterFlags::UNICODE_FLG);
    }

    bool IsIgnoreBom() const
    {
        uint32_t temp = label_ & static_cast<uint32_t>(ConverterFlags::IGNORE_BOM_FLG);
        return temp == static_cast<uint32_t>(ConverterFlags::IGNORE_BOM_FLG);
    }
    static void ConverterClose(UConverter *pointer)
    {
        ucnv_close(pointer);
    }

private:
    ani_string GetResultStr(ani_env *env, const UChar *arrDat, size_t length);
    void SetIgnoreBOM(const UChar *arr, size_t resultLen, bool &bomFlag);
    void FreedMemory(UChar *&pData);
    uint32_t label_ {};
    std::string encStr_ {};
    TransformToolPointer tranTool_;
};
}  // namespace ark::ets::sdk::util
#endif  // ANI_TEXTDECODER_H
