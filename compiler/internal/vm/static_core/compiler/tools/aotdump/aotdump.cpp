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

#include "aot_manager.h"
#include "aotdump_options.h"
#include "libarkfile/class_data_accessor.h"
#include "libarkfile/file.h"
#include "libarkfile/file-inl.h"
#include "libarkbase/mem/arena_allocator.h"
#include "mem/gc/gc_types.h"
#include "libarkbase/mem/pool_manager.h"
#include "libarkfile/method_data_accessor.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "libarkfile/proto_data_accessor.h"
#include "libarkfile/proto_data_accessor-inl.h"
#include "runtime/include/class_helper.h"
#include "libarkbase/utils/arch.h"
#include "libarkbase/utils/bit_memory_region-inl.h"

#ifdef PANDA_COMPILER_TARGET_AARCH64
#include "aarch64/disasm-aarch64.h"
using vixl::aarch64::Decoder;
using vixl::aarch64::Disassembler;
using vixl::aarch64::Instruction;
#endif  // PANDA_COMPILER_TARGET_AARCH64
#ifdef PANDA_COMPILER_TARGET_X86_64
#include "Zydis/Zydis.h"
#endif  // PANDA_COMPILER_TARGET_X86_64

#include <fstream>
#include <iomanip>
#include <elf.h>
#include <regex>

using namespace ark::compiler;  // NOLINT(*-build-using-namespace)

namespace ark::aotdump {

class TypePrinter {
public:
    explicit TypePrinter(panda_file::ProtoDataAccessor &pda, std::ostream &out) : pda_(pda), out_(out) {}
    NO_COPY_SEMANTIC(TypePrinter);
    NO_MOVE_SEMANTIC(TypePrinter);
    ~TypePrinter() = default;

    void Dump(panda_file::Type type)
    {
        if (!type.IsReference()) {
            out_ << type;
        } else {
            out_ << ClassHelper::GetName(pda_.GetPandaFile().GetStringData(pda_.GetReferenceType(refIdx_++)).data);
        }
    }

private:
    panda_file::ProtoDataAccessor &pda_;
    std::ostream &out_;
    uint32_t refIdx_ {0};
};

class PandaFileHelper {
public:
    explicit PandaFileHelper(const char *fileName) : file_(panda_file::OpenPandaFile(fileName)) {}
    NO_COPY_SEMANTIC(PandaFileHelper);
    NO_MOVE_SEMANTIC(PandaFileHelper);
    ~PandaFileHelper() = default;

    std::string GetMethodName(uint32_t id) const
    {
        if (!file_) {
            return "-";
        }
        auto fileId = panda_file::File::EntityId(id);

        panda_file::MethodDataAccessor mda(*file_, fileId);
        panda_file::ProtoDataAccessor pda(*file_, panda_file::MethodDataAccessor::GetProtoId(*file_, fileId));
        std::ostringstream ss;
        TypePrinter typePrinter(pda, ss);

        typePrinter.Dump(pda.GetReturnType());
        ss << ' ';

        auto className = ClassHelper::GetName(file_->GetStringData(mda.GetClassId()).data);
        ss << className << "::" << file_->GetStringData(mda.GetNameId()).data;

        ss << '(';
        bool firstArg = true;
        // inject class name as the first argument of non static method for consitency with ark::Method::GetFullName
        if (!mda.IsStatic()) {
            firstArg = false;
            ss << className;
        }
        for (uint32_t argIdx = 0; argIdx < pda.GetNumArgs(); ++argIdx) {
            if (!firstArg) {
                ss << ", ";
            }
            firstArg = false;
            typePrinter.Dump(pda.GetArgType(argIdx));
        }
        ss << ')';
        return ss.str();
    }

    std::string GetClassName(uint32_t id) const
    {
        if (!file_) {
            return "-";
        }
        return ClassHelper::GetName(file_->GetStringData(panda_file::File::EntityId(id)).data);
    }

    const std::string &GetFilename() const
    {
        return file_->GetFilename();
    }

private:
    std::unique_ptr<const panda_file::File> file_;
};

class AotDump {
public:
    explicit AotDump(ark::ArenaAllocator *allocator) : allocator_(allocator) {}
    NO_COPY_SEMANTIC(AotDump);
    NO_MOVE_SEMANTIC(AotDump);
    ~AotDump() = default;

