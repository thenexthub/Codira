/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_RUNTIME_REGEXP_EXECUTOR_H
#define PANDA_RUNTIME_REGEXP_EXECUTOR_H

#include "runtime/regexp/ecmascript/regexp_parser.h"

namespace ark {

template <class T>
struct RegExpMatchResult {
    uint32_t endIndex = 0;
    uint32_t index = 0;
    // NOTE(kirill-mitkin): Change ecmascript API to PandaVector<T>
    // first value is true if result is undefined
    PandaVector<std::pair<bool, T>> captures;
    PandaVector<std::pair<uint32_t, uint32_t>> indices;
    bool isSuccess = false;
    bool isWide = false;
};

class RegExpExecutor {
public:
    struct CaptureState {
        const uint8_t *captureStart;
        const uint8_t *captureEnd;
    };

    enum StateType : uint8_t {
        STATE_SPLIT = 0,
        STATE_MATCH_AHEAD,
        STATE_NEGATIVE_MATCH_AHEAD,
    };

    struct RegExpState {
        StateType type = STATE_SPLIT;
        uint32_t currentPc = 0;
        uint32_t currentStack = 0;
        const uint8_t *currentPtr = nullptr;
        __extension__ CaptureState *captureResultList[0];  // NOLINT(modernize-avoid-c-arrays)
    };

    explicit RegExpExecutor() = default;

    ~RegExpExecutor()
    {
        auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
        allocator->DeleteArray(stack_);
        allocator->DeleteArray(captureResultList_);
        allocator->DeleteArray(stateStack_);
    }

    NO_COPY_SEMANTIC(RegExpExecutor);
    NO_MOVE_SEMANTIC(RegExpExecutor);

    PANDA_PUBLIC_API bool Execute(const uint8_t *input, uint32_t lastIndex, uint32_t length, uint8_t *buf,
                                  bool isWideChar = false);

    bool ExecuteInternal(const DynChunk &byteCode, uint32_t pcEnd);
    // CC-OFFNXT(G.FUD.06) perf critical
    inline bool HandleFirstSplit()
    {
        if (GetCurrentPC() == RegExpParser::OP_START_OFFSET && stateStackLen_ == 0 &&
            (flags_ & RegExpParser::FLAG_STICKY) == 0) {
            if (IsEOF()) {
                if (MatchFailed()) {
                    return false;
                }
            } else {
                AdvanceCurrentPtr();
                PushRegExpState(STATE_SPLIT, RegExpParser::OP_START_OFFSET);
            }
        }
        return true;
    }

    inline bool HandleOpAll(uint8_t opCode)
    {
        if (IsEOF()) {
            return !MatchFailed();
        }
        uint32_t currentChar = GetCurrentChar();
        if ((opCode == RegExpOpCode::OP_DOTS) && IsTerminator(currentChar)) {
            return !MatchFailed();
        }
        Advance(opCode);
        return true;
    }

    // CC-OFFNXT(G.FUD.06) perf critical
    inline bool HandleOpChar(const DynChunk &byteCode, uint8_t opCode)
    {
        uint32_t expectedChar;
        if (opCode == RegExpOpCode::OP_CHAR32) {
            expectedChar = byteCode.GetU32(GetCurrentPC() + 1);
        } else {
            expectedChar = byteCode.GetU16(GetCurrentPC() + 1);
        }
        if (IsEOF()) {
            return !MatchFailed();
        }
        uint32_t currentChar = GetCurrentChar();
        if (IsIgnoreCase()) {
            currentChar = static_cast<uint32_t>(RegExpParser::Canonicalize(currentChar, IsUtf16()));
        }
        if (currentChar == expectedChar) {
            Advance(opCode);
        } else {
            if (MatchFailed()) {
                return false;
            }
        }
        return true;
    }

    // CC-OFFNXT(G.FUD.06) perf critical
    inline bool HandleOpWordBoundary(uint8_t opCode)
    {
        if (IsEOF()) {
            if (opCode == RegExpOpCode::OP_WORD_BOUNDARY) {
                Advance(opCode);
            } else {
                if (MatchFailed()) {
                    return false;
                }
            }
            return true;
        }
        bool preIsWord = false;
        if (GetCurrentPtr() != input_) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            preIsWord = IsWordChar(PeekPrevChar(currentPtr_, input_));
        }
        bool currentIsWord = IsWordChar(PeekChar(currentPtr_, inputEnd_));
        if (((opCode == RegExpOpCode::OP_WORD_BOUNDARY) &&
             ((!preIsWord && currentIsWord) || (preIsWord && !currentIsWord))) ||
            ((opCode == RegExpOpCode::OP_NOT_WORD_BOUNDARY) &&
             ((preIsWord && currentIsWord) || (!preIsWord && !currentIsWord)))) {
            Advance(opCode);
        } else {
            if (MatchFailed()) {
                return false;
            }
        }
        return true;
    }

