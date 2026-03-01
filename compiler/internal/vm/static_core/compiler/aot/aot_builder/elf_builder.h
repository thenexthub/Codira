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

#ifndef COMPILER_AOT_AOT_BULDER_ELF_BUILDER_H
#define COMPILER_AOT_AOT_BULDER_ELF_BUILDER_H

#include <elf.h>
#include <vector>
#include <string>
#include <tuple>
#include <fstream>
#include <functional>
#ifdef PANDA_COMPILER_DEBUG_INFO
#include <libdwarf/libdwarf.h>
#include <libdwarf/dwarf.h>
#endif

#include "include/class.h"

namespace ark::compiler {

class ElfSectionDataProvider {
public:
    using DataCallback = void (*)(Span<const uint8_t>);

    virtual void FillData(Span<uint8_t> os, size_t pos) const = 0;
    virtual size_t GetDataSize() const = 0;

    ElfSectionDataProvider() = default;
    NO_COPY_SEMANTIC(ElfSectionDataProvider);
    NO_MOVE_SEMANTIC(ElfSectionDataProvider);
    virtual ~ElfSectionDataProvider() = default;
};

#ifdef PANDA_COMPILER_DEBUG_INFO
class DwarfSectionData {
public:
    using FdeInst = std::tuple<Dwarf_Small, Dwarf_Unsigned, Dwarf_Unsigned>;
    DwarfSectionData() = default;

    void AddFdeInst(Dwarf_Small op, Dwarf_Unsigned val1, Dwarf_Unsigned val2)
    {
        fde_.emplace_back(op, val1, val2);
    }

    void AddFdeInst(const FdeInst &fde)
    {
        fde_.push_back(fde);
    }

    void SetOffset(Dwarf_Unsigned offset)
    {
        offset_ = offset;
    }

    void SetSize(Dwarf_Unsigned size)
    {
        size_ = size;
    }

    const auto &GetFde() const
    {
        return fde_;
    }

    auto GetOffset() const
    {
        return offset_;
    }

    auto GetSize() const
    {
        return size_;
    }

private:
    std::vector<FdeInst> fde_;
    Dwarf_Unsigned offset_ {0};
    Dwarf_Unsigned size_ {0};
};
#endif  // PANDA_COMPILER_DEBUG_INFO

template <Arch ARCH, bool IS_JIT_MODE = false>
class ElfBuilder {  // NOLINT(cppcoreguidelines-special-member-functions)
    static constexpr size_t PAGE_SIZE_VALUE = 0x1000;
    using ElfAddr = std::conditional_t<ArchTraits<ARCH>::IS_64_BITS, Elf64_Addr, Elf32_Addr>;
    using ElfOff = std::conditional_t<ArchTraits<ARCH>::IS_64_BITS, Elf64_Off, Elf32_Off>;
    using ElfHalf = std::conditional_t<ArchTraits<ARCH>::IS_64_BITS, Elf64_Half, Elf32_Half>;
    using ElfWord = std::conditional_t<ArchTraits<ARCH>::IS_64_BITS, Elf64_Word, Elf32_Word>;
    using ElfSword = std::conditional_t<ArchTraits<ARCH>::IS_64_BITS, Elf64_Sword, Elf32_Sword>;
    using ElfXword = std::conditional_t<ArchTraits<ARCH>::IS_64_BITS, Elf64_Xword, Elf32_Xword>;
    using ElfSxword = std::conditional_t<ArchTraits<ARCH>::IS_64_BITS, Elf64_Sxword, Elf32_Sxword>;
    using ElfEhdr = std::conditional_t<ArchTraits<ARCH>::IS_64_BITS, Elf64_Ehdr, Elf32_Ehdr>;
    using ElfShdr = std::conditional_t<ArchTraits<ARCH>::IS_64_BITS, Elf64_Shdr, Elf32_Shdr>;
    using ElfSym = std::conditional_t<ArchTraits<ARCH>::IS_64_BITS, Elf64_Sym, Elf32_Sym>;
    using ElfRel = std::conditional_t<ArchTraits<ARCH>::IS_64_BITS, Elf64_Rel, Elf32_Rel>;
    using ElfRela = std::conditional_t<ArchTraits<ARCH>::IS_64_BITS, Elf64_Rela, Elf32_Rela>;
    using ElfPhdr = std::conditional_t<ArchTraits<ARCH>::IS_64_BITS, Elf64_Phdr, Elf32_Phdr>;
    using ElfDyn = std::conditional_t<ArchTraits<ARCH>::IS_64_BITS, Elf64_Dyn, Elf32_Dyn>;
    using ElfSection = std::conditional_t<ArchTraits<ARCH>::IS_64_BITS, Elf64_Section, Elf32_Section>;

public:
    class Section {
    public:
        using ElfShdrParams = std::array<ElfWord, 5U>;

        // NOLINTNEXTLINE(modernize-pass-by-value)
        Section(ElfBuilder &builder, const std::string &name, Section *link, const ElfShdrParams &params)
            : builder_(builder), name_(name), link_(link)
        {
            header_.sh_type = params[0U];
            header_.sh_flags = params[1U];
            header_.sh_info = params[2U];
            header_.sh_addralign = params[3U];
            header_.sh_entsize = params[4U];
        }
        virtual ~Section() = default;
        NO_MOVE_SEMANTIC(Section);
        NO_COPY_SEMANTIC(Section);

        virtual void *GetData()
        {
            UNREACHABLE();
        }
        virtual size_t GetDataSize() const
        {
            return header_.sh_size;
        }

        const std::string &GetName() const
        {
            return name_;
        }

        size_t GetIndex() const
        {
            return index_;
        }

        auto GetAddress() const
        {
            return header_.sh_addr;
        }

        auto GetOffset() const
        {
            return header_.sh_offset;
        }

        void SetSize(size_t size)
        {
            header_.sh_size = size;
        }

        void SetDataProvider(std::unique_ptr<ElfSectionDataProvider> dataProvider)
        {
            dataProvider_ = std::move(dataProvider);
        }

        ElfSectionDataProvider *GetDataProvider() const
        {
            return dataProvider_.get();
        }

    private:
        ElfBuilder &builder_;
        std::string name_;
        size_t index_ {std::numeric_limits<size_t>::max()};
        ElfShdr header_ {};
        Section *link_ {};
        std::unique_ptr<ElfSectionDataProvider> dataProvider_ {nullptr};

        friend class ElfBuilder;
    };

    class DataSection : public Section {
    public:
        using Section::Section;

        DataSection() = default;
        NO_COPY_SEMANTIC(DataSection);
        NO_MOVE_SEMANTIC(DataSection);
        ~DataSection() override = default;  // NOLINT(hicpp-use-override, modernize-use-override)

