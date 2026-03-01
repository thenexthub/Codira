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

#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <libdwarf/dwarf.h>

#include "debug_info.h"
#include "libarkbase/utils/logger.h"

namespace ark {

class DwarfGuard {
public:
    DwarfGuard(Dwarf_Debug dbg, void *mem, Dwarf_Unsigned tag) : dbg_(dbg), mem_(mem), tag_(tag) {}

    void Release()
    {
        mem_ = nullptr;
    }

    void Reset(void *newMem)
    {
        if (mem_ != newMem && mem_ != nullptr) {
            dwarf_dealloc(dbg_, mem_, tag_);
            mem_ = newMem;
        }
    }

    ~DwarfGuard()
    {
        Reset(nullptr);
    }

    NO_MOVE_SEMANTIC(DwarfGuard);
    NO_COPY_SEMANTIC(DwarfGuard);

private:
    Dwarf_Debug dbg_;
    void *mem_;
    Dwarf_Unsigned tag_;
};

template <class F>
class AtReturn {
public:
    explicit AtReturn(F func) : func_(func) {}

    ~AtReturn()
    {
        func_();
    }

    NO_MOVE_SEMANTIC(AtReturn);
    NO_COPY_SEMANTIC(AtReturn);

private:
    F func_;
};

static void FreeAranges(Dwarf_Debug dbg, Dwarf_Arange *aranges, Dwarf_Signed count)
{
    for (Dwarf_Signed i = 0; i < count; ++i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        dwarf_dealloc(dbg, aranges[i], DW_DLA_ARANGE);
    }
    dwarf_dealloc(dbg, aranges, DW_DLA_LIST);
}

static void SkipCuHeaders(Dwarf_Debug dbg)
{
    Dwarf_Unsigned cuHeaderIdx;
    Dwarf_Half cuType;
    while (dwarf_next_cu_header_d(dbg, static_cast<Dwarf_Bool>(true), nullptr, nullptr, nullptr, nullptr, nullptr,
                                  nullptr, nullptr, nullptr, &cuHeaderIdx, &cuType, nullptr) != DW_DLV_NO_ENTRY) {
    }
}

static void DwarfErrorHandler(Dwarf_Error err, [[maybe_unused]] Dwarf_Ptr errarg)
{
    LOG(ERROR, RUNTIME) << "libdwarf error: " << dwarf_errmsg(err);
}

static bool GetDieRange(Dwarf_Die die, Dwarf_Addr *outLowPc, Dwarf_Addr *outHighPc)
{
    Dwarf_Addr lowPc = DW_DLV_BADADDR;
    Dwarf_Addr highPc = 0;
    Dwarf_Half form = 0;
    Dwarf_Form_Class formclass;

    if (dwarf_lowpc(die, &lowPc, nullptr) != DW_DLV_OK ||
        dwarf_highpc_b(die, &highPc, &form, &formclass, nullptr) != DW_DLV_OK) {
        return false;
    }
    if (formclass == DW_FORM_CLASS_CONSTANT) {
        highPc += lowPc;
    }
    *outLowPc = lowPc;
    *outHighPc = highPc;
    return true;
}

template <class F>
bool IterateDieRanges([[maybe_unused]] F func, [[maybe_unused]] Dwarf_Attribute attr, [[maybe_unused]] Dwarf_Half form,
                      [[maybe_unused]] Dwarf_Unsigned offset)
{
#ifdef HAVE_DWARF_RNGLISTS_API
    Dwarf_Rnglists_Head rnglhead = nullptr;
    Dwarf_Unsigned countRnglistsEntries = 0;
    if (dwarf_rnglists_get_rle_head(attr, form, offset, &rnglhead, &countRnglistsEntries, nullptr, nullptr) ==
        DW_DLV_OK) {
        AtReturn r([rnglhead]() { dwarf_dealloc_rnglists_head(rnglhead); });
        for (Dwarf_Unsigned i = 0; i < countRnglistsEntries; i++) {
            unsigned rleCode = 0;
            Dwarf_Bool noDebugAddrAvailable = false;
            Dwarf_Unsigned cooked1 = 0;
            Dwarf_Unsigned cooked2 = 0;

            if (dwarf_get_rnglists_entry_fields_a(rnglhead, i, nullptr, &rleCode, nullptr, nullptr,
                                                  &noDebugAddrAvailable, &cooked1, &cooked2, nullptr) != DW_DLV_OK) {
                return false;
            }

            if (rleCode == DW_RLE_base_addressx || rleCode == DW_RLE_base_address || noDebugAddrAvailable) {
                continue;
            }

            if (rleCode == DW_RLE_end_of_list) {
                break;
            }

            if (func(cooked1, cooked2)) {
                return true;
            }
        }
    }
#endif  // HAVE_DWARF_RNGLISTS_API

    return true;
}

template <class F>
bool IterateDieRanges(F func, const Span<Dwarf_Ranges> &ranges, Dwarf_Addr baseAddr)
{
    for (const Dwarf_Ranges &range : ranges) {
        if (range.dwr_type == DW_RANGES_ENTRY) {
            Dwarf_Addr rngLowPc = baseAddr + range.dwr_addr1;
            Dwarf_Addr rngHighPc = baseAddr + range.dwr_addr2;
            if (func(rngLowPc, rngHighPc)) {
                return true;
            }
        } else if (range.dwr_type == DW_RANGES_ADDRESS_SELECTION) {
            baseAddr = range.dwr_addr2;
        } else {
            break;
        }
    }

    return false;
}

template <class F>
bool IterateDieRanges(Dwarf_Debug dbg, Dwarf_Die die, F func, Dwarf_Attribute attr, Dwarf_Addr lowPc)
{
    Dwarf_Half form = 0;
    if (dwarf_whatform(attr, &form, nullptr) != DW_DLV_OK) {
        return false;
    }

    Dwarf_Half offsetSize = 0;
    Dwarf_Half dwversion = 0;
    if (dwarf_get_version_of_die(die, &dwversion, &offsetSize) != DW_DLV_OK) {
        return false;
    }

    DwarfGuard g(dbg, attr, DW_DLA_ATTR);
    Dwarf_Unsigned offset = 0;
    Dwarf_Addr baseAddr = 0;
    if (lowPc != DW_DLV_BADADDR) {
        baseAddr = lowPc;
    }
    Dwarf_Signed count = 0;
    Dwarf_Ranges *buf = nullptr;

    if (form == DW_FORM_rnglistx) {
        if (dwarf_formudata(attr, &offset, nullptr) != DW_DLV_OK) {
            return false;
        }
    } else {
        if (dwarf_global_formref(attr, &offset, nullptr) != DW_DLV_OK) {
            return false;
        }
    }

    if (dwversion < 5U) {
        if (dwarf_get_ranges_a(dbg, offset, die, &buf, &count, nullptr, nullptr) == DW_DLV_OK) {
            AtReturn r([dbg, buf, count]() { dwarf_ranges_dealloc(dbg, buf, count); });
            Span<Dwarf_Ranges> ranges(buf, count);
            return IterateDieRanges(func, ranges, baseAddr);
        }
    }

    return IterateDieRanges(func, attr, form, offset);
}

template <class F>
bool IterateDieRanges(Dwarf_Debug dbg, Dwarf_Die die, F func)
{
    Dwarf_Addr lowPc = DW_DLV_BADADDR;
    Dwarf_Addr highPc = DW_DLV_BADADDR;

    if (GetDieRange(die, &lowPc, &highPc)) {
        return func(lowPc, highPc);
    }

    Dwarf_Attribute attr;
    if (dwarf_attr(die, DW_AT_ranges, &attr, nullptr) != DW_DLV_OK) {
        return false;
    }

    return IterateDieRanges(dbg, die, func, attr, lowPc);
}

DebugInfo::CompUnit::~CompUnit()
{
    if (lineCtx_ != nullptr) {
        dwarf_srclines_dealloc_b(lineCtx_);
    }
    if (cuDie_ != nullptr) {
        dwarf_dealloc(dbg_, cuDie_, DW_DLA_DIE);
    }
}

Dwarf_Line_Context DebugInfo::CompUnit::GetLineContext()
{
    if (lineCtx_ != nullptr) {
        return lineCtx_;
    }
    // Decode line number information for the whole compilation unit
    Dwarf_Unsigned version = 0;
    Dwarf_Small tableCount = 0;
    if (dwarf_srclines_b(cuDie_, &version, &tableCount, &lineCtx_, nullptr) != DW_DLV_OK) {
        lineCtx_ = nullptr;
    }
    return lineCtx_;
}

DebugInfo::DebugInfo(DebugInfo &&info)
{
    fd_ = info.fd_;
    info.fd_ = INVALID_FD;
    dbg_ = info.dbg_;
    info.dbg_ = nullptr;
    aranges_ = info.aranges_;
    info.aranges_ = nullptr;
    arangeCount_ = info.arangeCount_;
    cuList_ = std::move(info.cuList_);
    ranges_ = std::move(info.ranges_);
}

void DebugInfo::Destroy()
{
    if (dbg_ == nullptr) {
        return;
    }
    if (aranges_ != nullptr) {
        FreeAranges(dbg_, aranges_, arangeCount_);
    }
    aranges_ = nullptr;
    arangeCount_ = 0;
    cuList_.clear();
    ranges_.clear();
    dwarf_finish(dbg_, nullptr);
    close(fd_);
    fd_ = INVALID_FD;
    dbg_ = nullptr;
}

DebugInfo &DebugInfo::operator=(DebugInfo &&info)
{
    fd_ = info.fd_;
    info.fd_ = INVALID_FD;
    dbg_ = info.dbg_;
    info.dbg_ = nullptr;
    aranges_ = info.aranges_;
    info.aranges_ = nullptr;
    arangeCount_ = info.arangeCount_;
    cuList_ = std::move(info.cuList_);
    ranges_ = std::move(info.ranges_);
    return *this;
}

DebugInfo::ErrorCode DebugInfo::ReadFromFile(const char *filename)
{
    Dwarf_Error err = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-signed-bitwise)
    fd_ = open(filename, O_RDONLY | O_CLOEXEC);
    int res = dwarf_init(fd_, DW_DLC_READ, DwarfErrorHandler, nullptr, &dbg_, &err);
    if (res != DW_DLV_OK) {
#ifdef PANDA_TARGET_LINUX_UBUNTU_22_04
        dwarf_dealloc(dbg_, err, DW_DLA_ERROR);
#else
        // NOTE(audovichenko): Libdwarf has a bug (memory leak).
        // In case dwarf_init fails it allocates memory for the error and  returns it in 'err' variable.
        // But since dbg is NULL, dwarf_dealloc just returns in case of dbg == nullptr and doesn't free this memory
        // A possible solution is to use 20201201 version and call dwarf_dealloc.
        free(err);  // NOLINT(cppcoreguidelines-no-malloc)
#endif  // PANDA_TARGET_LINUX_UBUNTU_22_04
        close(fd_);
        fd_ = INVALID_FD;
        dbg_ = nullptr;
    }
    if (res == DW_DLV_ERROR) {
        return ERROR;
    }
    if (res == DW_DLV_NO_ENTRY) {
        return NO_DEBUG_INFO;
    }
    // Aranges (address ranges) is an entity which help us to find the compilation unit quickly (something like index)
    if (dwarf_get_aranges(dbg_, &aranges_, &arangeCount_, nullptr) != DW_DLV_OK) {
        aranges_ = nullptr;
        arangeCount_ = 0;
    }
    return SUCCESS;
}

