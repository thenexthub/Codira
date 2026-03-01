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

/**
 * @file
 *
 * This file implements the SourceManager related classes.
 */

#include "Codira/Basic/SourceManager.h"
#include "Codira/Utils/CheckUtils.h"
#include "Codira/Utils/FileUtil.h"
#include "Codira/Utils/SafePointer.h"

using namespace Codira;

size_t Source::PosToOffset(const Position& pos) const
{
    if (pos.line > static_cast<int>(lineOffsets.size())) {
        return buffer.length();
    }
    if (pos.line < 1 || pos.column < 1) {
        return 0;
    }
    size_t index = static_cast<size_t>(pos.line) - 1;
    auto pStart = buffer.data() + lineOffsets[index];
    auto pEnd = buffer.data() + buffer.length();
    size_t columnOffset = 0;
    while (Utils::GetLineTerminatorLength(pStart + columnOffset, pEnd) == 0 &&
        columnOffset < (static_cast<size_t>(pos.column) - 1) && pStart + columnOffset < pEnd) {
        columnOffset++;
    }
    if (columnOffset < (static_cast<size_t>(pos.column) - 1) && pStart + columnOffset < pEnd) {
        // there's line terminator before `pos.column`
        columnOffset += Utils::GetLineTerminatorLength(pStart + columnOffset, pEnd);
    }

    return std::min(lineOffsets[index] + columnOffset, buffer.length());
}

Source::Source(unsigned int fileID, std::string path, std::string buffer, uint64_t fileHash,
    const std::optional<std::string>& packageName)
    : fileID(fileID), path(std::move(path)), buffer(std::move(buffer)), fileHash(fileHash), packageName(packageName)
{
    if (this->buffer.empty()) {
        return;
    }

    // build lineOffsets
    auto pStart = this->buffer.data();
    size_t length = this->buffer.length();
    auto pEnd = this->buffer.data() + length;
    for (auto ptr = pStart; ptr < pEnd;) {
        while (Utils::GetLineTerminatorLength(ptr, pEnd) == 0 && ptr < pEnd) {
            ptr++;
        }
        if (ptr >= pEnd) {
            break;
        }
        ptr += Utils::GetLineTerminatorLength(ptr, pEnd);
        lineOffsets.emplace_back(ptr - pStart);
    }
}

void SourceManager::SaveSourceFile(
    unsigned int fileID,
    std::string normalizedPath,
    std::string buffer,
    uint64_t fileHash,
    std::optional<std::string> packageName)
{
    sources.emplace_back(fileID, normalizedPath, buffer, fileHash, packageName);
    filePathToFileIDMap.emplace(normalizedPath, fileID);
}

void SourceManager::ReserveCommonPartSources(std::vector<std::string> files)
{
    for (size_t i = 0; i < files.size(); i++) {
        auto file = files.at(i);
        uint64_t fileHash = 0;
        SaveSourceFile(static_cast<unsigned int>(i + 1), files.at(i), "", fileHash);
        filePathToFileIDMap.emplace(file, i + 1);
    }
}

unsigned int SourceManager::AddSource(
    const std::string& path, const std::string& buffer, std::optional<std::string> packageName)
{
    // path canonicalize
    std::string normalizePath = FileUtil::Normalize(path);
    // Change fileHash from content hash to path hash.
    uint64_t fileHash = Utils::GetHash(normalizePath);
    auto existed = filePathToFileIDMap.find(normalizePath);
    if (existed != filePathToFileIDMap.end()) {
        unsigned int fileID = static_cast<unsigned int>(existed->second);
        CODEC_ASSERT(static_cast<size_t>(fileID) < sources.size());
        sources[fileID] = Source{fileID, normalizePath, buffer, fileHash, packageName};
        return fileID;
    } else {
        auto fileID = static_cast<unsigned int>(sources.size());
        SaveSourceFile(fileID, normalizePath, buffer, fileHash, packageName);
        return fileID;
    }
}