    inline bool HandleOpLineStart(uint8_t opCode)
    {
        if (IsEOF()) {
            return !MatchFailed();
        }
        if ((GetCurrentPtr() == input_) ||
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            ((flags_ & RegExpParser::FLAG_MULTILINE) != 0 && PeekPrevChar(currentPtr_, input_) == '\n')) {
            Advance(opCode);
        } else if (MatchFailed()) {
            return false;
        }
        return true;
    }

    inline bool HandleOpLineEnd(uint8_t opCode)
    {
        if (IsEOF() ||
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            ((flags_ & RegExpParser::FLAG_MULTILINE) != 0 && PeekChar(currentPtr_, inputEnd_) == '\n')) {
            Advance(opCode);
        } else {
            if (MatchFailed()) {
                return false;
            }
        }
        return true;
    }

    inline void HandleOpSaveStart(const DynChunk &byteCode, uint8_t opCode)
    {
        uint32_t captureIndex = byteCode.GetU8(GetCurrentPC() + 1);
        ASSERT(captureIndex < nCapture_);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        CaptureState *captureState = &captureResultList_[captureIndex];
        captureState->captureStart = GetCurrentPtr();
        Advance(opCode);
    }

    inline void HandleOpSaveEnd(const DynChunk &byteCode, uint8_t opCode)
    {
        uint32_t captureIndex = byteCode.GetU8(GetCurrentPC() + 1);
        ASSERT(captureIndex < nCapture_);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        CaptureState *captureState = &captureResultList_[captureIndex];
        captureState->captureEnd = GetCurrentPtr();
        Advance(opCode);
    }

    inline void HandleOpSaveReset(const DynChunk &byteCode, uint8_t opCode)
    {
        uint32_t catpureStartIndex = byteCode.GetU8(GetCurrentPC() + SAVE_RESET_START);
        uint32_t catpureEndIndex = byteCode.GetU8(GetCurrentPC() + SAVE_RESET_END);
        for (uint32_t i = catpureStartIndex; i <= catpureEndIndex; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            CaptureState *captureState = &captureResultList_[i];
            captureState->captureStart = nullptr;
            captureState->captureEnd = nullptr;
        }
        Advance(opCode);
    }

    inline void HandleOpMatch(const DynChunk &byteCode, uint8_t opCode)
    {
        auto type = static_cast<StateType>(opCode - RegExpOpCode::OP_SPLIT_NEXT);
        ASSERT(type == STATE_SPLIT || type == STATE_MATCH_AHEAD || type == STATE_NEGATIVE_MATCH_AHEAD);
        uint32_t offset = byteCode.GetU32(GetCurrentPC() + 1);
        Advance(opCode);
        uint32_t splitPc = GetCurrentPC() + offset;
        PushRegExpState(type, splitPc);
    }

    inline void HandleOpSplitFirst(const DynChunk &byteCode, uint8_t opCode)
    {
        uint32_t offset = byteCode.GetU32(GetCurrentPC() + 1);
        Advance(opCode);
        PushRegExpState(STATE_SPLIT, GetCurrentPC());
        AdvanceOffset(offset);
    }

    inline bool HandleOpPrev(uint8_t opCode)
    {
        if (GetCurrentPtr() == input_) {
            if (MatchFailed()) {
                return false;
            }
        } else {
            PrevPtr(&currentPtr_, input_);
            Advance(opCode);
        }
        return true;
    }

    // CC-OFFNXT(G.FUD.06) perf critical
    inline void HandleOpLoop(const DynChunk &byteCode, uint8_t opCode)
    {
        uint32_t quantifyMin = byteCode.GetU32(GetCurrentPC() + LOOP_MIN_OFFSET);
        uint32_t quantifyMax = byteCode.GetU32(GetCurrentPC() + LOOP_MAX_OFFSET);
        uint32_t pcOffset = byteCode.GetU32(GetCurrentPC() + LOOP_PC_OFFSET);
        Advance(opCode);
        uint32_t loopPcEnd = GetCurrentPC();
        uint32_t loopPcStart = GetCurrentPC() + pcOffset;
        bool isGreedy = opCode == RegExpOpCode::OP_LOOP_GREEDY;
        uint32_t loopMax = isGreedy ? quantifyMax : quantifyMin;

        uint32_t loopCount = PeekStack();
        SetStackValue(++loopCount);
        if (loopCount < loopMax) {
            // greedy failed, goto next
            if (loopCount >= quantifyMin) {
                PushRegExpState(STATE_SPLIT, loopPcEnd);
            }
            // Goto loop start
            SetCurrentPC(loopPcStart);
        } else {
            if (!isGreedy && (loopCount < quantifyMax)) {
                PushRegExpState(STATE_SPLIT, loopPcStart);
            }
        }
    }

