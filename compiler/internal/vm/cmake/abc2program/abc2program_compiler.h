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

#ifndef ABC2PROGRAM_ABC2PROGRAM_COMPILER_H
#define ABC2PROGRAM_ABC2PROGRAM_COMPILER_H

#include <string>
#include <assembly-program.h>
#include <debug_info_extractor.h>
#include "common/abc2program_entity_container.h"

namespace panda::abc2program {

class Abc2ProgramCompiler {
public:
    ~Abc2ProgramCompiler();
    bool OpenAbcFile(const std::string &file_path);
    bool CheckFileVersionIsSupported(std::array<uint8_t, panda_file::File::VERSION_SIZE> min_version,
                                     uint8_t target_api_version, std::string target_api_sub_version) const;
    const panda_file::File &GetAbcFile() const;
    const panda_file::DebugInfoExtractor &GetDebugInfoExtractor() const;
    pandasm::Program *CompileAbcFile();
    void CompileAbcClass(
        const panda_file::File::EntityId &record_id,
        pandasm::Program &program,
        std::string &record_name);
    bool CheckClassId(uint32_t class_id, size_t offset) const;

    void SetBundleName(std::string bundle_name)
    {
        bundle_name_ = bundle_name;
    }
    void SetModifyPkgName(const std::string &modify_pkg_name)
    {
        modify_pkg_name_ = modify_pkg_name;
    }
    void SetPackageName(const std::string &package_name)
    {
        package_name_ = package_name;
    }
    void SetOriginVersion(std::string origin_version)
    {
        origin_version_ = origin_version;
    }
    void SetTargetVersion(std::string target_version)
    {
        target_version_ = target_version;
    }

private:
    std::unique_ptr<const panda_file::File> file_;
    std::unique_ptr<panda_file::DebugInfoExtractor> debug_info_extractor_;
    // the single whole program compiled from the abc file, only used in non-parallel mode
    pandasm::Program *prog_ = nullptr;
    // It should modify record name when the bundle_name_ is not empty
    std::string bundle_name_ {};
    std::string modify_pkg_name_ {};
    // fix version for the abc input
    std::string package_name_ {};
    std::string origin_version_ {};
    std::string target_version_ {};
}; // class Abc2ProgramCompiler

} // namespace panda::abc2program

#endif // ABC2PROGRAM_ABC2PROGRAM_COMPILER_H