        void AppendData(const void *data, size_t size)
        {
            auto pdata = reinterpret_cast<const uint8_t *>(data);
            data_.insert(data_.end(), pdata, pdata + size);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }

        void *GetData() override
        {
            return data_.data();
        }

        auto &GetVector()
        {
            return data_;
        }

        size_t GetDataSize() const override
        {
            return this->dataProvider_ != nullptr ? this->dataProvider_->GetDataSize() : data_.size();
        }

    private:
        std::vector<uint8_t> data_;
    };

    class SymbolSection : public Section {
    public:
        using ThunkFunc = std::function<ElfAddr(void)>;
        using typename Section::ElfShdrParams;

        using Section::Section;
        SymbolSection(ElfBuilder &builder, const std::string &name, Section *link, const ElfShdrParams &params)
            : Section(builder, name, link, params)
        {
            symbols_.emplace_back(ElfSym {});
            thunks_.emplace_back();
        }

        void *GetData() override
        {
            return symbols_.data();
        }

        size_t GetDataSize() const override
        {
            return symbols_.size() * sizeof(ElfSym);
        }

        void Resolve();

    private:
        std::vector<ElfSym> symbols_;
        std::vector<ThunkFunc> thunks_;

        friend class ElfBuilder;
    };

    class StringSection : public DataSection {
    public:
        StringSection(ElfBuilder &builder, const std::string &name, ElfWord flags, ElfWord align)
            : DataSection(builder, name, nullptr, {SHT_STRTAB, flags, 0, align, 0})
        {
            AddString("\0");  // NOLINT(bugprone-string-literal-with-embedded-nul)
        }

        ElfWord AddString(const std::string &str)
        {
            auto pos = DataSection::GetDataSize();
            DataSection::AppendData(str.data(), str.size() + 1);
            return pos;
        }
    };

public:
    ~ElfBuilder()
    {
        for (auto roDataSection : roDataSections_) {
            delete roDataSection;
        }
        for (auto segment : segments_) {
            delete segment;
        }
    }

    ElfBuilder()
    {
        AddSection(&hashSection_);
        AddSection(&textSection_);
        AddSection(&shstrtabSection_);
        AddSection(&dynstrSection_);
        AddSection(&dynsymSection_);
        if constexpr (!IS_JIT_MODE) {  // NOLINT
            AddSection(&aotSection_);
            AddSection(&gotSection_);
            AddSection(&dynamicSection_);
        }
#ifdef PANDA_COMPILER_DEBUG_INFO
        AddSection(&frameSection_);
#endif
    }

    NO_MOVE_SEMANTIC(ElfBuilder);
    NO_COPY_SEMANTIC(ElfBuilder);

    ElfWord AddSectionName(const std::string &name)
    {
        return name.empty() ? 0 : shstrtabSection_.AddString(name);
    }

    void AddSection(Section *section)
    {
        sections_.push_back(section);
        section->index_ = sections_.size() - 1;
    }

    template <bool IS_FUNCTION = false>
    void AddSymbol(const std::string &name, ElfWord size, const Section &section,
                   typename SymbolSection::ThunkFunc thunk);

    auto GetTextSection()
    {
        return &textSection_;
    }

    auto GetAotSection()
    {
        return &aotSection_;
    }

#ifdef PANDA_COMPILER_DEBUG_INFO
    auto GetFrameSection()
    {
        return &frameSection_;
    }

    void SetFrameData(std::vector<DwarfSectionData> *frameData)
    {
        frameData_ = frameData;
    }
    void FillFrameSection();

    void SetCodeName(const std::string &methodName)
    {
        methodName_ = methodName;
    }
#endif

    auto GetGotSection()
    {
        return &gotSection_;
    }

    void PreSizeRoDataSections(size_t newSize)
    {
        roDataSections_.reserve(newSize);
    }

    void AddRoDataSection(const std::string &name, size_t alignment)
    {
        ASSERT(!name.empty());
        auto section = new DataSection(*this, name, nullptr,
                                       // CC-OFFNXT(G.FMT.03) project code style
                                       {SHT_PROGBITS, SHF_ALLOC | SHF_MERGE,  // NOLINT(hicpp-signed-bitwise)
                                        0, static_cast<ElfWord>(alignment), 0});
        AddSection(section);
        roDataSections_.push_back(section);
    }

    std::vector<DataSection *> *GetRoDataSections()
    {
        return &roDataSections_;
    }

    void Build(const std::string &fileName);

    void SettleSection(Section *section);

    void Write(const std::string &fileName);
    void Write(Span<uint8_t> stream);

    ElfOff UpdateOffset(ElfWord align)
    {
        currentOffset_ = RoundUp(currentOffset_, align);
        return currentOffset_;
    }

    ElfOff UpdateAddress(ElfWord align)
    {
        currentAddress_ = RoundUp(currentAddress_, align);
        return currentAddress_;
    }

    size_t GetFileSize() const
    {
        return currentOffset_;
    }

#ifdef PANDA_COMPILER_DEBUG_INFO
    std::vector<DwarfSectionData> *GetFrameData()
    {
        return frameData_;
    }

    Span<uint8_t> GetTextSectionData() const
    {
        static_assert(IS_JIT_MODE);
        return {reinterpret_cast<uint8_t *>(textSection_.header_.sh_addr), textSection_.header_.sh_size};
    }

    void HackAddressesForJit(const uint8_t *elfData)
    {
        static_assert(IS_JIT_MODE);
        ASSERT(frameData_->size() == 1U);
        ASSERT(segments_.empty());
        ASSERT(dynsymSection_.symbols_.size() == 2U);

        for (auto section : sections_) {
            if ((section->header_.sh_flags & SHF_ALLOC) != 0) {  // NOLINT(hicpp-signed-bitwise)
                section->header_.sh_addr += down_cast<typename ElfBuilder<ARCH, IS_JIT_MODE>::ElfAddr>(elfData);
            }
        }

        ASSERT(dynsymSection_.symbols_[0].st_value == 0);
        dynsymSection_.symbols_[1U].st_value +=
            down_cast<typename ElfBuilder<ARCH, IS_JIT_MODE>::ElfAddr>(elfData) + CodeInfo::GetCodeOffset(ARCH);

        // Some dark magic there. Here we patch the address of JIT code in frame debug info entry.
        // NOTE (asidorov): rework to more readable code
        uint8_t *cieAddr8 {static_cast<uint8_t *>(frameSection_.GetData())};
        uint32_t *cieAddr32 {reinterpret_cast<uint32_t *>(cieAddr8)};
        uint32_t cieLength {*cieAddr32 + static_cast<uint32_t>(sizeof(uint32_t))};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        uint8_t *fdeInitialPcAddr8 {cieAddr8 + cieLength + 2U * sizeof(uint32_t)};
        uintptr_t *fdeInitialPcAddrPtr {reinterpret_cast<uintptr_t *>(fdeInitialPcAddr8)};
        *fdeInitialPcAddrPtr += down_cast<typename ElfBuilder<ARCH, IS_JIT_MODE>::ElfAddr>(elfData);
    }
#endif

private:
    void MakeHeader();
    void AddSymbols();
    void SettleSectionsForAot();
    void SettleSectionsForJit();
    void ConstructHashSection();
    void ConstructDynamicSection(const std::string &fileName);

