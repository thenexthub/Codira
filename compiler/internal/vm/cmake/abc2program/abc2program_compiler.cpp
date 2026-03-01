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

#include "abc2program_compiler.h"
#include "abc_class_processor.h"
#include "utils/timers.h"

namespace panda::abc2program {

Abc2ProgramCompiler::~Abc2ProgramCompiler()
{
    if (prog_ != nullptr) {
        delete prog_;
        prog_ = nullptr;
    }
}

bool Abc2ProgramCompiler::OpenAbcFile(const std::string &file_path)
{
    file_ = panda_file::File::Open(file_path);
    if (file_ == nullptr) {
        return false;
    }
    debug_info_extractor_ = std::make_unique<panda_file::DebugInfoExtractor>(file_.get());
    return true;
}

bool Abc2ProgramCompiler::CheckFileVersionIsSupported(std::array<uint8_t, panda_file::File::VERSION_SIZE> min_version,
                                                      uint8_t target_api_version,
                                                      std::string target_api_sub_version) const
{
    auto target_version = panda_file::GetVersionByApi(target_api_version, target_api_sub_version);
    if (!target_version.has_value()) {
        return false;
    }
    const auto &file_version = file_->GetHeader()->version;
    return panda::panda_file::IsVersionLessOrEqual(min_version, file_version) &&
        panda::panda_file::IsVersionLessOrEqual(file_version, target_version.value());
}

const panda_file::File &Abc2ProgramCompiler::GetAbcFile() const
{
    return *file_;
}

const panda_file::DebugInfoExtractor &Abc2ProgramCompiler::GetDebugInfoExtractor() const
{
    return *debug_info_extractor_;
}

pandasm::Program *Abc2ProgramCompiler::CompileAbcFile()
{
    prog_ = new pandasm::Program();
    // An abc may be generated from serveral kinds of source files.
    // prog_->lang should not be used for setting any other structure's language such as record.
    // Since prog_->lang will not be emitted into abc, this will not effect the integraty of generated abc file.
    prog_->lang = DEFUALT_SOURCE_LANG;
    auto classes = file_->GetClasses();
    std::string record_name = "";
    for (size_t i = 0; i < classes.size(); i++) {
        panda_file::File::EntityId record_id(classes[i]);
        CompileAbcClass(record_id, *prog_, record_name);
    }
    return prog_;
}

void Abc2ProgramCompiler::CompileAbcClass(const panda_file::File::EntityId &record_id,
                                          pandasm::Program &program, std::string &record_name)
{
    Abc2ProgramEntityContainer entity_container(*file_, program, *debug_info_extractor_, record_id.GetOffset(),
                                                bundle_name_);
    if (!modify_pkg_name_.empty()) {
        entity_container.SetModifyPkgName(modify_pkg_name_);
    }
    if (!origin_version_.empty() && !target_version_.empty() && !package_name_.empty()) {
        entity_container.SetPackageName(package_name_);
        entity_container.SetOriginVersion(origin_version_);
        entity_container.SetTargetVersion(target_version_);
    }
    record_name = entity_container.GetFullRecordNameById(record_id);
    panda::Timer::timerStart(EVENT_COMPILE_ABC_FILE_RECORD, record_name);
    AbcClassProcessor class_processor(record_id, entity_container);
    class_processor.FillProgramData();
    panda::Timer::timerEnd(EVENT_COMPILE_ABC_FILE_RECORD, record_name);
}

bool Abc2ProgramCompiler::CheckClassId(uint32_t class_id, size_t offset) const
{
    auto *header = file_->GetHeader();
    auto class_off = header->class_idx_off + sizeof(uint32_t) * offset;
    if (class_id > header->file_size) {
        LOG(FATAL, ABC2PROGRAM) << "> error encountered in record at " << class_off << " (0x" << std::hex
                                << class_off << "). binary file corrupted. record offset (0x" << class_id
                                << ") out of bounds (0x" << header->file_size << ")!";
    }

    /**
     * Currently, only imported custom annotations will generate external records in the abc files. Before introducing
     * the annotation feature, when compiling abc files in parallel, external records were skipped, which resulted in
     * the inability to find matching records for the corresponding annotation usage, causing errors. Therefore, when
     * compiling abc in parallel, do not skip external records anymore.
     */
    return true;
}

} // namespace panda::abc2program