unsigned int SourceManager::AppendSource(const std::string& path, const std::string& buffer)
{
    // path canonicalize
    std::string normalizePath = FileUtil::Normalize(path);
    uint64_t fileHash = Utils::GetHash(normalizePath);
    auto existed = filePathToFileIDMap.find(normalizePath);
    if (existed != filePathToFileIDMap.end()) {
        unsigned int fileID = static_cast<unsigned int>(existed->second);
        CODEC_ASSERT(static_cast<size_t>(fileID) < sources.size());
        auto newBuffer = sources[fileID].buffer + buffer;
        sources[fileID] = Source{fileID, normalizePath, newBuffer, fileHash};
        return fileID;
    } else {
        auto fileID = static_cast<unsigned int>(sources.size());
        sources.emplace_back(fileID, normalizePath, buffer, fileHash);
        filePathToFileIDMap.emplace(normalizePath, fileID);
        return fileID;
    }
}

bool SourceManager::IsSourceFileExist(const unsigned int id)
{
    // Check whether the *.macrocall exists or not.
    if (id < sources.size()) {
        auto path = sources[id].path;
        if (!path.empty() && FileUtil::GetFileExtension(path) != "code") {
            return FileUtil::FileExist(path);
        }
    }
    return true;
}

int SourceManager::GetLineEnd(const Position& pos)
{
    if (pos.fileID >= sources.size()) {
        return 0;
    }
    auto buffer = sources[pos.fileID].buffer;
    auto sourceSplited = Utils::SplitLines(buffer);
    if (pos.line > static_cast<int>(sourceSplited.size())) {
        return 0;
    }
    CODEC_ASSERT(pos.line > 0);
    if (pos.line <= 0) {
        return 0;
    }
    return static_cast<int>(sourceSplited[static_cast<size_t>(pos.line - 1)].size());
}

std::string SourceManager::GetContentBetween(
    const Position& begin, const Position& end, const std::string& importGenericContent) const
{
    return GetContentBetween(begin.fileID, begin, end, importGenericContent);
}

std::string SourceManager::GetContentBetween(
    unsigned int fileID, const Position& begin, const Position& end, const std::string& importGenericContent) const
{
    if (fileID == 0 || begin <= INVALID_POSITION || end <= INVALID_POSITION || end < begin) {
        return "";
    }

    CODEC_ASSERT(INVALID_POSITION < begin && begin <= end);
    CODEC_ASSERT(!sources.empty());
    auto& sourceWithFileID = fileID >= sources.size() ? sources[0] : sources[fileID];

    // Use OwnedPtr for temporary Source to avoid mixed return types in ternary operator (? tempObj : ref).
    // This helps compiler optimization by having consistent pointer types
    OwnedPtr<Source> tempSource;
    Ptr<const Source> sourcePtr;

    if (sourceWithFileID.buffer.empty() && !importGenericContent.empty()) {
        tempSource = MakeOwned<Source>(sourceWithFileID.fileID, sourceWithFileID.path, importGenericContent);
        sourcePtr = tempSource.get();
    } else {
        sourcePtr = &sourceWithFileID;
    }

    const auto& buffer = sourcePtr->buffer;

    if (buffer.empty()) {
        return "";
    }

    auto startOffset = sourcePtr->PosToOffset(begin);
    auto endOffset = sourcePtr->PosToOffset(end);
    return buffer.substr(startOffset, endOffset - startOffset);
}

void SourceManager::AddComments(const TokenVecMap& commentsMap)
{
    for (const auto& it : commentsMap) {
        CODEC_ASSERT(it.first < sources.size());
        auto& source = sources[it.first];
        for (auto tok : it.second) {
            (void)source.offsetCommentsMap.insert_or_assign(source.PosToOffset(tok.Begin()), tok);
        }
    }
}

namespace Codira {
const std::string SourceManager::testPkgSuffix = "$test";
}