    struct Segment {
        Segment(ElfAddr addr, ElfOff offset, ElfWord type, ElfWord flags, ElfWord align)
        {
            header.p_type = type;
            header.p_flags = flags;
            header.p_vaddr = header.p_paddr = addr;
            header.p_align = align;
            header.p_offset = offset;
        }

        ElfPhdr header {};  // NOLINT(misc-non-private-member-variables-in-classes)
    };

    class SegmentScope {
    public:
        template <typename... Args>
        SegmentScope(ElfBuilder &builder, ElfWord type, ElfWord flags, bool first = false)
            : builder_(builder),
              startAddress_(first ? 0 : RoundUp(builder.currentAddress_, PAGE_SIZE_VALUE)),
              startOffset_(first ? 0 : RoundUp(builder.currentOffset_, PAGE_SIZE_VALUE))
        {
            auto *segment = new Segment(startAddress_, startOffset_, type, flags, PAGE_SIZE_VALUE);
            builder_.segments_.push_back(segment);
            builder_.currentSegment_ = segment;
        }
        ~SegmentScope()
        {
            builder_.currentSegment_->header.p_filesz = builder_.currentOffset_ - startOffset_;
            builder_.currentSegment_->header.p_memsz = builder_.currentAddress_ - startAddress_;
            builder_.currentSegment_ = nullptr;
        }

        NO_MOVE_SEMANTIC(SegmentScope);
        NO_COPY_SEMANTIC(SegmentScope);

    private:
        ElfBuilder &builder_;
        ElfAddr startAddress_;
        ElfAddr startOffset_;
    };

    class AddrPatch {
        using PatchFunc = std::function<ElfAddr(void)>;

    public:
        AddrPatch(ElfAddr *addr, PatchFunc func) : address_(addr), patchFunc_(std::move(func)) {}
        void Patch()
        {
            *address_ = patchFunc_();
        }

    private:
        ElfAddr *address_;
        PatchFunc patchFunc_;
    };

private:
    static constexpr size_t MAX_SEGMENTS_COUNT = 10;
    static constexpr size_t DYNSTR_SECTION_ALIGN = 8;
    static constexpr size_t JIT_TEXT_ALIGNMENT = ArchTraits<ARCH>::CODE_ALIGNMENT;
    static constexpr size_t JIT_DATA_ALIGNMENT = ArchTraits<ARCH>::POINTER_SIZE;
    static constexpr size_t JIT_DYNSTR_ALIGNMENT = 1U;
    ElfEhdr header_ {};
    std::vector<Section *> sections_;
    std::vector<Segment *> segments_;
    std::vector<AddrPatch> patches_;
    ElfAddr currentAddress_ {0};
    ElfOff currentOffset_ {0};
    Segment *currentSegment_ {nullptr};

    std::vector<DataSection *> roDataSections_;
    DataSection hashSection_ =  // NOLINTNEXTLINE(hicpp-signed-bitwise)
        DataSection(*this, ".hash", &dynsymSection_, {SHT_HASH, SHF_ALLOC, 0, sizeof(ElfWord), sizeof(ElfWord)});
    DataSection textSection_ =  // NOLINTNEXTLINE(hicpp-signed-bitwise)
        DataSection(
            *this, ".text", nullptr,
            {SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR, 0, IS_JIT_MODE ? JIT_TEXT_ALIGNMENT : PAGE_SIZE_VALUE, 0});
    StringSection shstrtabSection_ = StringSection(*this, ".shstrtab", 0, 1);
    StringSection dynstrSection_ =  // NOLINTNEXTLINE(hicpp-signed-bitwise)
        StringSection(*this, ".dynstr", SHF_ALLOC, IS_JIT_MODE ? JIT_DYNSTR_ALIGNMENT : DYNSTR_SECTION_ALIGN);
    SymbolSection dynsymSection_ =  // NOLINTNEXTLINE(hicpp-signed-bitwise)
        SymbolSection(*this, ".dynsym", &dynstrSection_, {SHT_DYNSYM, SHF_ALLOC, 1, sizeof(ElfOff), sizeof(ElfSym)});
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    DataSection aotSection_ = DataSection(*this, ".aot", nullptr, {SHT_PROGBITS, SHF_ALLOC, 0, sizeof(ElfWord), 0});
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    DataSection gotSection_ = DataSection(*this, ".aot_got", nullptr, {SHT_PROGBITS, SHF_ALLOC, 0, PAGE_SIZE_VALUE, 0});
    DataSection dynamicSection_ =  // NOLINTNEXTLINE(hicpp-signed-bitwise)
        DataSection(*this, ".dynamic", &dynstrSection_, {SHT_DYNAMIC, SHF_ALLOC, 0, PAGE_SIZE_VALUE, sizeof(ElfDyn)});
#ifdef PANDA_COMPILER_DEBUG_INFO
    DataSection frameSection_ =  // NOLINTNEXTLINE(hicpp-signed-bitwise)
        DataSection(*this, ".eh_frame", nullptr,
                    {SHT_PROGBITS, SHF_ALLOC, 0, IS_JIT_MODE ? JIT_DATA_ALIGNMENT : PAGE_SIZE_VALUE, 0});

    std::vector<DwarfSectionData> *frameData_ {nullptr};
#endif
    std::string methodName_ = std::string("code");