    void SetOptions(ark::aotdump::Options *options)
    {
        options_ = options;
    }

    std::string GetFilePathFromContext(size_t index) const
    {
        std::string_view context = aotFile_->GetClassContext();
        if (context.empty()) {
            return "";
        }

        size_t start = 0;
        size_t end = context.find(AotClassContextCollector::DELIMETER, start);
        size_t currentIndex = 1;

        while (end != std::string::npos) {
            if (currentIndex == index) {
                auto fileContext = context.substr(start, end - start);
                size_t hashPos = fileContext.find(AotClassContextCollector::HASH_DELIMETER);
                if (hashPos == std::string::npos) {
                    return "";
                }
                return std::string(fileContext.substr(0, hashPos));
            }
            start = end + 1;
            end = context.find(AotClassContextCollector::DELIMETER, start);
            currentIndex++;
        }

        // Check last file
        if (currentIndex == index) {
            auto fileContext = context.substr(start);
            size_t hashPos = fileContext.find(AotClassContextCollector::HASH_DELIMETER);
            if (hashPos == std::string::npos) {
                return "";
            }
            return std::string(fileContext.substr(0, hashPos));
        }

        return "";
    }

    int Run(int argc, const char *argv[])  // NOLINT(modernize-avoid-c-arrays)
    {
        ark::PandArgParser paParser;
        PandArg<std::string> inputFile {"input_file", "", "Input file path"};
        paParser.EnableTail();
        paParser.PushBackTail(&inputFile);
        std::array<char, NAME_MAX> tmpfileBuf {"/tmp/fixed_aot_XXXXXX"};
        // Remove temporary file at the function exit
        auto finalizer = [](const char *fileName) {
            if (fileName != nullptr) {
                remove(fileName);
            }
        };
        std::unique_ptr<const char, decltype(finalizer)> tempFileRemover(nullptr, finalizer);

        options_->AddOptions(&paParser);

        if (!paParser.Parse(argc, argv)) {
            std::cerr << "Parse options failed: " << paParser.GetErrorString() << std::endl;
            return -1;
        }
        if (inputFile.GetValue().empty()) {
            std::cerr << "Please specify input file\n";
            return -1;
        }
        Expected<std::unique_ptr<AotFile>, std::string> aotRes;
        // Fix elf header for cross platform files. Cross opening is available only in X86_64 arch.
        if (RUNTIME_ARCH == Arch::X86_64) {
            if (!FixElfHeader(tmpfileBuf, inputFile)) {
                return -1;
            }
            tempFileRemover.reset(tmpfileBuf.data());
            aotRes = AotFile::Open(tmpfileBuf.data(), 0, true);
        } else {
            aotRes = AotFile::Open(inputFile.GetValue(), 0, true);
        }
        if (!aotRes) {
            std::cerr << "Open AOT file failed: " << aotRes.Error() << std::endl;
            return -1;
        }
        aotFile_ = std::move(aotRes.Value());
        DumpAll();
        return 0;
    }

    void DumpAll()
    {
        std::ofstream outFstream;
        if (options_->WasSetOutputFile()) {
            outFstream.open(options_->GetOutputFile());
            stream_ = &outFstream;
        } else {
            stream_ = &std::cerr;
        }
        DumpHeader();
        DumpFiles();
        stream_->flush();
        aotFile_.reset();
    }

    bool FixElfHeader(std::array<char, NAME_MAX> &tmpfileBuf, PandArg<std::string> &inputFile)
    {
        int fd = mkstemp(tmpfileBuf.data());
        if (fd == -1) {
            std::cerr << "Failed to open temporary file\n";
            return false;
        }
        close(fd);
        std::ofstream ostm(tmpfileBuf.data(), std::ios::binary);
        std::ifstream istm(inputFile.GetValue(), std::ios::binary);
        if (!ostm.is_open() || !istm.is_open()) {
            std::cerr << "Cannot open tmpfile or input file\n";
            return false;
        }
        std::vector<char> buffer(std::istreambuf_iterator<char>(istm), {});
        auto *header = reinterpret_cast<Elf64_Ehdr *>(buffer.data());
        header->e_machine = EM_X86_64;
        ostm.write(buffer.data(), buffer.size());
        return true;
    }

