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

require "ostruct"
require_relative 'instructions_data'
require_relative 'output'

class InstIsaDescription
  def initialize(inst, dscr)
    @inst = inst
    @dscr = dscr
  end

  def pseudo?
    @dscr.flags.include? "pseudo"
  end

  def has_dst?
    return false if pseudo?
    !@dscr.signature.empty? && @dscr.signature[0].split('-').include?('d')
  end
end

class IRInstruction
  attr_reader :index, :inputs, :name, :bb, :fields, :modifiers, :dscr
  attr_accessor :type, :annotation, :debug_info

  DebugInfo = Struct.new(:dir, :file, :line)

  def initialize(name, index, bb, **kwargs)
    @name = name
    @index = index
    @bb = bb
    @dscr = OpenStruct.new(InstructionsData.instructions[name])
    @fields = kwargs
    @inputs = []
    @type = nil
    @modifiers = []
    @debug_info = DebugInfo.new
    abort "Wrong instructions #{name}" unless InstructionsData.instructions.include? name
  end

  def set_parameter_index(index)
    raise "Trying to set argument index for non Parameter instruction" unless IsParameter?
    @fields[:ArgIndex] = index
  end

  def no_dce
    SetFlag('inst_flags::NO_DCE')
  end

  def set_access_type(type)
    SetAccessType("DynObjectAccessType::#{type.to_s.upcase}")
  end

  def set_access_mode(mode)
    SetAccessMode("DynObjectAccessMode::#{mode.to_s.upcase}")
  end

  def global?
    return IsConstant? || IsLiveIn? || IsElse? || IsParameter?
  end

  def terminator?
    IsReturn? || IsReturnVoid? || IsThrow? || has_modifier?(:Terminator)
  end

  def has_modifier?(mod)
    @modifiers.any? { |x| x[0] == mod}
  end

  def opcode
    @name
  end

  def pseudo?
    @dscr.flags.include? "pseudo"
  end

  def dynamic_inputs?
    @dscr.signature&.any? { |operand| operand.end_with?('-dyn') }
  end

  def add_inputs(insts)
    @inputs += insts
  end

  def mw(&block)
    @type = "mw"
    if !block.nil?
      @bb.function.process_inst(self, &block)
    end
    self
  end

  def ref_uint(&block)
    @type = "ref_uint"
    if !block.nil?
      @bb.function.process_inst(self, &block)
    end
    self
  end

  def method_missing(method, *args, **kwargs, &block)
    if Options.compiling
      @modifiers << [method, args]
      @bb.function.process_inst(self, &block) unless block.nil?
      self
    else
      super
    end
  end

  def Method(name, setter = :TypeId)
    index = @bb.function.external_funcs.index(name)
    if index.nil?
        index = @bb.function.external_funcs.size
        @bb.function.external_funcs << name
    end
    send(setter, index)
  end

  def MethodAsImm(name)
    index = @bb.function.external_funcs.index(name)
    if index.nil?
        index = @bb.function.external_funcs.size
        @bb.function.external_funcs << name
    end
    AddImm(index)
  end

  def emit_ir
    opc = opcode()
    Output << "// #{self.to_s}"
    Output << "// #{self.annotation}"
    if IsConstant?()
      ss = "CONSTANT(#{@index}, #{@fields[:Value]})"
    elsif IsParameter?()
      Output.println "PARAMETER(#{@index}, #{@fields[:ArgIndex]}).#{@type}();"
      return
    elsif IsElse?
      return
    elsif IsStaticAssertEQ?
      lhs = @inputs[0].fields[:Value]
      rhs = @inputs[1].fields[:Value]
      ss = "static_assert((#{lhs}) == (#{rhs}));"
      Output.println ss
      return
    else
      opc = :If if IsWhile?
      opc = :Phi if IsWhilePhi?
      ss = "INST(#{@index}, Opcode::#{opc})"
    end

    if IsIntrinsic? || IsCallIndirect? || IsCall? || IsCallDynamic?
      inputs = @inputs.map do |input|
        t = input.type == "mw" ? "IrConstructor::MARK_WORD_TYPE" : "DataType::#{input.get_type_for_cpp}"
        "{#{t}, #{input.index}}"
      end.join(", ")
      ss += ".Inputs({#{inputs}})" unless inputs.empty?
    else
      raise "Instruction has unresolved inputs: #{self}" if @inputs.any? {|x| x.nil? }
      inputs = @inputs.map(&:index).join(", ")
      ss += ".Inputs(#{inputs})" unless inputs.empty?
    end

    type = @type == :void ? :v0id : @type
    ss += ".#{type}()" unless type.nil?
    @modifiers.each do |mod|
      ss += ".#{modifier_to_s(mod)}"
    end
    ss += ".Loc(DIR_#{@debug_info.dir}, FILE_#{@debug_info.file}, #{@debug_info.line})" if @debug_info.line
    ss += ';'
    Output.println ss
  end

  def to_s
    "Inst(id=#{@index}, opc=#{@name})"
  end

  def inspect
    to_s
  end

  def get_type_for_cpp
    @@type_map ||= {
      nil   => :NO_TYPE,
      :i8   => :INT8,
      :i16  => :INT16,
      :i32  => :INT32,
      :i64  => :INT64,
      :u8   => :UINT8,
      :u16  => :UINT16,
      :u32  => :UINT32,
      :u64  => :UINT64,
      :f32  => :FLOAT32,
      :f64  => :FLOAT64,
      :b    => :BOOL,
      :ref  => :REFERENCE,
      :ptr  => :POINTER,
      :void => :VOID,
      :any  => :ANY,
      :ref_uint => :'GetIntTypeForReference(GetGraph()->GetArch())'
    }
    res = @@type_map[@type&.to_sym]
    raise "Wrong type: #{@type}" if res.nil?
    res
  end

  def self.setup
    InstructionsData.types[:word] = nil
    InstructionsData.types[:sword] = nil
    InstructionsData.types.each do |name, _|
      name = name == 'bool' ? :b : name.to_sym
      define_method(name) do |&block|
        @type = name == :word ? (Options.arch_64_bits? ? 'u64' : 'u32') :
               (name == :sword ? (Options.arch_64_bits? ? 'i64' : 'i32') : name)
        @bb.function.process_inst(self, &block) if !block.nil?
        self
      end
    end

    InstructionsData.instructions.each do |opcode, inst|
      define_method("Is#{opcode}?".to_sym) do
        @name == inst['opcode'].to_sym
      end
    end

    # Generate concise functions for creating condition code: If().CC(:CC_EQ) => If().EQ
    [:EQ, :NE, :GE, :GT, :LE, :LT, :A, :AE, :B, :BE].each do |x| define_method(x) do |&block|
        send(:CC, "CC_#{x}".to_sym, &block)
        self
      end
    end
  end

  def generate_builder
    Output << "// #{self.to_s}"
    Output << "// #{self.annotation}"
    # TODO(mbolshov): raise 'No LiveIn/LiveOut are allowed in IR Builder generator' if IsLiveIn? || IsLiveOut?
    if IsReturnVoid?
      Output.println('return nullptr;')
    elsif IsReturn?
      raise "Return has #{@inputs.size}" if @inputs.size != 1

      Output.println("return #{@inputs.first.local_var_name};")
    elsif IsConstant?
      Output.println("// NOLINTNEXTLINE(readability-magic-numbers)")
      Output.println("auto* #{local_var_name} = graph->FindOrCreateConstant(#{@fields[:Value]});")
    else
      if IsCall?
        index = @modifiers.detect {|mod| mod[0] == :TypeId}[1][0]
        name = @bb.function.external_funcs[index].snakecase
        intrinsic_id = name.split('_')[0..-2].join('_').upcase
        intrinsic_id = "RuntimeInterface::IntrinsicId::INTRINSIC_#{intrinsic_id}"
        Output.println("auto* #{local_var_name} = graph->CreateInstIntrinsic(DataType::#{get_type_for_cpp}, pc);")
        Output.println("#{local_var_name}->SetIntrinsicId(#{intrinsic_id});")
        Output.println("#{local_var_name}->SetFlag(inst_flags::CAN_THROW);")
      else
        Output.println("auto* #{local_var_name} = graph->CreateInst#{@name}(DataType::#{get_type_for_cpp}, pc);")
      end
      generate_inst_inputs
      if IsCmp? || IsCompare? || IsIf? || IsIfImm? || IsSelect? || IsSelectImm?
        Output.println("#{local_var_name}->SetOperandsType(DataType::#{@inputs.first.get_type_for_cpp});")
      end
      generate_inst_modifiers
      if IsPhi?
        Output.println("bb_#{@bb.index}->AppendPhi(#{local_var_name});")
      else
        Output.println("bb_#{@bb.index}->AppendInst(#{local_var_name});")
      end
    end
  end

  # name of local variable in generated IR Builder source code
  def local_var_name
    IsParameter? ? "p_#{@index}" : "l_#{@index}"
  end

  def modifier_to_s(mod)
    "#{mod[0]}(#{mod[1].join(', ')})"
  end

  def generate_inst_inputs
    need_save_state = IsCall?
    raise 'SaveState is added only for instructions with dynamic number of inputs' if need_save_state && !dynamic_inputs?
    num_inputs = @inputs.size + (need_save_state ? 1 : 0)
    if dynamic_inputs?
      Output.println("// NOLINTNEXTLINE(readability-magic-numbers)")
      Output.println("#{local_var_name}->ReserveInputs(#{num_inputs});")
      Output.println("#{local_var_name}->AllocateInputTypes(graph->GetAllocator(), #{num_inputs});") unless IsPhi?
    end
    @inputs.each_with_index do |input, i|
      input_name = input.local_var_name
      if dynamic_inputs?
        Output.println("#{local_var_name}->AppendInput(#{input_name});")
        Output.println("#{local_var_name}->AddInputType(DataType::#{input.get_type_for_cpp});") unless IsPhi?
      else
        Output.println("#{local_var_name}->SetInput(#{i}, #{input_name});")
      end
    end
    if need_save_state
      # SaveState is the last input by convention:
      save_state_var_name = "#{local_var_name}_save_state"
      Output.println("auto #{save_state_var_name} = inst_builder->CreateSaveState(Opcode::SaveState, pc);");
      Output.println("#{local_var_name}->AppendInput(#{save_state_var_name});")
      Output.println("#{local_var_name}->AddInputType(DataType::NO_TYPE);")
      Output.println("bb_#{@bb.index}->AppendInst(#{save_state_var_name});")
    end
  end

  def generate_inst_modifiers
    @modifiers.each do |mod|
      next if IsCall?

      mod[0] = 'SetOperandsType' if mod[0] == :SrcType
      mod[0] = 'SetCc' if mod[0] == :CC
      prefix = 'Set' unless mod[0].to_s.start_with?('Set')
      Output.println("// NOLINTNEXTLINE(readability-magic-numbers)")
      Output.println("#{local_var_name}->#{prefix}#{modifier_to_s(mod)};")
    end
  end

  def dump(stm = STDOUT)
    mods = @modifiers.empty? ? "" : ".#{@modifiers.join('.')}"
    type = @type.nil? ? "" : ".#{@type}"
    stm.print("#{index}.#{@name}#{type}")
    if !@inputs.empty?
      inputs = @inputs.map { |x| "#{x.index}"}
      stm.print(" (#{inputs.join(', ')})")
    end
    stm.print(mods)
    if !@fields.empty?
      stm.print(", #{@fields}")
    end
  end

  def inspect
    ss = StringIO.new
    dump(ss)
    ss.string
  end

  def to_s
    inspect
  end

end