    friend SegmentScope;
};

template <Arch ARCH, bool IS_JIT_MODE>
template <bool IS_FUNCTION>
void ElfBuilder<ARCH, IS_JIT_MODE>::AddSymbol(const std::string &name, ElfWord size, const Section &section,
                                              typename SymbolSection::ThunkFunc thunk)
{
    uint8_t symbolType = IS_FUNCTION ? STT_FUNC : STT_OBJECT;
    auto nameIdx = dynstrSection_.AddString(name);
    constexpr uint8_t LOW_BITS_MASK = 0b1111;
    auto stInfo =
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        static_cast<uint8_t>((STB_GLOBAL << 4U) + (symbolType & LOW_BITS_MASK));

    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (ArchTraits<ARCH>::IS_64_BITS) {
        dynsymSection_.symbols_.push_back({nameIdx,                                      // st_name
                                           stInfo,                                       // st_info
                                           0,                                            // st_other
                                           static_cast<ElfSection>(section.GetIndex()),  // st_shndx
                                           0,                                            // st_value
                                           size});                                       // st_size
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        dynsymSection_.symbols_.push_back({nameIdx,                                        // st_name
                                           0,                                              // st_value
                                           size,                                           // st_size
                                           stInfo,                                         // st_info
                                           0,                                              // st_other
                                           static_cast<ElfSection>(section.GetIndex())});  // st_shndx
    }
    dynsymSection_.thunks_.push_back(thunk);
}

template <Arch ARCH, bool IS_JIT_MODE>
void ElfBuilder<ARCH, IS_JIT_MODE>::SymbolSection::Resolve()
{
    for (auto i = 0U; i < thunks_.size(); i++) {
        if (thunks_[i]) {
            symbols_[i].st_value = thunks_[i]();
        }
    }
}

template <Arch ARCH, bool IS_JIT_MODE>
void ElfBuilder<ARCH, IS_JIT_MODE>::ConstructDynamicSection(const std::string &fileName)
{
    using ElfDynDValType = decltype(ElfDyn::d_un.d_val);  // NOLINT(cppcoreguidelines-pro-type-union-access)
    auto soname = dynstrSection_.AddString(fileName);
    auto dynstrSectionSize = dynstrSection_.GetDataSize();
    // Make sure widening is zero-extension, if any
    static_assert(std::is_unsigned<decltype(soname)>::value && std::is_unsigned<decltype(dynstrSectionSize)>::value);

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    ElfDyn dyns[] = {
        {DT_HASH, {0}},    // will be patched
        {DT_STRTAB, {0}},  // will be patched
        {DT_SYMTAB, {0}},  // will be patched
        {DT_SYMENT, {sizeof(ElfSym)}},
        {DT_STRSZ, {static_cast<ElfDynDValType>(dynstrSectionSize)}},
        {DT_SONAME, {static_cast<ElfDynDValType>(soname)}},
        {DT_NULL, {0}},
    };
    dynamicSection_.AppendData(&dyns, sizeof(dyns));

    auto firstPatchArgument =  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        reinterpret_cast<ElfAddr *>(reinterpret_cast<uint8_t *>(dynamicSection_.GetData()) +
                                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                                    2U * sizeof(ElfDyn) + offsetof(ElfDyn, d_un.d_ptr));
    patches_.emplace_back(firstPatchArgument, [this]() { return dynsymSection_.GetAddress(); });

    auto secondPatchArgument =  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        reinterpret_cast<ElfAddr *>(reinterpret_cast<uint8_t *>(dynamicSection_.GetData()) +
                                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                                    1U * sizeof(ElfDyn) + offsetof(ElfDyn, d_un.d_ptr));
    patches_.emplace_back(secondPatchArgument, [this]() { return dynstrSection_.GetAddress(); });

    auto thirdPatchArgument =  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        reinterpret_cast<ElfAddr *>(reinterpret_cast<uint8_t *>(dynamicSection_.GetData()) +
                                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                                    0U * sizeof(ElfDyn) + offsetof(ElfDyn, d_un.d_ptr));
    patches_.emplace_back(thirdPatchArgument, [this]() { return hashSection_.GetAddress(); });
}

template <Arch ARCH, bool IS_JIT_MODE>
void ElfBuilder<ARCH, IS_JIT_MODE>::Build(const std::string &fileName)
{
    shstrtabSection_.header_.sh_name = AddSectionName(shstrtabSection_.GetName());
    for (auto section : sections_) {
        section->header_.sh_name = AddSectionName(section->GetName());
        if (section->link_) {
            section->header_.sh_link = section->link_->GetIndex();
        }
    }

    AddSymbols();

    ConstructHashSection();

    if constexpr (!IS_JIT_MODE) {  // NOLINT
        ConstructDynamicSection(fileName);
    }

    if constexpr (IS_JIT_MODE) {  // NOLINT
        SettleSectionsForJit();
    } else {  // NOLINT
        SettleSectionsForAot();
    }

    MakeHeader();

    dynsymSection_.Resolve();
    std::for_each(patches_.begin(), patches_.end(), [](auto &patch) { patch.Patch(); });
}

template <Arch ARCH, bool IS_JIT_MODE>
void ElfBuilder<ARCH, IS_JIT_MODE>::ConstructHashSection()
{
    ElfWord value = 1;
    auto symCount = dynsymSection_.GetDataSize() / sizeof(ElfSym);
    hashSection_.AppendData(&value, sizeof(value));
    hashSection_.AppendData(&symCount, sizeof(value));
    hashSection_.AppendData(&value, sizeof(value));
    value = 0;
    hashSection_.AppendData(&value, sizeof(value));
    for (auto i = 2U; i < symCount; i++) {
        hashSection_.AppendData(&i, sizeof(value));
    }
    value = 0;
    hashSection_.AppendData(&value, sizeof(value));
}

template <Arch ARCH, bool IS_JIT_MODE>
void ElfBuilder<ARCH, IS_JIT_MODE>::MakeHeader()
{
    header_.e_ident[EI_MAG0] = ELFMAG0;
    header_.e_ident[EI_MAG1] = ELFMAG1;
    header_.e_ident[EI_MAG2] = ELFMAG2;
    header_.e_ident[EI_MAG3] = ELFMAG3;
    header_.e_ident[EI_CLASS] = ArchTraits<ARCH>::IS_64_BITS ? ELFCLASS64 : ELFCLASS32;
    header_.e_ident[EI_DATA] = ELFDATA2LSB;
    header_.e_ident[EI_VERSION] = EV_CURRENT;
    header_.e_ident[EI_OSABI] = ELFOSABI_LINUX;
    header_.e_ident[EI_ABIVERSION] = 0;
    std::fill_n(&header_.e_ident[EI_PAD], EI_NIDENT - EI_PAD, 0);
    header_.e_type = ET_DYN;
    header_.e_version = 1;
    switch (ARCH) {
        case Arch::AARCH32:
            header_.e_machine = EM_ARM;
            header_.e_flags = EF_ARM_EABI_VER5;
            break;
        case Arch::AARCH64:
            header_.e_machine = EM_AARCH64;
            header_.e_flags = 0;
            break;
        case Arch::X86:
            header_.e_machine = EM_386;
            header_.e_flags = 0;
            break;
        case Arch::X86_64:
            header_.e_machine = EM_X86_64;
            header_.e_flags = 0;
            break;
        default:
            UNREACHABLE();
    }
    header_.e_entry = 0;
    header_.e_ehsize = sizeof(ElfEhdr);
    header_.e_phentsize = sizeof(ElfPhdr);
    header_.e_shentsize = sizeof(ElfShdr);
    header_.e_shstrndx = shstrtabSection_.GetIndex();
    currentOffset_ = RoundUp(currentOffset_, alignof(ElfShdr));
    header_.e_shoff = UpdateOffset(alignof(ElfShdr));
    currentOffset_ += sections_.size() * sizeof(ElfShdr);
    header_.e_phoff = IS_JIT_MODE ? 0U : sizeof(ElfEhdr);
    header_.e_shnum = sections_.size();
    header_.e_phnum = IS_JIT_MODE ? 0U : segments_.size();
}

template <Arch ARCH, bool IS_JIT_MODE>
void ElfBuilder<ARCH, IS_JIT_MODE>::AddSymbols()
{
    AddSymbol(methodName_, textSection_.GetDataSize(), textSection_, [this]() { return textSection_.GetAddress(); });
    if constexpr (!IS_JIT_MODE) {  // NOLINT
        AddSymbol("code_end", textSection_.GetDataSize(), textSection_,
                  [this]() { return textSection_.GetAddress() + textSection_.GetDataSize(); });
        AddSymbol("aot", aotSection_.GetDataSize(), aotSection_, [this]() { return aotSection_.GetAddress(); });
        AddSymbol("aot_end", aotSection_.GetDataSize(), aotSection_,
                  [this]() { return aotSection_.GetAddress() + aotSection_.GetDataSize(); });
    }
}

template <Arch ARCH, bool IS_JIT_MODE>
void ElfBuilder<ARCH, IS_JIT_MODE>::SettleSectionsForAot()
{
    static_assert(!IS_JIT_MODE);

    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    auto phdrSegment = new Segment(sizeof(ElfEhdr), sizeof(ElfEhdr), PT_PHDR, PF_R, sizeof(ElfOff));
    segments_.push_back(phdrSegment);

    {
        SegmentScope segmentScope(*this, PT_LOAD, PF_R, true);  // NOLINT(hicpp-signed-bitwise)
        currentAddress_ = sizeof(ElfEhdr) + sizeof(ElfPhdr) * MAX_SEGMENTS_COUNT;
        currentOffset_ = sizeof(ElfEhdr) + sizeof(ElfPhdr) * MAX_SEGMENTS_COUNT;
        SettleSection(&dynstrSection_);
        SettleSection(&dynsymSection_);
        SettleSection(&hashSection_);
        SettleSection(&aotSection_);
        for (auto roDataSection : roDataSections_) {
            ASSERT(roDataSection->GetDataSize() > 0);
            SettleSection(roDataSection);
        }
    }

    {
        SegmentScope segmentScope(*this, PT_LOAD, PF_R | PF_W);  // NOLINT(hicpp-signed-bitwise)
        SettleSection(&gotSection_);
    }

    {
        SegmentScope segmentScope(*this, PT_LOAD, PF_R | PF_X);  // NOLINT(hicpp-signed-bitwise)
        SettleSection(&textSection_);
    }

#ifdef PANDA_COMPILER_DEBUG_INFO
    if (!frameData_->empty()) {
        SegmentScope segmentScope(*this, PT_LOAD, PF_R);  // NOLINT(hicpp-signed-bitwise)
        FillFrameSection();
        SettleSection(&frameSection_);
    }
#endif

    {
        SegmentScope segmentScope(*this, PT_DYNAMIC, PF_R | PF_W);  // NOLINT(hicpp-signed-bitwise)
        SettleSection(&dynamicSection_);
    }

    SettleSection(&shstrtabSection_);

    auto lodDynamicSegment = new Segment(*segments_.back());
    ASSERT(lodDynamicSegment->header.p_type == PT_DYNAMIC);
    lodDynamicSegment->header.p_type = PT_LOAD;
    segments_.insert(segments_.end() - 1, lodDynamicSegment);

    ASSERT(segments_.size() <= MAX_SEGMENTS_COUNT);
    phdrSegment->header.p_filesz = phdrSegment->header.p_memsz = segments_.size() * sizeof(ElfPhdr);
}

template <Arch ARCH, bool IS_JIT_MODE>
void ElfBuilder<ARCH, IS_JIT_MODE>::SettleSectionsForJit()
{
    static_assert(IS_JIT_MODE);

    currentAddress_ = sizeof(ElfEhdr);
    currentOffset_ = sizeof(ElfEhdr);

    SettleSection(&textSection_);
#ifdef PANDA_COMPILER_DEBUG_INFO
    if (!frameData_->empty()) {
        FillFrameSection();
        SettleSection(&frameSection_);
    }
#endif
    SettleSection(&dynsymSection_);
    SettleSection(&hashSection_);
    SettleSection(&dynstrSection_);
    SettleSection(&shstrtabSection_);
}

template <Arch ARCH, bool IS_JIT_MODE>
void ElfBuilder<ARCH, IS_JIT_MODE>::SettleSection(Section *section)
{
    bool isSectionAlloc {(section->header_.sh_flags & SHF_ALLOC) != 0};  // NOLINT(hicpp-signed-bitwise)
    if (IS_JIT_MODE || !isSectionAlloc) {
        ASSERT(currentSegment_ == nullptr);
    } else {
        ASSERT(currentSegment_ != nullptr && currentSegment_->header.p_type != PT_NULL);
    }

    section->header_.sh_size = section->GetDataSize();
    if (isSectionAlloc) {
        section->header_.sh_addr = UpdateAddress(section->header_.sh_addralign);
        if (section->header_.sh_type != SHT_NOBITS) {
            ASSERT(section->GetDataSize() != 0 || section->header_.sh_type == SHT_PROGBITS);
            section->header_.sh_offset = UpdateOffset(section->header_.sh_addralign);
            currentOffset_ += section->header_.sh_size;
        } else {
            section->header_.sh_offset = 0;
        }
        currentAddress_ += section->header_.sh_size;
    } else {
        section->header_.sh_offset = RoundUp(currentOffset_, section->header_.sh_addralign);
        currentOffset_ += section->header_.sh_size;
    }
}

template <Arch ARCH, bool IS_JIT_MODE>
void ElfBuilder<ARCH, IS_JIT_MODE>::Write(const std::string &fileName)
{
    std::vector<uint8_t> data(GetFileSize());
    auto dataSpan {Span(data)};
    Write(dataSpan);

    std::ofstream elfFile(fileName, std::ios::binary);
    elfFile.write(reinterpret_cast<char *>(dataSpan.Data()), dataSpan.Size());
}

inline void CopyToSpan(Span<uint8_t> to, const char *from, size_t size, size_t beginIndex)
{
    ASSERT(beginIndex < to.Size());
    auto maxSize {to.Size() - beginIndex};
    errno_t res = memcpy_s(&to[beginIndex], maxSize, from, size);
    if (res != 0) {
        UNREACHABLE();
    }
}

template <Arch ARCH, bool IS_JIT_MODE>
void ElfBuilder<ARCH, IS_JIT_MODE>::Write(Span<uint8_t> stream)
{
    ASSERT(!stream.Empty());
    char *header = reinterpret_cast<char *>(&header_);
    CopyToSpan(stream, header, sizeof(header_), 0);
    for (auto section : sections_) {
        if (auto dataProvider = section->GetDataProvider(); dataProvider != nullptr) {
            auto i = section->header_.sh_offset;
            dataProvider->FillData(stream, i);
        } else if (section->GetDataSize() && section->header_.sh_type != SHT_NOBITS) {
            auto i = section->header_.sh_offset;
            const char *data = reinterpret_cast<const char *>(section->GetData());
            CopyToSpan(stream, data, section->GetDataSize(), i);
        }
    }

    auto i = header_.e_shoff;
    for (auto section : sections_) {
        const char *data = reinterpret_cast<const char *>(&section->header_);
        CopyToSpan(stream, data, sizeof(section->header_), i);
        i += sizeof(section->header_);
    }

    i = header_.e_phoff;
    for (auto segment : segments_) {
        const char *data = reinterpret_cast<const char *>(&segment->header);
        CopyToSpan(stream, data, sizeof(segment->header), i);
        i += sizeof(segment->header);
    }
}

#ifdef PANDA_COMPILER_DEBUG_INFO
template <Arch ARCH>
class CfiGenerator {
public:
    explicit CfiGenerator(Span<DwarfSectionData> frameData) : frameData_(frameData) {}