    void DumpHeader()
    {
        auto aotHeader = aotFile_->GetAotHeader();
        (*stream_) << "header:" << std::endl;
        (*stream_) << "  magic: " << aotHeader->magic.data() << std::endl;
        (*stream_) << "  version: " << aotHeader->version.data() << std::endl;
        (*stream_) << "  filename: " << aotFile_->GetFileName() << std::endl;
        (*stream_) << "  cmdline: " << aotFile_->GetCommandLine() << std::endl;
        (*stream_) << "  checksum: " << aotHeader->checksum << std::endl;
        (*stream_) << "  env checksum: " << aotHeader->environmentChecksum << std::endl;
        (*stream_) << "  arch: " << GetArchString(static_cast<Arch>(aotHeader->arch)) << std::endl;
        (*stream_) << "  gc_type: " << mem::GCStringFromType(static_cast<mem::GCType>(aotHeader->gcType)) << std::endl;
        (*stream_) << "  files_count: " << aotHeader->filesCount << std::endl;
        (*stream_) << "  files_offset: " << aotHeader->filesOffset << std::endl;
        (*stream_) << "  classes_offset: " << aotHeader->classesOffset << std::endl;
        (*stream_) << "  methods_offset: " << aotHeader->methodsOffset << std::endl;
        (*stream_) << "  bitmap_offset: " << aotHeader->bitmapOffset << std::endl;
        (*stream_) << "  strtab_offset: " << aotHeader->strtabOffset << std::endl;
    }

    void DumpClassHashTable(ark::compiler::AotPandaFile &aotPandaFile, PandaFileHelper &pfile)
    {
        (*stream_) << "  class_hash_table:" << std::endl;
        constexpr int ALIGN_SIZE = 32;
        (*stream_) << std::left << std::setfill(' ') << std::setw(ALIGN_SIZE) << "i" << std::left << std::setfill(' ')
                   << std::setw(ALIGN_SIZE) << "next_pos";
        (*stream_) << std::left << std::setfill(' ') << std::setw(ALIGN_SIZE) << "entity_id_offset" << std::left
                   << std::setfill(' ') << std::setw(ALIGN_SIZE) << "descriptor" << std::endl;
        auto classHashTable = aotPandaFile.GetClassHashTable();
        auto hashTableSize = classHashTable.size();
        for (size_t i = 0; i < hashTableSize; i++) {
            auto entityPair = classHashTable[i];
            if (entityPair.descriptorHash != 0) {
                auto descriptor = pfile.GetClassName(entityPair.entityIdOffset);
                (*stream_) << std::left << std::setfill(' ') << std::setw(ALIGN_SIZE) << (i + 1);
                (*stream_) << std::left << std::setfill(' ') << std::dec << std::setw(ALIGN_SIZE) << entityPair.nextPos;
                (*stream_) << std::left << std::setfill(' ') << std::dec << std::setw(ALIGN_SIZE)
                           << entityPair.entityIdOffset;
                (*stream_) << std::left << std::setfill(' ') << std::setw(ALIGN_SIZE) << descriptor << std::endl;
            } else {
                (*stream_) << std::left << std::setfill(' ') << std::setw(ALIGN_SIZE) << (i + 1) << std::endl;
            }
        }
    }

    void DumpFiles()
    {
        (*stream_) << "files:" << std::endl;
        for (decltype(auto) fileHeader : aotFile_->FileHeaders()) {
            AotPandaFile aotPandaFile(aotFile_.get(), &fileHeader);
            auto fileName = aotFile_->GetString(fileHeader.fileNameStr);
            PandaFileHelper pfile(fileName);
            (*stream_) << "- name: " << fileName << std::endl;
            (*stream_) << "  classes:\n";
            for (decltype(auto) classHeader : aotPandaFile.GetClassHeaders()) {
                AotClass klass(aotFile_.get(), &classHeader);
                (*stream_) << "  - class_id: " << classHeader.classId << std::endl;
                (*stream_) << "    name: " << pfile.GetClassName(classHeader.classId) << std::endl;
                auto methodsBitmap = klass.GetBitmap();
                BitMemoryRegion rgn(methodsBitmap.data(), methodsBitmap.size());
                (*stream_) << "    methods_bitmap: " << rgn << std::endl;
                (*stream_) << "    methods:\n";
                for (decltype(auto) methodHeader : klass.GetMethodHeaders()) {
                    DumpMethodHeader(pfile, aotPandaFile, methodHeader);
                }
            }
            DumpClassHashTable(aotPandaFile, pfile);
        }
    }

