#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#

def check_N_v8_format(insn)
  insn.operands.each do |op|
    raise "Instruction " + insn.mnemonic + " has unexpected format " + insn.format.pretty if ! op.reg? || op.width != 8
  end
end

def get_format_for(insn)
  fmt = insn.format.pretty
  if fmt == "imm4_v4_v4_v4_v4_v4"
    # Merge imm4_v4_v4_v4_v4_v4 and imm4_v4_v4_v4 since they haave the same handling code
    fmt = "imm4_v4_v4_v4"
  end

  if insn.prefix
    if fmt != "pref_imm16_v8" &&  fmt != "pref_imm16_v8_prof16"
      check_N_v8_format(insn)
    end
    fmt = insn.opcode
  end
  return "call_#{fmt}"
end

def get_call_insns
  Panda.instructions.select { |insn| insn.properties.include?('call') && insn.properties.include?('dynamic') }
end