bool DebugInfo::GetSrcLocation(uintptr_t pc, std::string *function, std::string *srcFile, uint32_t *line)
{
    if (dbg_ == nullptr) {
        return false;
    }
    // Debug information have hierarchical structure.
    // Each node is represented by DIE (debug information entity).
    // .debug_info has a list of DIE which corresponds to compilation units (object files).
    // Mapping pc to function is to find the compilation unit DIE and then find the subprogram DIE.
    // From the subprogram DIE we get the function name.
    // Line information is available for compilation unit DIEs. So we decode lines for the whole
    // compilation unit and find the corresponding line and file which matches the pc.
    //
    // You could use objdump --dwarf=info <object file> to view available debug information.

    Range range(pc, pc);
    auto it = ranges_.upper_bound(range);
    if (it == ranges_.end() || !it->Contain(pc)) {
        Dwarf_Die cuDie = nullptr;
        if (!FindCompUnitByPc(pc, &cuDie)) {
            return false;
        }
        cuList_.emplace_back(CompUnit(cuDie, dbg_));
        auto ranges = &ranges_;
        auto cu = &cuList_.back();
        IterateDieRanges(dbg_, cuDie, [ranges, cu](Dwarf_Addr lowPc, Dwarf_Addr highPc) {
            ranges->insert(Range(lowPc, highPc, cu));
            return false;
        });
        TraverseChildren(cu, cuDie);
    }
    it = ranges_.upper_bound(range);
    if (it == ranges_.end() || !it->Contain(pc)) {
        return false;
    }

    ASSERT(it->GetCu() != nullptr);
    *function = it->GetFunction();
    // Find the corresponding line number and source file.
    GetSrcFileAndLine(pc, it->GetCu()->GetLineContext(), srcFile, line);
    return true;
}