    void DumpMethodHeader(const PandaFileHelper &pfile, const AotPandaFile &aotPandaFile,
                          const MethodHeader &methodHeader)
    {
        auto methodName = pfile.GetMethodName(methodHeader.methodId);
        if (options_->WasSetMethodRegex() && !methodName.empty()) {
            std::regex rgx(options_->GetMethodRegex());
            if (!std::regex_match(methodName, rgx)) {
                return;
            }
        }
        (*stream_) << "    - id: " << std::dec << methodHeader.methodId << std::endl;
        (*stream_) << "      name: " << methodName << std::endl;
        (*stream_) << "      code_offset: 0x" << std::hex << methodHeader.codeOffset << std::dec << std::endl;
        (*stream_) << "      code_size: " << methodHeader.codeSize << std::endl;
        auto codeInfo = aotPandaFile.GetMethodCodeInfo(&methodHeader);
        if (options_->GetShowCode() == "disasm") {
            (*stream_) << "      code: |\n";
            PrintCode("        ", codeInfo, pfile);
        }
    }

    void PrintCode(const char *prefix, const CodeInfo &codeInfo, const PandaFileHelper &pfile) const
    {
        Arch arch = static_cast<Arch>(aotFile_->GetAotHeader()->arch);
        switch (arch) {
#ifdef PANDA_COMPILER_TARGET_AARCH64
            case Arch::AARCH64:
                return PrintCodeArm64(prefix, codeInfo, pfile);
#endif  // PANDA_COMPILER_TARGET_AARCH64
#ifdef PANDA_COMPILER_TARGET_X86_64
            case Arch::X86_64:
                return PrintCodeX8664(prefix, codeInfo, pfile);
#endif  // PANDA_COMPILER_TARGET_X86_64
            default:
                (*stream_) << prefix << "Unsupported target arch: " << GetArchString(arch) << std::endl;
                break;
        }
    }

#ifdef PANDA_COMPILER_TARGET_AARCH64
    void PrintCodeArm64(const char *prefix, const CodeInfo &codeInfo, const PandaFileHelper &pfile) const
    {
        Span<const uint8_t> code = codeInfo.GetCodeSpan();

        Decoder decoder(allocator_);
        Disassembler disasm(allocator_);
        decoder.AppendVisitor(&disasm);
        auto startInstr = reinterpret_cast<const Instruction *>(code.data());
        auto endInstr = reinterpret_cast<const Instruction *>(code.end());
        uint32_t pc = 0;

        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (auto instr = startInstr; instr < endInstr; instr += vixl::aarch64::kInstructionSize) {
            auto stackmap = codeInfo.FindStackMapForNativePc(pc, Arch::AARCH64);
            if (stackmap.IsValid()) {
                PrintStackmap(prefix, codeInfo, stackmap, Arch::AARCH64, pfile);
            }
            decoder.Decode(instr);
            (*stream_) << prefix << std::hex << std::setw(8U) << std::setfill('0') << instr - startInstr << ": "
                       << disasm.GetOutput() << std::endl;
            pc += vixl::aarch64::kInstructionSize;
        }
        (*stream_) << std::dec;
    }
#endif  // PANDA_COMPILER_TARGET_AARCH64
#ifdef PANDA_COMPILER_TARGET_X86_64
    void PrintCodeX8664(const char *prefix, const CodeInfo &codeInfo, const PandaFileHelper &pfile) const
    {
        Span<const uint8_t> code = codeInfo.GetCodeSpan();
        constexpr size_t LENGTH = ZYDIS_MAX_INSTRUCTION_LENGTH;  // 15 bytes is max inst length in amd64
        size_t codeSize = code.size();

        // Initialize decoder context
        ZydisDecoder decoder;
        if (!ZYAN_SUCCESS(ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64))) {
            LOG(FATAL, AOT) << "ZydisDecoderInit failed";
        }