    // CC-OFFNXT(G.FUD.06) perf critical
    inline bool HandleOpRange32(const DynChunk &byteCode)
    {
        if (IsEOF()) {
            return !MatchFailed();
        }
        uint32_t currentChar = GetCurrentChar();
        if (IsIgnoreCase()) {
            currentChar = static_cast<uint32_t>(RegExpParser::Canonicalize(currentChar, IsUtf16()));
        }
        uint16_t rangeCount = byteCode.GetU16(GetCurrentPC() + 1);
        bool isFound = false;
        int32_t idxMin = 0;
        int32_t idxMax = static_cast<int32_t>(rangeCount) - 1;
        int32_t idx = 0;
        uint32_t low = 0;
        uint32_t high = byteCode.GetU32(GetCurrentPC() + RANGE32_HEAD_OFFSET + idxMax * RANGE32_MAX_OFFSET +
                                        RANGE32_MAX_HALF_OFFSET);
        if (currentChar <= high) {
            while (idxMin <= idxMax) {
                idx = (idxMin + idxMax) / RANGE32_OFFSET;
                low = byteCode.GetU32(GetCurrentPC() + RANGE32_HEAD_OFFSET +
                                      static_cast<uint32_t>(idx) * RANGE32_MAX_OFFSET);
                high = byteCode.GetU32(GetCurrentPC() + RANGE32_HEAD_OFFSET +
                                       static_cast<uint32_t>(idx) * RANGE32_MAX_OFFSET + RANGE32_MAX_HALF_OFFSET);
                if (currentChar < low) {
                    idxMax = idx - 1;
                } else if (currentChar > high) {
                    idxMin = idx + 1;
                } else {
                    isFound = true;
                    break;
                }
            }
        }
        if (isFound) {
            AdvanceOffset(rangeCount * RANGE32_MAX_OFFSET + RANGE32_HEAD_OFFSET);
        } else {
            if (MatchFailed()) {
                return false;
            }
        }
        return true;
    }

    // CC-OFFNXT(G.FUD.06) perf critical
    inline bool HandleOpRange(const DynChunk &byteCode)
    {
        if (IsEOF()) {
            return !MatchFailed();
        }
        uint32_t currentChar = GetCurrentChar();
        if (IsIgnoreCase()) {
            currentChar = static_cast<uint32_t>(RegExpParser::Canonicalize(currentChar, IsUtf16()));
        }
        uint16_t rangeCount = byteCode.GetU16(GetCurrentPC() + 1);
        bool isFound = false;
        int32_t idxMin = 0;
        int32_t idxMax = rangeCount - 1;
        int32_t idx = 0;
        uint32_t low = 0;
        uint32_t high =
            byteCode.GetU16(GetCurrentPC() + RANGE32_HEAD_OFFSET + idxMax * RANGE32_MAX_HALF_OFFSET + RANGE32_OFFSET);
        if (currentChar <= high) {
            while (idxMin <= idxMax) {
                idx = (idxMin + idxMax) / RANGE32_OFFSET;
                low = byteCode.GetU16(GetCurrentPC() + RANGE32_HEAD_OFFSET +
                                      static_cast<uint32_t>(idx) * RANGE32_MAX_HALF_OFFSET);
                high = byteCode.GetU16(GetCurrentPC() + RANGE32_HEAD_OFFSET +
                                       static_cast<uint32_t>(idx) * RANGE32_MAX_HALF_OFFSET + RANGE32_OFFSET);
                if (currentChar < low) {
                    idxMax = idx - 1;
                } else if (currentChar > high) {
                    idxMin = idx + 1;
                } else {
                    isFound = true;
                    break;
                }
            }
        }
        if (isFound) {
            AdvanceOffset(rangeCount * RANGE32_MAX_HALF_OFFSET + RANGE32_HEAD_OFFSET);
        } else {
            if (MatchFailed()) {
                return false;
            }
        }
        return true;
    }

    bool HandleOpBackReference(const DynChunk &byteCode, uint8_t opCode);

    bool HandleOpBackReferenceMatch(const uint8_t *captureStart, const uint8_t *captureEnd, uint8_t opCode);

    bool HandleOpBackwardBackReferenceMatch(const uint8_t *captureStart, const uint8_t *captureEnd, uint8_t opCode);

    inline void Advance(uint8_t opCode, uint32_t offset = 0)
    {
        currentPc_ += offset + static_cast<uint32_t>(RegExpOpCode::GetRegExpOpCode(opCode)->GetSize());
    }