bool DebugInfo::FindCompUnitByPc(uintptr_t pc, Dwarf_Die *cuDie)
{
    if (aranges_ != nullptr) {
        Dwarf_Arange arange = nullptr;
        Dwarf_Off offset = 0;
        if (dwarf_get_arange(aranges_, arangeCount_, pc, &arange, nullptr) == DW_DLV_OK &&
            dwarf_get_cu_die_offset(arange, &offset, nullptr) == DW_DLV_OK &&
            dwarf_offdie(dbg_, offset, cuDie, nullptr) == DW_DLV_OK) {
            return true;
        }
    }

    // No aranges are available or we can't find the corresponding arange. Iterate over all compilation units.
    // Its slow but works.
    Dwarf_Unsigned cuHeaderIdx;
    Dwarf_Half cuType;
    int res = dwarf_next_cu_header_d(dbg_, static_cast<Dwarf_Bool>(true), nullptr, nullptr, nullptr, nullptr, nullptr,
                                     nullptr, nullptr, nullptr, &cuHeaderIdx, &cuType, nullptr);
    while (res == DW_DLV_OK) {
        Dwarf_Die die = nullptr;
        if (dwarf_siblingof_b(dbg_, nullptr, static_cast<Dwarf_Bool>(true), &die, nullptr) == DW_DLV_OK) {
            if (PcMatches(pc, die)) {
                *cuDie = die;
                // Skip the rest cu headers because next time we need to stat search from the beginning.
                SkipCuHeaders(dbg_);
                return true;
            }
            dwarf_dealloc(dbg_, die, DW_DLA_DIE);
        }
        res = dwarf_next_cu_header_d(dbg_, static_cast<Dwarf_Bool>(true), nullptr, nullptr, nullptr, nullptr, nullptr,
                                     nullptr, nullptr, nullptr, &cuHeaderIdx, &cuType, nullptr);
    }
    return false;
}

