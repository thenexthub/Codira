//===-- CommandObjectDisassemble.h ------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_SOURCE_COMMANDS_COMMANDOBJECTDISASSEMBLE_H
#define LLDB_SOURCE_COMMANDS_COMMANDOBJECTDISASSEMBLE_H

#include "lldb/Interpreter/CommandObject.h"
#include "lldb/Interpreter/Options.h"
#include "lldb/Utility/ArchSpec.h"

namespace lldb_private {

// CommandObjectDisassemble

class CommandObjectDisassemble : public CommandObjectParsed {
public:
  class CommandOptions : public Options {
  public:
    CommandOptions();

    ~CommandOptions() override;

    Status SetOptionValue(uint32_t option_idx, llvm::StringRef option_arg,
                          ExecutionContext *execution_context) override;

    void OptionParsingStarting(ExecutionContext *execution_context) override;

    llvm::ArrayRef<OptionDefinition> GetDefinitions() override;

    const char *GetPluginName() {
      return (plugin_name.empty() ? nullptr : plugin_name.c_str());
    }

    const char *GetFlavorString() {
      if (flavor_string.empty() || flavor_string == "default")
        return nullptr;
      return flavor_string.c_str();
    }

    const char *GetCPUString() {
      if (cpu_string.empty() || cpu_string == "default")
        return nullptr;
      return cpu_string.c_str();
    }

    const char *GetFeaturesString() {
      if (features_string.empty() || features_string == "default")
        return nullptr;
      return features_string.c_str();
    }

    Status OptionParsingFinished(ExecutionContext *execution_context) override;

    bool show_mixed; // Show mixed source/assembly
    bool show_bytes;
    bool show_control_flow_kind;
    uint32_t num_lines_context = 0;
    uint32_t num_instructions = 0;
    bool raw;
    std::string func_name;
    bool current_function = false;
    lldb::addr_t start_addr = 0;
    lldb::addr_t end_addr = 0;
    bool at_pc = false;
    bool frame_line = false;
    std::string plugin_name;
    std::string flavor_string;
    std::string cpu_string;
    std::string features_string;
    ArchSpec arch;
    bool some_location_specified = false; // If no location was specified, we'll
                                          // select "at_pc".  This should be set
    // in SetOptionValue if anything the selects a location is set.
    lldb::addr_t symbol_containing_addr = 0;
    bool force = false;
    bool enable_variable_annotations = false;
  };

  CommandObjectDisassemble(CommandInterpreter &interpreter);

  ~CommandObjectDisassemble() override;

  Options *GetOptions() override { return &m_options; }

protected:
  void DoExecute(Args &command, CommandReturnObject &result) override;

  llvm::Expected<std::vector<AddressRange>>
  GetRangesForSelectedMode(CommandReturnObject &result);

  llvm::Expected<std::vector<AddressRange>> GetContainingAddressRanges();
  llvm::Expected<std::vector<AddressRange>> GetCurrentFunctionRanges();
  llvm::Expected<std::vector<AddressRange>> GetCurrentLineRanges();
  llvm::Expected<std::vector<AddressRange>>
  GetNameRanges(CommandReturnObject &result);
  llvm::Expected<std::vector<AddressRange>> GetPCRanges();
  llvm::Expected<std::vector<AddressRange>> GetStartEndAddressRanges();

  llvm::Expected<std::vector<AddressRange>>
  CheckRangeSize(std::vector<AddressRange> ranges, llvm::StringRef what);

  CommandOptions m_options;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_COMMANDS_COMMANDOBJECTDISASSEMBLE_H
