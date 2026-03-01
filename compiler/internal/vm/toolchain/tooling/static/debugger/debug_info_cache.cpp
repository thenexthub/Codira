/**
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "debug_info_cache.h"

#include "libarkfile/debug_info_extractor.h"
#include "include/tooling/pt_location.h"
#include "libarkbase/utils/bit_utils.h"
#include "libarkfile/method_data_accessor.h"
#include "libarkbase/os/mutex.h"

namespace ark::tooling::inspector {
void DebugInfoCache::AddPandaFile(const panda_file::File &file, bool isUserPandafile)
{
    os::memory::LockHolder lock(debugInfosMutex_);
    auto &debugInfo =
        debugInfos_
            .emplace(std::piecewise_construct, std::forward_as_tuple(&file),
                     std::forward_as_tuple(file,
                                           [this, &file](auto methodId, auto sourceName) {
                                               os::memory::LockHolder l(disassembliesMutex_);
                                               disassemblies_.emplace(std::piecewise_construct,
                                                                      std::forward_as_tuple(sourceName),
                                                                      std::forward_as_tuple(file, methodId));
                                           }))
            .first->second;
    debugInfo.SetUserFile(isUserPandafile);

    // For all methods add non-empty source code read from debug-info
    for (auto methodId : debugInfo.GetMethodIdList()) {
        std::string_view sourceRelativePath = debugInfo.GetSourceFile(methodId);
        std::string_view sourceCode = debugInfo.GetSourceCode(methodId);
        if (!sourceCode.empty()) {
            auto inserted = fileToSourceCode_.try_emplace(sourceRelativePath, sourceCode).second;
            if (!inserted) {
                LOG(WARNING, DEBUGGER) << "Duplicate source code in debug info for file \"" << sourceRelativePath
                                       << '"';
            }
        }
    }
}

void DebugInfoCache::GetSourceLocation(const PtFrame &frame, std::string_view &sourceFile, std::string_view &methodName,
                                       int32_t &lineNumber)
{
    auto method = frame.GetMethod();
    auto pandaFile = method->GetPandaFile();
    auto debugInfo = GetDebugInfo(pandaFile);
    if (debugInfo == nullptr || !debugInfo->IsUserFile()) {
        lineNumber = 1;
        return;
    }
    sourceFile = debugInfo->GetSourceFile(method->GetFileId());

    methodName = utf::Mutf8AsCString(method->GetName().data);

    // Line number entry corresponding to the bytecode location is
    // the last such entry for which the bytecode offset is not greater than
    // the given offset. Use `std::upper_bound` to find the first entry
    // for which the condition is not true, and then step back.
    auto &table = debugInfo->GetLineNumberTable(method->GetFileId());
    auto lineNumberIter = std::upper_bound(table.begin(), table.end(), frame.GetBytecodeOffset(),
                                           [](auto offset, auto &entry) { return offset < entry.offset; });
    ASSERT(lineNumberIter != table.begin());
    lineNumber = std::prev(lineNumberIter)->line;
}

std::unordered_set<PtLocation, HashLocation> DebugInfoCache::GetCurrentLineLocations(const PtFrame &frame)
{
    std::unordered_set<PtLocation, HashLocation> locations;

    auto method = frame.GetMethod();
    auto pandaFile = method->GetPandaFile();
    auto methodId = method->GetFileId();
    if (GetDebugInfo(pandaFile) == nullptr) {
        return locations;
    }
    auto &table = GetDebugInfo(pandaFile)->GetLineNumberTable(methodId);
    auto it = std::upper_bound(table.begin(), table.end(), frame.GetBytecodeOffset(),
                               [](auto offset, auto entry) { return offset < entry.offset; });
    if (it == table.begin()) {
        return locations;
    }
    auto lineNumber = (--it)->line;

    for (it = table.begin(); it != table.end(); ++it) {
        if (it->line != lineNumber) {
            continue;
        }

        auto next = it + 1;
        auto nextOffset = next != table.end() ? next->offset : method->GetCodeSize();
        for (auto o = it->offset; o < nextOffset; o++) {
            locations.emplace(pandaFile->GetFilename().c_str(), methodId, o);
        }
    }

    return locations;
}

std::unordered_set<PtLocation, HashLocation> DebugInfoCache::GetContinueToLocations(std::string_view sourceFile,
                                                                                    int32_t lineNumber)
{
    std::unordered_set<PtLocation, HashLocation> locations;
    EnumerateLineEntries(
        [](auto, auto &) { return true; },
        [sourceFile](auto, auto &debugInfo, auto methodId) { return debugInfo.GetSourceFile(methodId) == sourceFile; },
        [lineNumber, &locations](auto pandaFile, auto &, auto methodId, auto &entry, auto next) {
            if (entry.line != lineNumber) {
                // continue enumeration
                return true;
            }

            uint32_t nextOffset;
            if (next == nullptr) {
                panda_file::MethodDataAccessor mda(*pandaFile, methodId);
                if (auto codeId = mda.GetCodeId()) {
                    nextOffset = panda_file::CodeDataAccessor(*pandaFile, *codeId).GetCodeSize();
                } else {
                    nextOffset = 0;
                }
            } else {
                nextOffset = next->offset;
            }

            for (auto o = entry.offset; o < nextOffset; o++) {
                locations.emplace(pandaFile->GetFilename().data(), methodId, o);
            }
            return true;
        });
    return locations;
}

std::unordered_set<PtLocation, HashLocation> DebugInfoCache::GetBreakpointLocations(
    const std::function<bool(std::string_view)> &sourceFileFilter, int32_t lineNumber,
    std::set<std::string_view> &sourceFiles) const
{
    std::unordered_set<PtLocation, HashLocation> locations;
    sourceFiles.clear();
    // clang-format off
    EnumerateLineEntries(
        [](auto, auto &) { return true; },
        [&sourceFileFilter](auto, auto &debugInfo, auto methodId) {
            return sourceFileFilter(debugInfo.GetSourceFile(methodId));
        },
        [lineNumber, &sourceFiles, &locations](auto pandaFile, auto &debugInfo, auto methodId,
                                               auto &entry, auto /* next */) {
            if (entry.line == lineNumber) {
                sourceFiles.insert(debugInfo.GetSourceFile(methodId));
                locations.emplace(pandaFile->GetFilename().data(), methodId, entry.offset);
                // Must choose the first found bytecode location in each method
                return false;
            }
            // Continue search
            return true;
        });
    // clang-format on
    return locations;
}