void DebugInfo::TraverseChildren(CompUnit *cu, Dwarf_Die die)
{
    Dwarf_Die childDie = nullptr;
    if (dwarf_child(die, &childDie, nullptr) != DW_DLV_OK) {
        return;
    }
    TraverseSiblings(cu, childDie);
}

void DebugInfo::TraverseSiblings(CompUnit *cu, Dwarf_Die die)
{
    DwarfGuard g(dbg_, die, DW_DLA_DIE);
    Dwarf_Half tag = 0;
    int res;
    do {
        if (dwarf_tag(die, &tag, nullptr) != DW_DLV_OK) {
            return;
        }
        if ((tag == DW_TAG_subprogram || tag == DW_TAG_inlined_subroutine)) {
            Dwarf_Addr lowPc = DW_DLV_BADADDR;
            Dwarf_Addr highPc = 0;

            if (GetDieRange(die, &lowPc, &highPc)) {
                std::string fname;
                GetFunctionName(die, &fname);
                AddFunction(cu, lowPc, highPc, fname);
            }
        }
        TraverseChildren(cu, die);
        Dwarf_Die sibling = nullptr;
        res = dwarf_siblingof_b(dbg_, die, static_cast<Dwarf_Bool>(true), &sibling, nullptr);
        if (res == DW_DLV_OK) {
            g.Reset(sibling);
            die = sibling;
        }
    } while (res == DW_DLV_OK);
}