        // Initialize formatter
        ZydisFormatter formatter;
        if (!ZYAN_SUCCESS(ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL))) {
            LOG(FATAL, AOT) << "ZydisFormatterInit failed";
        }

        for (size_t pos = 0; pos < codeSize;) {
            ZydisDecodedInstruction instruction;
            constexpr auto BUF_SIZE = 256;
            std::array<char, BUF_SIZE> buffer;  // NOLINT(cppcoreguidelines-pro-type-member-init)
            auto len = std::min(LENGTH, codeSize - pos);
            if (!ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&decoder, &code[pos], len, &instruction))) {
                LOG(FATAL, AOT) << "ZydisDecoderDecodeBuffer failed";
            }
            if (!ZYAN_SUCCESS(ZydisFormatterFormatInstruction(&formatter, &instruction, buffer.data(), buffer.size(),
                                                              uintptr_t(&code[pos])))) {
                LOG(FATAL, AOT) << "ZydisFormatterFormatInstruction failed";
            }
            auto stackmap = codeInfo.FindStackMapForNativePc(pos, Arch::X86_64);
            if (stackmap.IsValid()) {
                PrintStackmap(prefix, codeInfo, stackmap, Arch::X86_64, pfile);
            }
            (*stream_) << prefix << std::hex << std::setw(8U) << std::setfill('0') << pos << ": " << buffer.data()
                       << std::endl;
            pos += instruction.length;
        }
    }
#endif  // PANDA_COMPILER_TARGET_X86_64

    void PrintStackmap(const char *prefix, const CodeInfo &codeInfo, const StackMap &stackmap, Arch arch,
                       const PandaFileHelper &pfile) const
    {
        (*stream_) << prefix << "          ";
        codeInfo.Dump(*stream_, stackmap, arch);
        (*stream_) << std::endl;
        if (!stackmap.HasInlineInfoIndex()) {
            return;
        }
        for (auto ii : const_cast<CodeInfo &>(codeInfo).GetInlineInfos(stackmap)) {
            (*stream_) << prefix << "          ";
            codeInfo.DumpInlineInfo(*stream_, stackmap, ii.GetRow() - stackmap.GetInlineInfoIndex());
            auto id = const_cast<CodeInfo &>(codeInfo).GetMethod(stackmap, ii.GetRow() - stackmap.GetInlineInfoIndex());
            std::string methodName;

            auto i = ii.GetFileIndex();
            if (i > 0) {
                auto filePath = GetFilePathFromContext(i);
                if (pfile.GetFilename() != filePath) {
                    methodName = PandaFileHelper(filePath.c_str()).GetMethodName(std::get<uint32_t>(id));
                }
            }
            if (methodName.empty()) {
                methodName = pfile.GetMethodName(std::get<uint32_t>(id));
            }

            (*stream_) << ", method: " << methodName;
            (*stream_) << std::endl;
        }
    }

private:
    [[maybe_unused]] ark::ArenaAllocator *allocator_;
    std::ostream *stream_ {};
    ark::aotdump::Options *options_ {};
    std::unique_ptr<AotFile> aotFile_ {};
};

}  // namespace ark::aotdump

int main(int argc, const char *argv[])
{
    // NOLINTBEGIN(readability-magic-numbers)
    ark::mem::MemConfig::Initialize(ark::operator""_MB(64ULL), ark::operator""_MB(64ULL), ark::operator""_MB(64ULL),
                                    ark::operator""_MB(32ULL), 0, 0);
    // NOLINTEND(readability-magic-numbers)
    ark::PoolManager::Initialize();
    auto allocator = new ark::ArenaAllocator(ark::SpaceType::SPACE_TYPE_COMPILER);
    // Initialize options before Run() - options lifetime covers entire main()
    ark::Span<const char *> sp(argv, argc);
    ark::aotdump::Options options(sp[0]);
    ark::aotdump::AotDump aotdump(allocator);
    aotdump.SetOptions(&options);

    auto result = aotdump.Run(argc, argv);

    delete allocator;
    ark::PoolManager::Finalize();
    ark::mem::MemConfig::Finalize();
    return result;
}
