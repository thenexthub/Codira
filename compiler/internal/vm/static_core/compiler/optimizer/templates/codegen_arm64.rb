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


class Codegen
  attr_accessor :prologue

  def reset_prologue
    @prologue = ''
  end

  def reset
    @runtime_info = false
  end

  def runtime_info
    if !@runtime_info
       @prologue += %Q(auto runtime_info = visitor->GetGraph()->GetRuntime();)
       @runtime_info = true
    end
    'runtime_info'
  end

  def tmp_reg

  end

  #######################################################################################
  # Next methods are DSL tokens implementation
  #
  def safepoint_state
    @prologue += %Q(
    if (cg->GetGraph()->GetRuntime()->GetGCEntryPoint() == 0) {
       // No GC_ENTRY - do not encode SafePoint
       return;
    }
    auto runtime_info = visitor->GetGraph()->GetRuntime();
    auto flag_addr_offset = #{runtime_info}->GetFlagAddrOffset();
    auto tmp_reg = #{tmp 'u64'};
    auto arg_mem = vixl::aarch64::MemOperand(vixl::aarch64::sp, cg->GetStackOffset());
    __ Ldr(tmp_reg, arg_mem);
    // TMP <= ExecState Address
    auto safepoint_mem = vixl::aarch64::MemOperand(tmp_reg, flag_addr_offset);
    // safepoint_mem - memory for read flag value
    )
    "safepoint_mem"
  end

  def safepoint_constant
    "#{runtime_info}->GetSafepointActiveValue()"
  end

  def back_safepoint_label
    "cg->GetBackEdgeSP(inst)"
  end

  def safepoint_label
    @prologue += %Q(
    auto label = cg->GetLabelSS(inst);)
    'label'
  end

  def ld(size, dst, src)
    case size
    when 8
        "__ Ldrb(#{dst}, #{src});"
    when 16
        "__ Ldrh(#{dst}, #{src});"
    else
        "__ Ldr(#{dst}, #{src});"
    end
  end

  def cmp(a, b)
    "__ Cmp(#{a}), #{b});"
  end

  def jeq(a, b, target)
    %Q(__ Cmp(#{a}, #{b});
    __ B(vixl::aarch64::Condition::eq, #{target});)
  end

  def jneq(a, b, target)
    %Q(__ Cmp(#{a}, #{b});
    __ B(vixl::aarch64::Condition::ne, #{target});)
  end

  def bind(label)
    %Q(__ Bind(#{label});)
  end

  def tmp(type)
    "cg->EncodeReg(cg->GetTmp(), #{IR::get_ir_type(type)})"
  end

  ##
  # This method aims to handle DSL tokens that don't have methods with same name, for example, ld_8 redirected to
  # ld method with 8 passed as argument
  #
  def method_missing name, *args
    if m = /^ld_(\d+)$/.match(name)
      return ld(m[1], *args)
    elsif m = /^tmp_([suf]\d+)$/.match(name)
      return tmp(m[1])
    else
      raise NoMethodError, "Unknown codegen command: #{name}"
    end
  end
end
