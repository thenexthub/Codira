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

#ifndef PANDA_GUARD_OBFUSCATE_MODULE_RECORD_H
#define PANDA_GUARD_OBFUSCATE_MODULE_RECORD_H

#include "entity.h"
#include "file_path.h"

namespace panda::guard {

class FilePathItem final : public Entity, public IExtractNames {
public:
    FilePathItem(Program *program, std::string literalArrayIdx)
        : Entity(program), refFilePath_(program), literalArrayIdx_(std::move(literalArrayIdx))
    {
    }

    void Build() override;

    void RefreshNeedUpdate() override;

    void Update() override;

    void ExtractNames(std::set<std::string> &strings) const override;

    ReferenceFilePath refFilePath_;
    std::string literalArrayIdx_;
};

class RegularImportItem final : public Entity, public IExtractNames {
public:
    RegularImportItem(Program *program, std::string literalArrayIdx)
        : Entity(program), literalArrayIdx_(std::move(literalArrayIdx))
    {
    }

    void RefreshNeedUpdate() override;

    void Update() override;

    void ExtractNames(std::set<std::string> &strings) const override;

    std::string literalArrayIdx_;

    uint32_t localNameIndex_ = 0;
    std::string localName_;
    std::string obfLocalName_;

    uint32_t importNameIndex_ = 0;
    std::string importName_;
    std::string obfImportName_;

    bool remoteFile_ = false;

protected:
    void WriteFileCache(const std::string &filePath) override;

    void WritePropertyCache() override;
};

class NameSpaceImportItem final : public Entity, public IExtractNames {
public:
    NameSpaceImportItem(Program *program, std::string literalArrayIdx)
        : Entity(program), literalArrayIdx_(std::move(literalArrayIdx))
    {
    }

    void RefreshNeedUpdate() override;

    void Update() override;

    void ExtractNames(std::set<std::string> &strings) const override;

    std::string literalArrayIdx_;

    uint32_t localNameIndex_ = 0;
    std::string localName_;
    std::string obfLocalName_;

    bool remoteFile_ = false;

protected:
    void WriteFileCache(const std::string &filePath) override;

    void WritePropertyCache() override;
};

class LocalExportItem final : public Entity, public IExtractNames {
public:
    LocalExportItem(Program *program, std::string literalArrayIdx)
        : Entity(program), literalArrayIdx_(std::move(literalArrayIdx))
    {
    }

    void RefreshNeedUpdate() override;

    void Update() override;

    void ExtractNames(std::set<std::string> &strings) const override;

    std::string literalArrayIdx_;

    uint32_t localNameIndex_ = 0;
    std::string localName_;
    std::string obfLocalName_;

    uint32_t exportNameIndex_ = 0;
    std::string exportName_;
    std::string obfExportName_;

protected:
    void WriteFileCache(const std::string &filePath) override;

    void WritePropertyCache() override;
};

class IndirectExportItem final : public Entity, public IExtractNames {
public:
    IndirectExportItem(Program *program, std::string literalArrayIdx)
        : Entity(program), literalArrayIdx_(std::move(literalArrayIdx))
    {
    }

    void RefreshNeedUpdate() override;

    void Update() override;

    void ExtractNames(std::set<std::string> &strings) const override;

    std::string literalArrayIdx_;

    uint32_t exportNameIndex_ = 0;
    std::string exportName_;
    std::string obfExportName_;

    uint32_t importNameIndex_ = 0;
    std::string importName_;
    std::string obfImportName_;

    bool remoteFile_ = false;

protected:
    void WriteFileCache(const std::string &filePath) override;

    void WritePropertyCache() override;
};

class ModuleRecord final : public Entity, public IExtractNames {
public:
    ModuleRecord(Program *program, const std::string &name) : Entity(program, name) {}

    void Build() override;

    void ExtractNames(std::set<std::string> &strings) const override;

    bool IsExportVar(const std::string &var);

    void RefreshNeedUpdate() override;

    void Update() override;

    void WriteNameCache(const std::string &filePath) override;

    /**
     * get local var export name by index
     * @return export name
     */
    std::string GetLocalExportName(uint32_t index);

    /**
     * Update file name references in this node
     */
    void UpdateFileNameReferences();

    /**
     * Update literalArrayIdx
    */
    void UpdateLiteralArrayIdx(const std::string &literalArrayIdx);

private:
    void CreateModuleVar(const pandasm::LiteralArray &literalArray);

    void CreateFilePathList(const std::vector<pandasm::LiteralArray::Literal> &literals, uint32_t &offset);

    void CreateRegularImportList(const std::vector<pandasm::LiteralArray::Literal> &literals, uint32_t &offset);

    void CreateNameSpaceImportList(const std::vector<pandasm::LiteralArray::Literal> &literals, uint32_t &offset);

    void CreateLocalExportList(const std::vector<pandasm::LiteralArray::Literal> &literals, uint32_t &offset);

    void CreateIndirectExportList(const std::vector<pandasm::LiteralArray::Literal> &literals, uint32_t &offset);

    void Print();

public:
    std::string literalArrayIdx_;
    std::vector<FilePathItem> filePathList_ {};
    std::vector<RegularImportItem> regularImportList_ {};
    std::vector<NameSpaceImportItem> nameSpaceImportList_ {};
    std::vector<LocalExportItem> localExportList_ {};
    std::vector<IndirectExportItem> indirectExportList_ {};
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_OBFUSCATE_MODULE_RECORD_H