    void GenerateDebugInfo()
    {
        if (!debugInfo_.empty() || frameData_.empty()) {
            return;
        }

        auto dw {Initialize()};
        auto cie {CreateCie(dw)};

        for (const auto &data : frameData_) {
            AddFde(dw, cie, data);
        }

        Finalize(dw);
    }

    Span<const uint8_t> GetDebugInfo() const
    {
        return Span(debugInfo_);
    }

private:
    enum DwarfSection {
        REL_DEBUG_FRAME = 0,
        DEBUG_FRAME = 1,
    };

    static inline int CreateSectionCallback(char *name, [[maybe_unused]] int size, [[maybe_unused]] Dwarf_Unsigned type,
                                            [[maybe_unused]] Dwarf_Unsigned flags, [[maybe_unused]] Dwarf_Unsigned link,
                                            [[maybe_unused]] Dwarf_Unsigned info,
                                            [[maybe_unused]] Dwarf_Unsigned *sectNameIndex,
                                            [[maybe_unused]] void *userData, [[maybe_unused]] int *error)
    {
        if (strcmp(name, ".rel.debug_frame") == 0) {
            return DwarfSection::REL_DEBUG_FRAME;
        }
        if (strcmp(name, ".debug_frame") == 0) {
            return DwarfSection::DEBUG_FRAME;
        }
        UNREACHABLE();
    }