std::set<int32_t> DebugInfoCache::GetValidLineNumbers(std::string_view sourceFile, int32_t startLine,
                                                      int32_t endLine, bool restrictToFunction)
{
    std::set<int32_t> lineNumbers;
    auto lineHandler = [startLine, endLine, &lineNumbers](auto, auto &, auto, auto &entry, auto /* next */) {
        if (entry.line >= startLine && entry.line < endLine) {
            lineNumbers.insert(entry.line);
        }

        return true;
    };
    if (!restrictToFunction) {
        EnumerateLineEntries([](auto, auto &) { return true; },
                             [sourceFile](auto, auto &debugInfo, auto methodId) {
                                 return (debugInfo.GetSourceFile(methodId) == sourceFile);
                             },
                             lineHandler);
        return lineNumbers;
    }

    auto methodFilter = [sourceFile, startLine](auto, auto &debugInfo, auto methodId) {
        if (debugInfo.GetSourceFile(methodId) != sourceFile) {
            return false;
        }

        bool hasLess = false;
        bool hasGreater = false;
        for (auto &entry : debugInfo.GetLineNumberTable(methodId)) {
            if (entry.line <= startLine) {
                hasLess = true;
            }

            if (entry.line >= startLine) {
                hasGreater = true;
            }

            if (hasLess && hasGreater) {
                break;
            }
        }

        return hasLess && hasGreater;
    };
    EnumerateLineEntries([](auto, auto &) { return true; }, methodFilter, lineHandler);
    return lineNumbers;
}

