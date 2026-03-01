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

#ifndef LIBABCKIT_SRC_WRAPPERS_PANDASM_WRAPPER_H
#define LIBABCKIT_SRC_WRAPPERS_PANDASM_WRAPPER_H

#include <cstdint>
#include <unordered_set>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace panda::panda_file {
enum class SourceLang : uint8_t;
}  // namespace panda::panda_file

namespace libabckit {

struct LiteralArrayWrapper;
struct InsWrapper;

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct LiteralArrayWrapper {
    enum class LiteralTagWrapper : uint8_t {
        TAGVALUE = 0x00,
        BOOL = 0x01,
        INTEGER = 0x02,
        FLOAT = 0x03,
        DOUBLE = 0x04,
        STRING = 0x05,
        METHOD = 0x06,
        GENERATORMETHOD = 0x07,
        ACCESSOR = 0x08,
        METHODAFFILIATE = 0x09,
        ARRAY_U1 = 0x0a,
        ARRAY_U8 = 0x0b,
        ARRAY_I8 = 0x0c,
        ARRAY_U16 = 0x0d,
        ARRAY_I16 = 0x0e,
        ARRAY_U32 = 0x0f,
        ARRAY_I32 = 0x10,
        ARRAY_U64 = 0x11,
        ARRAY_I64 = 0x12,
        ARRAY_F32 = 0x13,
        ARRAY_F64 = 0x14,
        ARRAY_STRING = 0x15,
        ASYNCGENERATORMETHOD = 0x16,
        LITERALBUFFERINDEX = 0x17,
        LITERALARRAY = 0x18,
        BUILTINTYPEINDEX = 0x19,
        GETTER = 0x1a,
        SETTER = 0x1b,
        NULLVALUE = 0xff
    };

    struct LiteralWrapper {
        LiteralTagWrapper tag;
        std::variant<bool, uint8_t, uint16_t, uint32_t, uint64_t, float, double, std::string> value;
    };

    std::vector<LiteralArrayWrapper::LiteralWrapper> literals;

    explicit LiteralArrayWrapper(std::vector<LiteralArrayWrapper::LiteralWrapper> lits) : literals(std::move(lits)) {}
    explicit LiteralArrayWrapper() = default;
};

struct FunctionWrapper {
    struct CatchBlockWrapper {
        std::string wholeLine;
        std::string exceptionRecord;
        std::string tryBeginLabel;
        std::string tryEndLabel;
        std::string catchBeginLabel;
        std::string catchEndLabel;

        CatchBlockWrapper() = default;
        CatchBlockWrapper(std::string ln, std::string excr, std::string tbl, std::string tel, std::string cbl,
                          std::string cel)
            : wholeLine(std::move(ln)),
              exceptionRecord(std::move(excr)),
              tryBeginLabel(std::move(tbl)),
              tryEndLabel(std::move(tel)),
              catchBeginLabel(std::move(cbl)),
              catchEndLabel(std::move(cel))
        {
        }
    };
    explicit FunctionWrapper(void *func) : impl(func) {}

    void Fill();
    void Update();

    void *impl = nullptr;
    std::string name;
    int64_t valueOfFirstParam = -1;
    size_t regsNum = 0;
    size_t slotsNum = 0;
    std::vector<InsWrapper> ins = {};
    std::vector<CatchBlockWrapper> catchBlocks = {};
};

struct InsWrapper {
    static std::string opcodeInvalid_;

    struct DebugIns {
        size_t lineNumber = 0;
        uint32_t columnNumber = 0;
        std::string wholeLine;  // NOTE(mbolshov): redundant given file and line_number
        size_t boundLeft = 0;
        size_t boundRight = 0;

        void SetLineNumber(size_t ln)
        {
            lineNumber = ln;
        }

        void SetColumnNumber(size_t cn)
        {
            columnNumber = cn;
        }

        DebugIns() = default;
        DebugIns(size_t ln, std::string &fc, size_t bl, size_t br)
            : lineNumber(ln), wholeLine(fc), boundLeft(bl), boundRight(br)
        {
        }
    };
    using IType = std::variant<int64_t, double>;

    constexpr static uint16_t ACCUMULATOR = -1;
    constexpr static size_t MAX_CALL_SHORT_ARGS = 2;
    constexpr static size_t MAX_CALL_ARGS = 4;
    constexpr static uint16_t MAX_NON_RANGE_CALL_REG = 15;
    constexpr static uint16_t MAX_RANGE_CALL_START_REG = (1U << 12U) * 2 - 1U;

    std::string opcode = opcodeInvalid_;
    std::vector<uint16_t> regs;
    std::vector<std::string> ids;
    std::vector<IType> imms;
    std::string label;
    bool setLabel = false;
    DebugIns insDebug;

    InsWrapper() = default;
    InsWrapper(std::string opcodeArg, std::vector<uint16_t> regsArg, std::vector<std::string> idsArg,
               std::vector<IType> immsArg, std::string labelArg, bool setLabelArg, DebugIns insDebugArg)
        : opcode(std::move(opcodeArg)),
          regs(std::move(regsArg)),
          ids(std::move(idsArg)),
          imms(std::move(immsArg)),
          label(std::move(labelArg)),
          setLabel(setLabelArg),
          insDebug(std::move(insDebugArg))
    {
    }
// NOLINTNEXTLINE(readability-duplicate-include)
#include "arkcompiler/runtime_core/libabckit/src/wrappers/generated/ins_create_wrapper_api.inc"
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

class PandasmWrapper {
public:
    using ArgTypes = std::variant<uint8_t, int64_t, double, uint64_t, uint32_t, std::string>;

    static FunctionWrapper *CreateWrappedFunction(panda::panda_file::SourceLang lang);

    static FunctionWrapper *GetWrappedFunction(void *function)
    {
        auto wrFunc = new FunctionWrapper(function);
        wrFunc->Fill();
        return wrFunc;
    }

    static std::map<std::string, void *> GetUnwrappedLiteralArrayTable(void *prog);
    static void *GetPandasmFunction(FunctionWrapper *wrFunc)
    {
        wrFunc->Update();
        auto function = wrFunc->impl;
        delete wrFunc;
        return function;
    }
// NOLINTNEXTLINE(readability-duplicate-include)
#include "arkcompiler/runtime_core/libabckit/src/wrappers/generated/ins_create_wrapper_api.inc"
};

// std::string DeMangleName(const std::string &name);

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_WRAPPERS_PANDASM_WRAPPER_H