    Dwarf_P_Debug Initialize() const
    {
        Dwarf_P_Debug dw {nullptr};
        Dwarf_Error error {nullptr};
        [[maybe_unused]] auto ret =  // NOLINTNEXTLINE(hicpp-signed-bitwise)
            dwarf_producer_init(DW_DLC_WRITE | DW_DLC_SIZE_64 | DW_DLC_SYMBOLIC_RELOCATIONS,
                                reinterpret_cast<Dwarf_Callback_Func>(CreateSectionCallback), nullptr, nullptr, nullptr,
                                GetIsaName(ARCH), "V2", nullptr, &dw, &error);

        ASSERT(error == DW_DLV_OK);
        return dw;
    }

    Span<Dwarf_Small> GetCieInitInstructions() const
    {
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static Dwarf_Small cieInitInstructionsArm[] = {
            DW_CFA_def_cfa,
            GetDwarfSP(ARCH),
            0U,
        };

        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static Dwarf_Small cieInitInstructionsAmd64[] = {
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            DW_CFA_def_cfa, GetDwarfSP(ARCH), PointerSize(ARCH), DW_CFA_offset | GetDwarfRIP(ARCH), 1U,
        };

        if (ARCH == Arch::AARCH32 || ARCH == Arch::AARCH64) {
            return Span(cieInitInstructionsArm, std::size(cieInitInstructionsArm));
        }
        if (ARCH == Arch::X86_64) {
            return Span(cieInitInstructionsAmd64, std::size(cieInitInstructionsAmd64));
        }
        UNREACHABLE();
    }

    Dwarf_Unsigned CreateCie(Dwarf_P_Debug dw) const
    {
        Dwarf_Error error {nullptr};
        auto cieInitInstructions {GetCieInitInstructions()};
        Dwarf_Unsigned cie =
            dwarf_add_frame_cie(dw, const_cast<char *>(""), static_cast<Dwarf_Small>(GetInstructionAlignment(ARCH)),
                                static_cast<Dwarf_Small>(-PointerSize(ARCH)), GetDwarfRIP(ARCH),
                                cieInitInstructions.data(), cieInitInstructions.SizeBytes(), &error);
        ASSERT(error == DW_DLV_OK);
        return cie;
    }

    void AddFde(Dwarf_P_Debug dw, Dwarf_Unsigned cie, const DwarfSectionData &data) const
    {
        Dwarf_Error error {nullptr};
        Dwarf_P_Fde fde = dwarf_new_fde(dw, &error);
        ASSERT(error == DW_DLV_OK);
        for (const auto &inst : data.GetFde()) {
            auto [op, par1, par2] = inst;
            dwarf_add_fde_inst(fde, op, par1, par2, &error);
            ASSERT(error == DW_DLV_OK);
        }

        dwarf_add_frame_fde(dw, fde, nullptr, cie, data.GetOffset(), data.GetSize(), DwarfSection::DEBUG_FRAME, &error);
        ASSERT(error == DW_DLV_OK);
    }