// NOLINTBEGIN(readability-magic-numbers)
static TypedValue CreateTypedValueFromReg(uint64_t reg, panda_file::Type::TypeId type)
{
    switch (type) {
        case panda_file::Type::TypeId::INVALID:
            return TypedValue::Invalid();
        case panda_file::Type::TypeId::VOID:
            return TypedValue::Void();
        case panda_file::Type::TypeId::U1:
            return TypedValue::U1(static_cast<bool>(ExtractBits(reg, 0U, 1U)));
        case panda_file::Type::TypeId::I8:
            return TypedValue::I8(static_cast<int8_t>(ExtractBits(reg, 0U, 8U)));
        case panda_file::Type::TypeId::U8:
            return TypedValue::U8(static_cast<uint8_t>(ExtractBits(reg, 0U, 8U)));
        case panda_file::Type::TypeId::I16:
            return TypedValue::I16(static_cast<int16_t>(ExtractBits(reg, 0U, 16U)));
        case panda_file::Type::TypeId::U16:
            return TypedValue::U16(static_cast<uint16_t>(ExtractBits(reg, 0U, 16U)));
        case panda_file::Type::TypeId::I32:
            return TypedValue::I32(static_cast<int32_t>(ExtractBits(reg, 0U, 32U)));
        case panda_file::Type::TypeId::U32:
            return TypedValue::U32(static_cast<uint32_t>(ExtractBits(reg, 0U, 32U)));
        case panda_file::Type::TypeId::F32:
            return TypedValue::F32(bit_cast<float>(static_cast<int32_t>(ExtractBits(reg, 0U, 32U))));
        case panda_file::Type::TypeId::F64:
            return TypedValue::F64(bit_cast<double>(reg));
        case panda_file::Type::TypeId::I64:
            return TypedValue::I64(reg);
        case panda_file::Type::TypeId::U64:
            return TypedValue::U64(reg);
        case panda_file::Type::TypeId::REFERENCE:
            return TypedValue::Reference(reinterpret_cast<ObjectHeader *>(reg));
        case panda_file::Type::TypeId::TAGGED:
            return TypedValue::Tagged(coretypes::TaggedValue(static_cast<coretypes::TaggedType>(reg)));
        default:
            UNREACHABLE();
            return TypedValue::Invalid();
    }
}
// NOLINTEND(readability-magic-numbers)

static panda_file::Type::TypeId GetTypeIdBySignature(char signature)
{
    switch (signature) {
        case 'V':
            return panda_file::Type::TypeId::VOID;
        case 'Z':
            return panda_file::Type::TypeId::U1;
        case 'B':
            return panda_file::Type::TypeId::I8;
        case 'H':
            return panda_file::Type::TypeId::U8;
        case 'S':
            return panda_file::Type::TypeId::I16;
        case 'C':
            return panda_file::Type::TypeId::U16;
        case 'I':
            return panda_file::Type::TypeId::I32;
        case 'U':
            return panda_file::Type::TypeId::U32;
        case 'F':
            return panda_file::Type::TypeId::F32;
        case 'D':
            return panda_file::Type::TypeId::F64;
        case 'J':
            return panda_file::Type::TypeId::I64;
        case 'Q':
            return panda_file::Type::TypeId::U64;
        case 'A':
            return panda_file::Type::TypeId::TAGGED;
        case 'L':
        case '[':
            return panda_file::Type::TypeId::REFERENCE;
        default:
            return panda_file::Type::TypeId::INVALID;
    }
}

