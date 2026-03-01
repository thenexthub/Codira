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

#ifndef LIBPANDAFILE_DATA_PROTECT_H
#define LIBPANDAFILE_DATA_PROTECT_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#if defined(PANDA_ENABLE_DATA_PROTECT)
#include <sys/auxv.h>
#include <asm/hwcap.h>
#endif

namespace panda::panda_file {

class DataProtect {
public:
    DataProtect() : protect_pointer_(0) {}
    explicit DataProtect(const uintptr_t pointer)
    {
        if (pointer == 0) {
            protect_pointer_ = 0;
            return;
        }
        protect_pointer_ = DataProtectPac(pointer, reinterpret_cast<uintptr_t>(&protect_pointer_));
    }

    ~DataProtect() = default;

    static inline uintptr_t DataProtectAut(const uintptr_t pointer, [[maybe_unused]]const uintptr_t address)
    {
#if defined(PANDA_ENABLE_DATA_PROTECT)
        uint64_t hwcaps = getauxval(AT_HWCAP);
        if (!(hwcaps & HWCAP_PACA) || !(hwcaps & HWCAP_PACG)) {
            return pointer;
        }
        void *t1 = reinterpret_cast<void*>(pointer);
        void *t2 = reinterpret_cast<void*>(address);
#ifdef PAC_DFI_PTR_BKEY
        __asm__ __volatile__("autdb %0, %1":"+r"(t1):"r"(t2):);
#else
        __asm__ __volatile__("autda %0, %1":"+r"(t1):"r"(t2):);
#endif
        return reinterpret_cast<uintptr_t>(t1);
#else
        return pointer;
#endif
    }

    static inline uintptr_t DataProtectPac(const uintptr_t pointer, [[maybe_unused]]const uintptr_t address)
    {
#if defined(PANDA_ENABLE_DATA_PROTECT)
        uint64_t hwcaps = getauxval(AT_HWCAP);
        if (!(hwcaps & HWCAP_PACA) || !(hwcaps & HWCAP_PACG)) {
            return pointer;
        }
        void *t1 = reinterpret_cast<void*>(pointer);
        void *t2 = reinterpret_cast<void*>(address);
#ifdef PAC_DFI_PTR_BKEY
        __asm__ __volatile__("pacdb %0, %1":"+r"(t1):"r"(t2):);
#else
        __asm__ __volatile__("pacda %0, %1":"+r"(t1):"r"(t2):);
#endif
        return reinterpret_cast<uintptr_t>(t1);
#else
        return pointer;
#endif
    }

    void Update(const uintptr_t pointer)
    {
        if (pointer == 0) {
            protect_pointer_ = 0;
            return;
        }
        protect_pointer_ = DataProtectPac(pointer, reinterpret_cast<uintptr_t>(&protect_pointer_));
    }

    uintptr_t GetOriginPointer() const
    {
        if (protect_pointer_ == 0) {
            return protect_pointer_;
        }
        return DataProtectAut(protect_pointer_, reinterpret_cast<uintptr_t>(&protect_pointer_));
    }
    
private:
    uintptr_t protect_pointer_;
};

class BoolPacProtect {
public:
    enum PacBoolean : uintptr_t {
        PAC_FALSE = 1,
        PAC_TRUE = 2,
    };

    BoolPacProtect()
    {
        Update(false);
    }

    explicit BoolPacProtect(const bool flag)
    {
        Update(flag);
    }

    void Update(const bool flag)
    {
        protect_bool_ = DataProtect::DataProtectPac(flag ? PAC_TRUE : PAC_FALSE,
            reinterpret_cast<uintptr_t>(&protect_bool_));
    }

    bool GetBool() const
    {
        auto value = DataProtect::DataProtectAut(protect_bool_, reinterpret_cast<uintptr_t>(&protect_bool_));
        return value == PAC_TRUE ? true : false;
    }

private:
    uintptr_t protect_bool_;
};

class StringPacProtect {
public:

    enum Shift : uint8_t {
        SHIFT8 = 8,
        SHIFT16 = 16,
        SHIFT24 = 24,
        SHIFT32 = 32
    };

    StringPacProtect() : data_(std::vector<uintptr_t>()), origin_length_(0) {}

    explicit StringPacProtect(std::string_view str_data)
    {
        Update(str_data);
    }

    ~StringPacProtect()
    {
        Clear();
    }

    // replace data with pac(str_data)
    void Update(std::string_view str_data)
    {
        Clear();
        AppendWithoutCheckBack(str_data);
    }