void DebugInfo::AddFunction(CompUnit *cu, Dwarf_Addr lowPc, Dwarf_Addr highPc, const std::string &function)
{
    auto it = ranges_.upper_bound(Range(lowPc, lowPc));
    ASSERT(it != ranges_.end());
    Range range(lowPc, highPc, cu, function);
    if (it->Contain(range)) {
        Range enclosing = *it;  // NOLINT(performance-unnecessary-copy-initialization)
        ranges_.erase(it);
        if (enclosing.GetLowPc() < lowPc) {
            ranges_.insert(Range(enclosing.GetLowPc(), lowPc, enclosing.GetCu(), enclosing.GetFunction()));
        }
        ranges_.insert(range);
        if (highPc < enclosing.GetHighPc()) {
            ranges_.insert(Range(highPc, enclosing.GetHighPc(), enclosing.GetCu(), enclosing.GetFunction()));
        }
    } else if (range.Contain(*it)) {
        ranges_.insert(Range(range.GetLowPc(), it->GetLowPc(), cu, function));
        ranges_.insert(Range(it->GetHighPc(), range.GetHighPc(), cu, function));
    } else if (highPc <= it->GetLowPc()) {
        ranges_.insert(range);
    }
}

void DebugInfo::GetFunctionName(Dwarf_Die die, std::string *function)
{
    char *name = nullptr;

    // Prefer linkage name instead of name
    // Linkage name is a mangled name which contains information about enclosing class,
    // return type, parameters and so on.
    // The name which is stored in DW_AT_name attribute is only a function name.
    if (dwarf_die_text(die, DW_AT_linkage_name, &name, nullptr) == DW_DLV_OK ||
        dwarf_diename(die, &name, nullptr) == DW_DLV_OK) {
        DwarfGuard g(dbg_, name, DW_DLA_STRING);
        *function = name;
        return;
    }

    Dwarf_Off off = 0;
    Dwarf_Attribute attr = nullptr;
    Dwarf_Die absOrigDie = nullptr;
    // If there is no name | linkage_name the function may be inlined.
    // Try to get it from the abstract origin
    if (dwarf_attr(die, DW_AT_abstract_origin, &attr, nullptr) == DW_DLV_OK) {
        DwarfGuard ag(dbg_, attr, DW_DLA_ATTR);
        if (dwarf_global_formref(attr, &off, nullptr) == DW_DLV_OK &&
            dwarf_offdie(dbg_, off, &absOrigDie, nullptr) == DW_DLV_OK) {
            DwarfGuard dg(dbg_, absOrigDie, DW_DLA_DIE);
            GetFunctionName(absOrigDie, function);
            return;
        }
    }

    // If there is no name | linkage_name try to get it from the specification.
    Dwarf_Die specDie = nullptr;
    if (dwarf_attr(die, DW_AT_specification, &attr, nullptr) == DW_DLV_OK) {
        DwarfGuard ag(dbg_, attr, DW_DLA_ATTR);
        if (dwarf_global_formref(attr, &off, nullptr) == DW_DLV_OK &&
            dwarf_offdie(dbg_, off, &specDie, nullptr) == DW_DLV_OK) {
            DwarfGuard dg(dbg_, specDie, DW_DLA_DIE);
            GetFunctionName(specDie, function);
        }
    }
}

bool DebugInfo::GetSrcFileAndLine(uintptr_t pc, Dwarf_Line_Context lineCtx, std::string *outSrcFile, uint32_t *outLine)
{
    if (lineCtx == nullptr) {
        return false;
    }
    Dwarf_Line *lineBuf = nullptr;
    Dwarf_Signed lineBufSize = 0;
    if (dwarf_srclines_from_linecontext(lineCtx, &lineBuf, &lineBufSize, nullptr) != DW_DLV_OK) {
        return false;
    }
    Span<Dwarf_Line> lines(lineBuf, lineBufSize);
    Dwarf_Addr prevLinePc = 0;
    Dwarf_Line prevLine = nullptr;
    bool found = false;
    for (auto it = lines.begin(); it != lines.end() && !found; ++it) {
        Dwarf_Line line = *it;
        Dwarf_Addr linePc = 0;
        dwarf_lineaddr(line, &linePc, nullptr);
        if (pc == linePc) {
            GetSrcFileAndLine(GetLastLineWithPc(pc, it, lines.end()), outSrcFile, outLine);
            found = true;
        } else if (prevLine != nullptr && prevLinePc < pc && pc < linePc) {
            GetSrcFileAndLine(prevLine, outSrcFile, outLine);
            found = true;
        } else {
            Dwarf_Bool isLineEnd;
            dwarf_lineendsequence(line, &isLineEnd, nullptr);
            if (isLineEnd != 0) {
                prevLine = nullptr;
            } else {
                prevLinePc = linePc;
                prevLine = line;
            }
        }
    }
    return found;
}

