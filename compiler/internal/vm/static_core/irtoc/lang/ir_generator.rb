#!/usr/bin/env ruby

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

require_relative 'output'

# Generates c++ source code, that replaces source instruction by the given graph.
class GeneratorIrInline

  def initialize
    @func = nil
  end

  def generate_function(func)
    @func = func

    Output.scoped_puts "inline bool #{func.name}(Inst* source_inst) {" do
      Output << "Graph* graph = source_inst->GetBasicBlock()->GetGraph();"
      Output << "[[maybe_unused]] auto pc = source_inst->GetPc();"
      Output << "[[maybe_unused]] auto save_state = source_inst->GetSaveState();"
      Output << "[[maybe_unused]] auto current_inst = source_inst;" if @func.simple_control_flow?
      func.params.keys.each_with_index do |_, index|
        Output << "Inst* p_#{index} = source_inst->GetInput(#{index}).GetInst();"
      end
      Output << "[[maybe_unused]] auto *runtime = graph->GetRuntime();"
      func.constants.each { |_, inst| generate_instruction(inst) }

      if @func.simple_control_flow?
        @func.basic_blocks.each { |bb| generate_bb(bb) }
      else
        generate_ir_with_control_flow
      end

      Output << "source_inst->GetBasicBlock()->RemoveInst(source_inst);"
      Output << "return true;"
    end
  end

  def generate_ir_with_control_flow
    Output << "// NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)"
    Output << "auto cur_bb = source_inst->GetBasicBlock();"
    Output << "[[maybe_unused]] auto end_bb = cur_bb->SplitBlockAfterInstruction(source_inst, false);"
    @func.basic_blocks.each do |bb|
      if false && bb.terminator?
        Output << "auto* bb_#{bb.index} = end_bb;"
      else
        Output << "auto* bb_#{bb.index} = graph->CreateEmptyBlock(pc);"
        Output << "bb_#{bb.index}->CopyTryCatchProps(cur_bb);"
        Output << "cur_bb->GetLoop()->AppendBlock(bb_#{bb.index});"
      end
    end
    Output << "cur_bb->AddSucc(bb_#{@func.basic_blocks.first.index});"
    @func.basic_blocks.each do |bb|
      name = "bb_#{bb.index}"
      if bb.terminator?
        raise "Terminator block should have no successors" if bb.true_succ || bb.false_succ
        if bb.instructions.last.IsReturn?
          Output << "#{name}->AddSucc(end_bb);"
        else
          Output << "#{name}->AddSucc(graph->GetEndBlock());"
        end
      else
        Output << "#{name}->AddSucc(bb_#{bb.true_succ.index});" if bb.true_succ
        Output << "#{name}->AddSucc(bb_#{bb.false_succ.index});" if bb.false_succ
      end
    end
    @func.basic_blocks.each { |bb| generate_bb(bb) }
  end

  def generate_bb(bb)
    Output << "/********************************************************"
    Output << " * bb #{bb.index}"
    Output << ' */'
    Output << "// NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)"
    Output << "cur_bb = bb_#{bb.index};" unless @func.simple_control_flow?
    bb.instructions.each { |inst| generate_instruction(inst) }
  end

  def generate_instruction(inst)
    raise 'No LiveIn/LiveOut are allowed in IR Builder generator' if inst.IsLiveIn? || inst.IsLiveOut?
    var_name = inst.local_var_name
    Output << "// Inst #{inst}"
    if inst.IsReturnVoid?
      raise "ReturnVoid is not allowed in cpp mode"
    elsif inst.IsReturn?
      raise "Return has #{inst.inputs.size}" if inst.inputs.size != 1

      Output << "source_inst->ReplaceUsers(#{inst.inputs.first.local_var_name});"
    elsif inst.IsConstant?
      Output << "auto* #{var_name} = graph->FindOrCreateConstant(#{inst.fields[:Value]});"
    else
      if inst.IsCall?
        index = inst.modifiers.detect {|mod| mod[0] == :TypeId}[1][0]
        name = inst.bb.function.external_funcs[index].snakecase
        intrinsic_id = name.split('_')[0..-2].join('_').upcase
        Output << "auto* #{var_name} = graph->CreateInstIntrinsic(DataType::#{inst.get_type_for_cpp}, pc);"
        Output << "#{var_name}->SetIntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_#{intrinsic_id});"
        Output << "#{var_name}->SetFlag(inst_flags::CAN_THROW);"
      else
        Output << "auto* #{var_name} = graph->CreateInst#{inst.name}(DataType::#{inst.get_type_for_cpp}, pc);"
      end
      Output << "#{var_name}->SetNeedBarrier(true);" if inst.IsStoreArray? && inst.type == :ref
      generate_inst_inputs(inst)
      if inst.IsCmp? || inst.IsCompare? || inst.IsIf? || inst.IsIfImm? || inst.IsSelect? || inst.IsSelectImm?
        Output << "#{var_name}->SetOperandsType(DataType::#{inst.inputs.first.get_type_for_cpp});"
      end
      inst.generate_inst_modifiers
      if inst.IsCastAnyTypeValue? || inst.IsCast?
        input_name = get_inst_var_name(inst.inputs.first)
        Output.scoped_puts "if (#{input_name}->GetOpcode() == Opcode::AnyTypeCheck) {" do
          if inst.IsCastAnyTypeValue?
            Output << "#{input_name}->CastToAnyTypeCheck()->SetAnyType(#{var_name}->GetAnyType());"
            Output << "#{var_name}->SetAllowedInputType(#{input_name}->CastToAnyTypeCheck()->GetAllowedInputType());"
          end
        end
      end
      if inst.IsIntrinsic?
        Output.scoped_puts "if (#{var_name}->RequireState()) {" do
          Output << "#{var_name}->SetSaveState(save_state);"
        end
      elsif inst.dscr.flags.include?('require_state')
        Output << "#{var_name}->SetSaveState(save_state);"
      end
      append_inst(inst.local_var_name, inst.IsPhi?)
    end
  end

  def generate_inst_inputs(inst)
    need_save_state = inst.IsCall?
    raise 'SaveState is added only for instructions with dynamic number of inputs' if need_save_state && !inst.dynamic_inputs?
    num_inputs = inst.inputs.size + (need_save_state ? 1 : 0)
    var_name = inst.local_var_name
    if inst.dynamic_inputs?
      Output << "// NOLINTNEXTLINE(readability-magic-numbers)"
      Output << "#{var_name}->ReserveInputs(#{num_inputs});"
      Output << "#{var_name}->AllocateInputTypes(graph->GetAllocator(), #{num_inputs});" unless inst.IsPhi?
    end
    inst.inputs.each_with_index do |input, i|
      input_name = input.IsParameter? ? "p_#{input.index}" : input.local_var_name
      if inst.dynamic_inputs?
        Output << "#{var_name}->AppendInput(#{input_name});"
        Output << "#{var_name}->AddInputType(DataType::#{input.get_type_for_cpp});" unless inst.IsPhi?
      else
        Output << "#{var_name}->SetInput(#{i}, #{input_name});"
      end
    end
    if need_save_state
      save_state_var_name = "#{var_name}_save_state"
      Output << "auto #{save_state_var_name} = graph->CreateInstSaveState(DataType::Type::NO_TYPE, pc);"
      Output << "#{var_name}->AppendInput(#{save_state_var_name});"
      Output << "#{var_name}->AddInputType(DataType::NO_TYPE);"
      append_inst(save_state_var_name)
    end
  end

  def append_inst(var_name, is_phi = false)
    if @func.simple_control_flow?
      Output << "current_inst->InsertAfter(#{var_name});"
      Output << "current_inst = #{var_name};"
    else
      Output << "cur_bb->Append#{is_phi ? "Phi" : "Inst"}(#{var_name});"
    end
  end

  def get_inst_var_name(inst)
    inst.IsParameter? ? "p_#{inst.index}" : inst.local_var_name
  end
end