    void Finalize(Dwarf_P_Debug dw)
    {
        Dwarf_Error error {nullptr};
        auto sections = dwarf_transform_to_disk_form(dw, &error);
        ASSERT(error == DW_DLV_OK);

        ASSERT(debugInfo_.empty());
        for (decltype(sections) i {0}; i < sections; ++i) {
            Dwarf_Unsigned len = 0;
            Dwarf_Signed elfIdx = 0;
            auto bytes = reinterpret_cast<const uint8_t *>(
                dwarf_get_section_bytes(dw, DwarfSection::DEBUG_FRAME, &elfIdx, &len, &error));
            ASSERT(error == DW_DLV_OK);

            std::copy_n(bytes, len, std::back_inserter(debugInfo_));
        }

        constexpr size_t TERMINATOR_SIZE {4U};
        constexpr uint8_t TERMINATOR_VALUE {0U};
        std::fill_n(std::back_inserter(debugInfo_), TERMINATOR_SIZE, TERMINATOR_VALUE);

        dwarf_producer_finish(dw, &error);
        ASSERT(error == DW_DLV_OK);

        constexpr size_t CIE_ID_OFFSET {4U};
        // zero out CIE ID field
        *reinterpret_cast<uint32_t *>(&debugInfo_[CIE_ID_OFFSET]) = 0;

        constexpr size_t FDE_CIE_DISTANCE_OFFSET {4U};
        constexpr size_t FDE_LENGTH_SIZE {4U};
        size_t base {0};
        for (size_t i {0}; i < frameData_.size(); ++i) {
            // read FDE length field + 4 bytes (size of length field), get next FDE
            size_t fdeOffset {base + *reinterpret_cast<uint32_t *>(&debugInfo_[base]) + FDE_LENGTH_SIZE};
            // set distance to the parent CIE in FDE
            ASSERT(debugInfo_.size() > fdeOffset + FDE_CIE_DISTANCE_OFFSET + 3U);
            *reinterpret_cast<uint32_t *>(&debugInfo_[fdeOffset + FDE_CIE_DISTANCE_OFFSET]) =
                fdeOffset + FDE_CIE_DISTANCE_OFFSET;
            base = fdeOffset;
            ASSERT(debugInfo_.size() > base);
        }
    }

    Span<DwarfSectionData> frameData_;
    std::vector<uint8_t> debugInfo_;
};

template <Arch ARCH, bool IS_JIT_MODE>
void ElfBuilder<ARCH, IS_JIT_MODE>::FillFrameSection()
{
    // compute code offset for method
    for (auto &data : *frameData_) {
        data.SetOffset(textSection_.header_.sh_offset + data.GetOffset() + CodeInfo::GetCodeOffset(ARCH));
    }

    CfiGenerator<ARCH> cfiGen {Span(*frameData_)};
    cfiGen.GenerateDebugInfo();
    auto debugInfo {cfiGen.GetDebugInfo()};

    // Generate frame data
    GetFrameSection()->AppendData(debugInfo.data(), debugInfo.size());
}
#endif  // #ifdef PANDA_COMPILER_DEBUG_INFO

class ElfWriter {
public:
    void SetArch(Arch arch)
    {
        arch_ = arch;
    }
    Arch GetArch() const
    {
        return arch_;
    }

    void SetRuntime(RuntimeInterface *runtime)
    {
        runtime_ = runtime;
    }

    RuntimeInterface *GetRuntime()
    {
        return runtime_;
    }

    uintptr_t GetCurrentCodeAddress() const
    {
        return currentCodeSize_;
    }

    void StartClass(const Class &klass)
    {
        ClassHeader *classHeader = &classHeaders_.emplace_back();
        currentBitmap_ = &classMethodsBitmaps_.emplace_back();
        classHeader->classId = klass.GetFileId().GetOffset();
        classHeader->methodsOffset = methodHeaders_.size();
        currentBitmap_->resize(klass.GetMethods().size());
    }

    void EndClass()
    {
        ASSERT(!classHeaders_.empty());
        auto &classHeader = classHeaders_.back();
        classHeader.methodsCount = methodHeaders_.size() - classHeader.methodsOffset;
        if (classHeader.methodsCount != 0) {
            ASSERT(IsAligned<sizeof(uint32_t)>(currentBitmap_->GetContainerSizeInBytes()));
            classHeader.methodsBitmapOffset = bitmapSize_;
            classHeader.methodsBitmapSize = currentBitmap_->size();
            bitmapSize_ += currentBitmap_->GetContainerSize();
        } else {
            CHECK_EQ(classMethodsBitmaps_.size(), classHeaders_.size());
            classHeaders_.pop_back();
            classMethodsBitmaps_.pop_back();
        }
    }

    void AddMethod(const CompiledMethod &method)
    {
        if (method.GetMethod() == nullptr || method.GetCode().Empty()) {
            return;
        }
        methods_.push_back(method);
        auto &methodHeader = methodHeaders_.emplace_back();
        methodHeader.methodId = method.GetMethod()->GetFileId().GetOffset();
        methodHeader.codeOffset = currentCodeSize_;
        methodHeader.codeSize = method.GetOverallSize();
        currentCodeSize_ += methodHeader.codeSize;
        currentCodeSize_ = RoundUp(currentCodeSize_, GetCodeAlignment(arch_));
        currentBitmap_->SetBit(method.GetIndex());

#ifdef PANDA_COMPILER_DEBUG_INFO
        if (GetEmitDebugInfo()) {
            FillDebugInfo(method.GetCfiInfo(), methodHeader);
        }
#endif
    }

    void SetClassContext(const std::string &ctx)
    {
        classCtx_ = ctx;
    }

#ifdef PANDA_COMPILER_DEBUG_INFO
    void SetEmitDebugInfo([[maybe_unused]] bool emitDebugInfo)
    {
        emitDebugInfo_ = emitDebugInfo;
    }

    bool GetEmitDebugInfo() const
    {
        return emitDebugInfo_;
    }
#endif

    size_t AddString(const std::string &str)
    {
        auto pos = stringTable_.size();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        stringTable_.insert(stringTable_.end(), str.data(), str.data() + str.size() + 1);
        return pos;
    }

    template <Arch ARCH, bool IS_JIT_MODE>
    void GenerateSymbols(ElfBuilder<ARCH, IS_JIT_MODE> &builder);

#ifdef PANDA_COMPILER_DEBUG_INFO
    inline void PrepareOffsetsForDwarf(CfiOffsets *offsets) const;

    inline void FillPrologueInfo(DwarfSectionData *sectData, const CfiOffsets &cfiOffsets) const;

    inline void FillCalleesInfo(DwarfSectionData *sectData, const CfiInfo &cfiInfo) const;

    inline void FillEpilogInfo(DwarfSectionData *sectData, const CfiOffsets &cfiOffsets) const;

    inline void FillDebugInfo(CfiInfo cfiInfo, const compiler::MethodHeader &methodHeader);

    void AddFrameData(const DwarfSectionData &data)
    {
        frameData_.push_back(data);
    }

    std::vector<DwarfSectionData> *GetFrameData()
    {
        return &frameData_;
    }

#endif
protected:
    std::vector<CompiledMethod> methods_;                // NOLINT(misc-non-private-member-variables-in-classes)
    std::vector<compiler::MethodHeader> methodHeaders_;  // NOLINT(misc-non-private-member-variables-in-classes)
    Arch arch_ {Arch::NONE};                             // NOLINT(misc-non-private-member-variables-in-classes)
    std::vector<char> stringTable_;                      // NOLINT(misc-non-private-member-variables-in-classes)
    BitVector<> *currentBitmap_ {nullptr};               // NOLINT(misc-non-private-member-variables-in-classes)
    size_t currentCodeSize_ {0};                         // NOLINT(misc-non-private-member-variables-in-classes)

private:
    std::string classCtx_;