    void Append(char ch)
    {
        constexpr uint32_t step = 4;
        if (origin_length_ % step > 0) {
            // Filling back empty
            uint32_t empty_count = step - (origin_length_ % step);
            auto last_data = DataProtect::DataProtectAut(data_.back(), reinterpret_cast<uintptr_t>(&data_));
            data_.pop_back();
            uint8_t shift = (SHIFT8 * (empty_count - 1));
            last_data |= static_cast<uintptr_t>(ch) << shift;
            data_.push_back(DataProtect::DataProtectPac(last_data, reinterpret_cast<uintptr_t>(&data_)));
        } else {
            uintptr_t temp_data = uintptr_t(ch);
            temp_data <<= SHIFT24;
            data_.push_back(DataProtect::DataProtectPac(temp_data, reinterpret_cast<uintptr_t>(&data_)));
        }
        origin_length_++;
    }

    // data += pac(str_data)
    void Append(std::string_view str_data)
    {
        if (str_data.empty()) {
            return;
        }
        constexpr uint32_t step = 4;
        
        auto str = reinterpret_cast<const uint8_t *>(str_data.data());
        auto len = str_data.length();
        if (origin_length_ % step != 0) {
            // Filling back empty
            uint32_t iter = 0;
            uint32_t empty_count = step - (origin_length_ % step);
            auto last_data = DataProtect::DataProtectAut(data_.back(), reinterpret_cast<uintptr_t>(&data_));
            data_.pop_back();
            
            last_data >>= SHIFT8 * empty_count;
            while (iter < empty_count) {
                last_data <<= SHIFT8;
                last_data += (iter < len ? str[iter] : 0);
                iter++;
            }
            data_.push_back(DataProtect::DataProtectPac(last_data, reinterpret_cast<uintptr_t>(&data_)));
            origin_length_ += (empty_count < len ? empty_count : len);
            if (empty_count < len) {
                AppendWithoutCheckBack(str_data.substr(empty_count));
            }
        } else {
            AppendWithoutCheckBack(str_data);
        }
    }

    // return string(aut(data))
    std::string GetOriginString() const
    {
        if (data_.empty()) {
            return "";
        }

        std::string res = "";
        constexpr uintptr_t mask = 0xff000000;
        for (uint32_t iter = 0; iter < data_.size(); ++iter) {
            uintptr_t temp_data = DataProtect::DataProtectAut(data_[iter], reinterpret_cast<uintptr_t>(&data_));
            res.push_back(char((temp_data & mask) >> SHIFT24));
            temp_data <<= SHIFT8;
            res.push_back(char((temp_data & mask) >> SHIFT24));
            temp_data <<= SHIFT8;
            res.push_back(char((temp_data & mask) >> SHIFT24));
            temp_data <<= SHIFT8;
            res.push_back(char((temp_data & mask) >> SHIFT24));
        }

        // delete back empty
        while (res.size() > origin_length_) {
            res.pop_back();
        }

        return res;
    }

    bool CompareStringWithPacedString(std::string_view str_data)
    {
        uint32_t len = str_data.length();
        constexpr uint32_t step = 4;
        if (len != origin_length_) {
            return false;
        }
        auto str = reinterpret_cast<const uint8_t *>(str_data.data());
        
        auto data_ptr = data_.begin();
        uint32_t left = 0;
        while (left < len) {
            uint32_t right = left + step;
            uintptr_t temp_data = 0;

            while (left < right) {
                temp_data <<= SHIFT8;
                temp_data += (left < len ? str[left] : 0);
                ++left;
            }
            auto res = DataProtect::DataProtectPac(temp_data, reinterpret_cast<uintptr_t>(&data_));
            if (res != *data_ptr) {
                return false;
            }
            ++data_ptr;
        }
        return true;
    }

    void Clear()
    {
        std::vector<uintptr_t>().swap(data_);
        origin_length_ = 0;
    }

    // PacDataSize = ceil(StrLength / 4)
    uint32_t PacDataSize()
    {
        return data_.size();
    }

    // Original String Length Before Pac
    uint32_t StrLength()
    {
        return origin_length_;
    }

private:

    void AppendWithoutCheckBack(std::string_view str_data)
    {
        if (str_data.empty()) {
            return;
        }
        auto str = reinterpret_cast<const uint8_t *>(str_data.data());
        uint32_t len = str_data.length();
        
        constexpr uint32_t step = 4;
        uint32_t left = 0;
        // uint32 = char << 24 | char << 16 | char << 8 | char
        // compress 4 char => 1 uint32 => PAC(uint32) => uintptr_t
        while (left < len) {
            uint32_t right = left + step;
            uintptr_t temp_data = 0;

            while (left < right) {
                temp_data <<= SHIFT8;
                temp_data += (left < len ? str[left] : 0);
                ++left;
            }
            data_.push_back(DataProtect::DataProtectPac(temp_data, reinterpret_cast<uintptr_t>(&data_)));
        }
        origin_length_ += str_data.length();
    }

    std::vector<uintptr_t>data_;
    uint32_t origin_length_;
};

}
#endif  // LIBPANDAFILE_DATA_PROTECT_H