Dwarf_Line DebugInfo::GetLastLineWithPc(Dwarf_Addr pc, Span<Dwarf_Line>::ConstIterator it,
                                        Span<Dwarf_Line>::ConstIterator end)
{
    Dwarf_Addr linePc = 0;
    auto next = std::next(it);
    while (next != end) {
        dwarf_lineaddr(*next, &linePc, nullptr);
        if (pc != linePc) {
            return *it;
        }
        it = next;
        ++next;
    }
    return *it;
}

void DebugInfo::GetSrcFileAndLine(Dwarf_Line line, std::string *outSrcFile, uint32_t *outLine)
{
    Dwarf_Unsigned ln;
    dwarf_lineno(line, &ln, nullptr);
    *outLine = ln;
    char *srcFile = nullptr;
    if (dwarf_linesrc(line, &srcFile, nullptr) == DW_DLV_OK) {
        *outSrcFile = srcFile;
        DwarfGuard g(dbg_, srcFile, DW_DLA_STRING);
    } else {
        dwarf_linesrc(line, &srcFile, nullptr);
        *outSrcFile = srcFile;
        DwarfGuard g(dbg_, srcFile, DW_DLA_STRING);
    }
}

bool DebugInfo::PcMatches(uintptr_t pc, Dwarf_Die die)
{
    Dwarf_Addr lowPc = DW_DLV_BADADDR;
    Dwarf_Addr highPc = 0;
    return GetDieRangeForPc(pc, die, &lowPc, &highPc);
}

bool DebugInfo::GetDieRangeForPc(uintptr_t pc, Dwarf_Die die, Dwarf_Addr *outLowPc, Dwarf_Addr *outHighPc)
{
    Dwarf_Addr lowPc = DW_DLV_BADADDR;
    Dwarf_Addr highPc = 0;

    if (GetDieRange(die, &lowPc, &highPc) && (*outLowPc <= pc && pc < *outHighPc)) {
        *outLowPc = lowPc;
        *outHighPc = highPc;
        return true;
    }

    Dwarf_Attribute attr;
    if (dwarf_attr(die, DW_AT_ranges, &attr, nullptr) == DW_DLV_OK) {
        DwarfGuard g(dbg_, attr, DW_DLA_ATTR);
        Dwarf_Unsigned offset;
        Dwarf_Addr baseAddr = 0;
        if (lowPc != DW_DLV_BADADDR) {
            baseAddr = lowPc;
        }
        Dwarf_Signed count = 0;
        Dwarf_Ranges *ranges = nullptr;
        if (dwarf_global_formref(attr, &offset, nullptr) == DW_DLV_OK &&
            dwarf_get_ranges_a(dbg_, offset, die, &ranges, &count, nullptr, nullptr) == DW_DLV_OK) {
            Dwarf_Debug dbg = dbg_;
            AtReturn r([dbg, ranges, count]() { dwarf_ranges_dealloc(dbg, ranges, count); });
            return FindRangeForPc(pc, Span<Dwarf_Ranges>(ranges, count), baseAddr, outLowPc, outHighPc);
        }
    }
    return false;
}

bool DebugInfo::FindRangeForPc(uintptr_t pc, const Span<Dwarf_Ranges> &ranges, Dwarf_Addr baseAddr,
                               Dwarf_Addr *outLowPc, Dwarf_Addr *outHighPc)
{
    for (const Dwarf_Ranges &range : ranges) {
        if (range.dwr_type == DW_RANGES_ENTRY) {
            Dwarf_Addr rngLowPc = baseAddr + range.dwr_addr1;
            Dwarf_Addr rngHighPc = baseAddr + range.dwr_addr2;
            if (rngLowPc <= pc && pc < rngHighPc) {
                *outLowPc = rngLowPc;
                *outHighPc = rngHighPc;
                return true;
            }
        } else if (range.dwr_type == DW_RANGES_ADDRESS_SELECTION) {
            baseAddr = range.dwr_addr2;
        } else {
            break;
        }
    }
    return false;
}

}  // namespace ark