    inline void AdvanceOffset(uint32_t offset)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        currentPc_ += offset;
    }

    inline uint32_t GetCurrentChar()
    {
        return GetChar(&currentPtr_, inputEnd_);
    }

    inline void AdvanceCurrentPtr()
    {
        AdvancePtr(&currentPtr_, inputEnd_);
    }

    uint32_t GetChar(const uint8_t **pp, const uint8_t *end) const;

    uint32_t PeekChar(const uint8_t *p, const uint8_t *end) const;

    void AdvancePtr(const uint8_t **pp, const uint8_t *end) const;

    uint32_t PeekPrevChar(const uint8_t *p, const uint8_t *start) const;

    uint32_t GetPrevChar(const uint8_t **pp, const uint8_t *start) const;

    void PrevPtr(const uint8_t **pp, const uint8_t *start) const;

    bool MatchFailed(bool isMatched = false);

    void SetCurrentPC(uint32_t pc)
    {
        currentPc_ = pc;
    }

    void SetCurrentPtr(const uint8_t *ptr)
    {
        currentPtr_ = ptr;
    }

    bool IsEOF() const
    {
        return currentPtr_ >= inputEnd_;
    }

    uint32_t GetCurrentPC() const
    {
        return currentPc_;
    }

    void PushStack(uintptr_t val)
    {
        ASSERT(currentStack_ < nStack_);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        stack_[currentStack_++] = val;
    }

    void SetStackValue(uintptr_t val) const
    {
        ASSERT(currentStack_ >= 1);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        stack_[currentStack_ - 1] = val;
    }

    uintptr_t PopStack()
    {
        ASSERT(currentStack_ >= 1);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return stack_[--currentStack_];
    }

    uintptr_t PeekStack() const
    {
        ASSERT(currentStack_ >= 1);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return stack_[currentStack_ - 1];
    }

    const uint8_t *GetCurrentPtr() const
    {
        return currentPtr_;
    }

    CaptureState *GetCaptureResultList() const
    {
        return captureResultList_;
    }

    uint32_t GetCaptureCount() const
    {
        return nCapture_;
    }

    void DumpResult(std::ostream &out) const;

    void PushRegExpState(StateType type, uint32_t pc);

    RegExpState *PopRegExpState(bool copyCaptrue = true);

    void DropRegExpState()
    {
        stateStackLen_--;
    }

    RegExpState *PeekRegExpState() const
    {
        ASSERT(stateStackLen_ >= 1);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return reinterpret_cast<RegExpState *>(stateStack_ + (stateStackLen_ - 1) * stateSize_);
    }

    void ReAllocStack(uint32_t stackLen);

    inline bool IsWordChar(uint8_t value) const
    {
        return ((value >= '0' && value <= '9') || (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z') ||
                (value == '_'));
    }

    inline bool IsTerminator(uint32_t value) const
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        return (value == '\n' || value == '\r' || value == 0x2028 || value == 0x2029);
    }

    inline bool IsIgnoreCase() const
    {
        return (flags_ & RegExpParser::FLAG_IGNORECASE) != 0;
    }

    inline bool IsUtf16() const
    {
        return (flags_ & RegExpParser::FLAG_UTF16) != 0;
    }

    inline bool IsWideChar() const
    {
        return isWideChar_;
    }

    inline uint8_t *GetInputPtr() const
    {
        return input_;
    }

    static constexpr size_t CHAR_SIZE = 1;
    static constexpr size_t WIDE_CHAR_SIZE = 2;
    static constexpr size_t SAVE_RESET_START = 1;
    static constexpr size_t SAVE_RESET_END = 2;
    static constexpr size_t LOOP_MIN_OFFSET = 5;
    static constexpr size_t LOOP_MAX_OFFSET = 9;
    static constexpr size_t LOOP_PC_OFFSET = 1;
    static constexpr size_t RANGE32_HEAD_OFFSET = 3;
    static constexpr size_t RANGE32_MAX_HALF_OFFSET = 4;
    static constexpr size_t RANGE32_MAX_OFFSET = 8;
    static constexpr size_t RANGE32_OFFSET = 2;
    static constexpr uint32_t STACK_MULTIPLIER = 2;
    static constexpr uint32_t MIN_STACK_SIZE = 8;

private:
    uint8_t *input_ = nullptr;
    uint8_t *inputEnd_ = nullptr;
    bool isWideChar_ = false;

    uint32_t currentPc_ = 0;
    const uint8_t *currentPtr_ = nullptr;
    CaptureState *captureResultList_ = nullptr;
    uintptr_t *stack_ = nullptr;
    uint32_t currentStack_ = 0;

    uint32_t nCapture_ = 0;
    uint32_t nStack_ = 0;

    uint32_t flags_ = 0;
    uint32_t stateStackLen_ = 0;
    uint32_t stateStackSize_ = 0;
    uint32_t stateSize_ = 0;
    uint8_t *stateStack_ = nullptr;
};
}  // namespace ark
#endif  // core_REGEXP_REGEXP_EXECUTOR_H