std::map<std::string, TypedValue> DebugInfoCache::GetLocals(const PtFrame &frame)
{
    std::map<std::string, TypedValue> result;

    auto localHandler = [&result](const std::string &name, const std::string &signature, uint64_t reg,
                                  PtFrame::RegisterKind kind) {
        auto type = signature.empty() ? panda_file::Type::TypeId::INVALID : GetTypeIdBySignature(signature[0]);
        if (type == panda_file::Type::TypeId::INVALID) {
            switch (kind) {
                case PtFrame::RegisterKind::PRIMITIVE:
                    type = panda_file::Type::TypeId::U64;
                    break;
                case PtFrame::RegisterKind::REFERENCE:
                    type = panda_file::Type::TypeId::REFERENCE;
                    break;
                case PtFrame::RegisterKind::TAGGED:
                    type = panda_file::Type::TypeId::TAGGED;
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        }

        result.emplace(name, CreateTypedValueFromReg(reg, type));
    };

    auto method = frame.GetMethod();
    auto methodId = method->GetFileId();
    auto debugInfo = GetDebugInfo(method->GetPandaFile());
    if (debugInfo == nullptr) {
        return result;
    }
    auto &parameters = debugInfo->GetParameterInfo(methodId);
    for (auto i = 0U; i < parameters.size(); i++) {
        auto &parameter = parameters[i];
        localHandler(parameter.name, parameter.signature, frame.GetArgument(i), frame.GetArgumentKind(i));
    }

    auto &variables = debugInfo->GetLocalVariableTable(methodId);
    auto frameOffset = frame.GetBytecodeOffset();
    for (auto &variable : variables) {
        if (variable.IsAccessibleAt(frameOffset)) {
            localHandler(variable.name, variable.typeSignature,
                         // We introduced a hack in DisasmBackedDebugInfoExtractor, assigning -1 to Accumulator
                         variable.regNumber == -1 ? frame.GetAccumulator() : frame.GetVReg(variable.regNumber),
                         variable.regNumber == -1 ? frame.GetAccumulatorKind() : frame.GetVRegKind(variable.regNumber));
        }
    }

    return result;
}

std::string DebugInfoCache::GetSourceCode(std::string_view sourceFile)
{
    {
        os::memory::LockHolder lock(disassembliesMutex_);

        auto it = disassemblies_.find(sourceFile);
        if (it != disassemblies_.end()) {
            auto* debugInfo = GetDebugInfo(&it->second.first);
            if (debugInfo != nullptr) {
                return debugInfo->GetSourceCode(it->second.second);
            }
        }
    }

    // Try to get source code read from debug info
    {
        os::memory::LockHolder lock(debugInfosMutex_);

        auto iter = fileToSourceCode_.find(sourceFile);
        if (iter != fileToSourceCode_.end()) {
            return std::string(iter->second);
        }
    }

    if (!os::file::File::IsRegularFile(sourceFile.data())) {
        return {};
    }

    std::string result;

    std::stringstream buffer;
    buffer << std::ifstream(sourceFile.data()).rdbuf();

    result = buffer.str();
    if (!result.empty() && result.back() != '\n') {
        result += "\n";
    }

    return result;
}

std::vector<const panda_file::File*> DebugInfoCache::GetPandaFiles(
    const std::function<bool(std::string_view)> &sourceFileFilter)
{
    std::unordered_set<const panda_file::File*> uniquePandaFiles;
    // clang-format off
    EnumerateLineEntries(
        [](auto, auto &) { return true; },
        [&sourceFileFilter](auto, auto &debugInfo, auto methodId) {
            return sourceFileFilter(debugInfo.GetSourceFile(methodId));
        },
        [&uniquePandaFiles](const panda_file::File* pf, auto &, auto, auto &, auto) {
            uniquePandaFiles.insert(pf);
            return false;
        });
    // clang-format on
    std::vector<const panda_file::File*> pandaFiles;
    pandaFiles.reserve(uniquePandaFiles.size());
    pandaFiles.assign(uniquePandaFiles.begin(), uniquePandaFiles.end());
    return pandaFiles;
}

const panda_file::DebugInfoExtractor *DebugInfoCache::GetDebugInfo(const panda_file::File *file) const
{
    os::memory::LockHolder lock(debugInfosMutex_);
    auto it = debugInfos_.find(file);
    if (it == debugInfos_.end()) {
        return nullptr;
    }
    return &it->second;
}

const char *DebugInfoCache::GetSourceFile(Method *method)
{
    auto pandaFile = method->GetPandaFile();
    auto debugInfo = GetDebugInfo(pandaFile);
    if (debugInfo == nullptr) {
        return nullptr;
    }
    return debugInfo->GetSourceFile(method->GetFileId());
}

const char *DebugInfoCache::GetUserSourceFile(Method *method)
{
    auto pandaFile = method->GetPandaFile();
    auto debugInfo = GetDebugInfo(pandaFile);
    if ((debugInfo == nullptr) || !debugInfo->IsUserFile()) {
        return nullptr;
    }
    return debugInfo->GetSourceFile(method->GetFileId());
}

}  // namespace ark::tooling::inspector
