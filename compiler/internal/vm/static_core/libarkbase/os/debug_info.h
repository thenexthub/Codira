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
#ifndef PANDA_DEBUG_INFO_H_
#define PANDA_DEBUG_INFO_H_

#include <set>
#include <list>
#include <string>

#include <libdwarf/libdwarf.h>

#include "libarkbase/macros.h"
#include "libarkbase/utils/span.h"

namespace ark {

class DebugInfo {
public:
    enum ErrorCode { SUCCESS, NO_DEBUG_INFO, ERROR };

    explicit DebugInfo() = default;
    DebugInfo(DebugInfo &&info);

    ~DebugInfo()
    {
        Destroy();
    }

    DebugInfo &operator=(DebugInfo &&info);

    ErrorCode ReadFromFile(const char *filename);

    /*
     * Find location (name, source file, line) of the specified pc in source code
     */
    bool GetSrcLocation(uintptr_t pc, std::string *function, std::string *srcFile, uint32_t *line);

    void Destroy();

    NO_COPY_SEMANTIC(DebugInfo);

private:
    /**
     * Cache entry for a compilation unit (object file).
     * It contains the pointer to the corresponding DIE (Debug Information Entity),
     * offset of the DIE in .debug_info, decoded line numbers for the compilation unit
     * and function cache.
     */
    class CompUnit {
    public:
        CompUnit(Dwarf_Die cuDie, Dwarf_Debug dbg) : dbg_(dbg), cuDie_(cuDie) {}

        CompUnit(CompUnit &&e) : dbg_(e.dbg_), cuDie_(e.cuDie_), lineCtx_(e.lineCtx_)
        {
            e.cuDie_ = nullptr;
            e.lineCtx_ = nullptr;
        }

        ~CompUnit();

        CompUnit &operator=(CompUnit &&e)
        {
            dbg_ = e.dbg_;
            cuDie_ = e.cuDie_;
            e.cuDie_ = nullptr;
            lineCtx_ = e.lineCtx_;
            e.lineCtx_ = nullptr;
            return *this;
        }

        Dwarf_Die GetDie() const
        {
            return cuDie_;
        }

        Dwarf_Line_Context GetLineContext();

        NO_COPY_SEMANTIC(CompUnit);

    private:
        Dwarf_Debug dbg_;
        Dwarf_Die cuDie_;
        Dwarf_Line_Context lineCtx_ {nullptr};
    };

    class Range {
    public:
        Range(Dwarf_Addr lowPc, Dwarf_Addr highPc, CompUnit *cu = nullptr,
              const std::string &function = std::string())  // NOLINT(modernize-pass-by-value)
            : lowPc_(lowPc), highPc_(highPc), cu_(cu), function_(function)
        {
        }

        Dwarf_Addr GetLowPc() const
        {
            return lowPc_;
        }

        Dwarf_Addr GetHighPc() const
        {
            return highPc_;
        }

        bool Contain(Dwarf_Addr addr) const
        {
            return lowPc_ <= addr && addr < highPc_;
        }

        bool Contain(const Range &r) const
        {
            return lowPc_ <= r.lowPc_ && r.highPc_ <= highPc_;
        }

        CompUnit *GetCu() const
        {
            return cu_;
        }

        std::string GetFunction() const
        {
            return function_;
        }

        void SetFunction(const std::string &function)
        {
            this->function_ = function;
        }

        bool operator<(const Range &r) const
        {
            return highPc_ < r.highPc_;
        }

        bool operator==(const Range &r) const
        {
            return lowPc_ == r.lowPc_ && highPc_ == r.highPc_;
        }

    private:
        Dwarf_Addr lowPc_;
        Dwarf_Addr highPc_;
        CompUnit *cu_ = nullptr;
        std::string function_;
    };

private:
    bool FindCompUnitByPc(uintptr_t pc, Dwarf_Die *cuDie);
    void TraverseChildren(CompUnit *cu, Dwarf_Die die);
    void TraverseSiblings(CompUnit *cu, Dwarf_Die die);
    void GetFunctionName(Dwarf_Die die, std::string *function);
    void AddFunction(CompUnit *cu, Dwarf_Addr lowPc, Dwarf_Addr highPc, const std::string &function);
    bool GetSrcFileAndLine(uintptr_t pc, Dwarf_Line_Context lineCtx, std::string *outSrcFile, uint32_t *outLine);
    Dwarf_Line GetLastLineWithPc(Dwarf_Addr pc, Span<Dwarf_Line>::ConstIterator it,
                                 Span<Dwarf_Line>::ConstIterator end);
    void GetSrcFileAndLine(Dwarf_Line line, std::string *outSrcFile, uint32_t *outLine);
    bool PcMatches(uintptr_t pc, Dwarf_Die die);
    bool GetDieRangeForPc(uintptr_t pc, Dwarf_Die die, Dwarf_Addr *outLowPc, Dwarf_Addr *outHighPc);
    bool FindRangeForPc(uintptr_t pc, const Span<Dwarf_Ranges> &ranges, Dwarf_Addr baseAddr, Dwarf_Addr *outLowPc,
                        Dwarf_Addr *outHighPc);

private:
    static constexpr int INVALID_FD = -1;

    int fd_ {INVALID_FD};
    Dwarf_Debug dbg_ {nullptr};
    Dwarf_Arange *aranges_ {nullptr};
    Dwarf_Signed arangeCount_ {0};
    std::list<CompUnit> cuList_;
    std::set<Range> ranges_;
};

}  // namespace ark
#endif  // PANDA_DEBUG_INFO_H_