    std::vector<compiler::PandaFileHeader> fileHeaders_;
    std::vector<compiler::ClassHeader> classHeaders_;

    uint32_t gcType_ {static_cast<uint32_t>(mem::GCType::INVALID_GC)};

    std::vector<BitVector<>> classMethodsBitmaps_;
    static_assert(sizeof(BitVector<>::container_value_type) == sizeof(uint32_t));
    uint32_t bitmapSize_ {0};

    RuntimeInterface *runtime_ {nullptr};
#ifdef PANDA_COMPILER_DEBUG_INFO
    std::vector<DwarfSectionData> frameData_;
    bool emitDebugInfo_ {false};
#endif

    friend class CodeDataProvider;
    friend class JitCodeDataProvider;
    friend class AotBuilder;
    friend class JitDebugWriter;
};

#ifdef PANDA_COMPILER_DEBUG_INFO
static constexpr size_t LR_CFA_OFFSET {1U};
static constexpr size_t FP_CFA_OFFSET {2U};
static constexpr size_t DWARF_ARM_FP_REGS_START {64U};

void ElfWriter::PrepareOffsetsForDwarf(CfiOffsets *offsets) const
{
    // Make relative offsets
    offsets->popFplr -= offsets->popCallees;
    offsets->popCallees -= offsets->pushCallees;
    offsets->pushCallees -= offsets->setFp;
    offsets->setFp -= offsets->pushFplr;

    // Make offsets in alignment units
    auto instAlignment {GetInstructionAlignment(arch_)};

    offsets->pushFplr /= instAlignment;
    offsets->setFp /= instAlignment;
    offsets->pushCallees /= instAlignment;
    offsets->popCallees /= instAlignment;
    offsets->popFplr /= instAlignment;
}

void ElfWriter::FillPrologueInfo(DwarfSectionData *sectData, const CfiOffsets &cfiOffsets) const
{
    sectData->AddFdeInst(DW_CFA_advance_loc, cfiOffsets.pushFplr, 0);
    sectData->AddFdeInst(DW_CFA_def_cfa_offset, FP_CFA_OFFSET * PointerSize(arch_), 0);
    if (arch_ == Arch::AARCH32 || arch_ == Arch::AARCH64) {
        sectData->AddFdeInst(DW_CFA_offset, GetDwarfLR(arch_), LR_CFA_OFFSET);
    }
    sectData->AddFdeInst(DW_CFA_offset, GetDwarfFP(arch_), FP_CFA_OFFSET);

    sectData->AddFdeInst(DW_CFA_advance_loc, cfiOffsets.setFp, 0);
    sectData->AddFdeInst(DW_CFA_def_cfa_register, GetDwarfFP(arch_), 0);
}

void ElfWriter::FillCalleesInfo(DwarfSectionData *sectData, const CfiInfo &cfiInfo) const
{
    const auto &cfiOffsets {cfiInfo.offsets};

    sectData->AddFdeInst(DW_CFA_advance_loc, cfiOffsets.pushCallees, 0);

    const auto &callees {cfiInfo.calleeRegs};
    size_t calleeSlot {0};
    for (size_t i {0}; i < callees.size(); ++i) {
        auto reg {(callees.size() - 1U) - i};
        if (callees.test(reg)) {
#ifdef PANDA_COMPILER_TARGET_X86_64
            if (arch_ == Arch::X86_64) {
                reg = amd64::ConvertRegNumber(reg);
            }
#endif
            sectData->AddFdeInst(DW_CFA_offset, reg,
                                 FP_CFA_OFFSET + CFrameLayout::CALLEE_REGS_START_SLOT + calleeSlot++);
        }
    }

    const auto &vcallees {cfiInfo.calleeVregs};
    for (size_t i {0}; i < vcallees.size(); ++i) {
        auto vreg {(vcallees.size() - 1) - i};
        if (vcallees.test(vreg)) {
            ASSERT(arch_ == Arch::AARCH32 || arch_ == Arch::AARCH64);
            sectData->AddFdeInst(DW_CFA_offset, DWARF_ARM_FP_REGS_START + vreg,
                                 FP_CFA_OFFSET + CFrameLayout::CALLEE_REGS_START_SLOT + calleeSlot++);
        }
    }

    sectData->AddFdeInst(DW_CFA_advance_loc, cfiOffsets.popCallees, 0);
    for (size_t i {0}; i < callees.size(); ++i) {
        auto reg {(callees.size() - 1U) - i};
        if (callees.test(reg)) {
#ifdef PANDA_COMPILER_TARGET_X86_64
            if (arch_ == Arch::X86_64) {
                reg = amd64::ConvertRegNumber(reg);
            }
#endif
            sectData->AddFdeInst(DW_CFA_same_value, reg, 0);
        }
    }

    for (size_t i {0}; i < vcallees.size(); ++i) {
        auto vreg {(vcallees.size() - 1) - i};
        if (vcallees.test(vreg)) {
            ASSERT(arch_ == Arch::AARCH32 || arch_ == Arch::AARCH64);
            sectData->AddFdeInst(DW_CFA_same_value, DWARF_ARM_FP_REGS_START + vreg, 0);
        }
    }
}

void ElfWriter::FillEpilogInfo(DwarfSectionData *sectData, const CfiOffsets &cfiOffsets) const
{
    sectData->AddFdeInst(DW_CFA_advance_loc, cfiOffsets.popFplr, 0);
    sectData->AddFdeInst(DW_CFA_same_value, GetDwarfFP(arch_), 0);
    if (arch_ == Arch::AARCH32 || arch_ == Arch::AARCH64) {
        sectData->AddFdeInst(DW_CFA_same_value, GetDwarfLR(arch_), 0);
        sectData->AddFdeInst(DW_CFA_def_cfa, GetDwarfSP(arch_), 0);
    } else if (arch_ == Arch::X86_64) {
        sectData->AddFdeInst(DW_CFA_def_cfa, GetDwarfSP(arch_), 1U * PointerSize(arch_));
    } else {
        UNREACHABLE();
    }
}

void ElfWriter::FillDebugInfo(CfiInfo cfiInfo, const compiler::MethodHeader &methodHeader)
{
    DwarfSectionData sectData;
    // Will be patched later
    sectData.SetOffset(methodHeader.codeOffset);
    sectData.SetSize(methodHeader.codeSize);

    auto &cfiOffsets {cfiInfo.offsets};
    PrepareOffsetsForDwarf(&cfiOffsets);

    FillPrologueInfo(&sectData, cfiOffsets);
    FillCalleesInfo(&sectData, cfiInfo);
    FillEpilogInfo(&sectData, cfiOffsets);

    AddFrameData(sectData);
}
#endif

}  // namespace ark::compiler

#endif  // COMPILER_AOT_AOT_BULDER_ELF_BUILDER_H
