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

require_relative 'basic_block'
require_relative 'instruction'
require_relative 'output'
require_relative 'regmask'

class CfBlock
  attr_accessor :bb, :head_bb, :goto
  attr_reader :kind, :depth

  module Kind
    IfThen = 0
    IfElse = 1
    While = 2
  end

  def initialize(kind, depth)
    @kind = kind
    @depth = depth
    @head_bb = nil
    @bb = nil
    @goto = false
  end

  def set_succ(bb)
    if @kind == Kind::While
      @bb.set_false_succ(bb)
    else
      @bb.set_true_succ(bb)
    end
  end

  def self.get_kind(inst)
    @@opc_map ||= { If: Kind::IfThen, IfImm: Kind::IfThen, Else: Kind::IfElse, While: Kind::While, AddOverflow: Kind::IfThen, SubOverflow: Kind::IfThen }
    @@opc_map[inst.opcode]
  end
end

class UnresolvedVar
  attr_reader :name
  attr_accessor :inst

  def initialize(name)
    @name = name.to_sym
    @inst = nil
  end
end

class Label
  attr_reader :refs, :name
  attr_accessor :bb, :explicitly_defined

  def initialize(name, bb=nil)
    @name = name
    @bb = bb
    @refs = []
    @explicitly_defined = false
  end
end

class Function

  attr_reader :name, :mode, :validation, :enable_builder, :external_funcs, :source_dirs, :source_files, :arguments, :params, :constants, :basic_blocks

  def initialize(name, params: {}, regmap: {}, mode: nil, regalloc_set: nil, regalloc_include: nil, regalloc_exclude: nil,
                 validate: nil, enable_builder: false, lang: 'PANDA_ASSEMBLY', compile_for_llvm: false, &block)
    @name = name
    @mode = mode
    @lang = lang
    @compile_for_llvm = compile_for_llvm
    @regalloc_mask = regalloc_set ? regalloc_set : $default_mask
    @regalloc_mask |= regalloc_include unless regalloc_include.nil?
    @regalloc_mask &= ~regalloc_exclude unless regalloc_exclude.nil?

    @instructions = []
    @basic_blocks = []
    @locals = [{}]
    @current_depth = 0
    @unresolved = [{}]
    @cf_stack = []
    @labels = [{}]
    @global_labels = {}

    if validate
      @validation = {}
      validate.each do |k, v|
        if v.is_a?(Hash)
          value = v[Options.arch.to_sym]
          value = v[:default] unless value
        else
          value = v
        end
        @validation[k] = value
      end

      @validation.each { |name, v| raise "#{@name}: no target arch in validation value: #{name}" unless v }
    end
    @body = block
    @external_funcs = []
    @enable_builder = enable_builder
    @source_dirs = []
    @source_files = []

    self.define_singleton_method(:regmap) { regmap }
    # Lists of special/global instructions
    @arguments = []
    @registers = {}
    @constants = {}

    new_basic_block()

    # Process function's arguments
    params.each_with_index do |(name, type), i|
      inst = create_instruction(:Parameter)
      inst.send(type)
      inst.set_parameter_index(i)
      @arguments << inst
      let(name, inst)
    end
    @params = params
  end

  # Return true if graph doesn't contain conditional basic block, i.e. with more than one successor.
  def simple_control_flow?
    @result ||= @basic_blocks.none? { |bb| bb.true_succ && bb.false_succ }
  end

  def compile
    Options.compiling = true
    instance_eval &@body
    Options.compiling = false
    resolve_variables
    @basic_blocks.delete_if { |bb| bb.empty? && bb.preds.empty? }
  end

  def defines
    Options.definitions
  end

  def create_instruction(opcode, inputs = [])
    if opcode == :WhilePhi
      raise "Wrong place of `WhilePhi` instruction" unless @cf_stack[-1].kind == CfBlock::Kind::While
      raise "Invalid `While` block" if @cf_stack[-1].head_bb.nil?
      block = @cf_stack[-1].head_bb
    else
      block = @current_block
    end
    inst = IRInstruction.new(opcode, inst_index(), block)
    inst.add_inputs(inputs.map { |input|
      (input.is_a? Integer or input.is_a? Float or input.is_a? String) ? get_or_create_constant(input) : input
    })
    @instructions << inst unless inst.IsElse?
    block.append inst unless inst.global?
    inst
  end

  def get_or_create_constant(value)
    constant = @constants[value]
    return @constants[value] unless constant.nil?
    @constants[value] = IRInstruction.new(:Constant, inst_index(), @current_block, Value: value).i64
  end

  def let(var_name, inst)
    if inst.is_a? Integer or inst.is_a? String
      inst = get_or_create_constant(inst)
    end
    resolved = @unresolved.last[var_name.to_sym]
    if resolved
      raise "Unresolved variable is defined more than once: #{var_name}" unless resolved.inst.nil?
      resolved.inst = inst
    end
    @locals.last[var_name.to_sym] = inst
    self.define_singleton_method(var_name.to_sym) do
      r = @locals.last[var_name.to_sym]
      return r unless r.nil?
      var = UnresolvedVar.new(var_name.to_sym)
      @unresolved.last[var_name.to_sym] = var
      var
    end
    inst
  end

  def global(var_sym)
    self.define_singleton_method(var_sym) do
      scope = @locals.reverse_each.detect { |scope| !scope[var_sym].nil? }
      return scope[var_sym] unless scope.nil?
      var = UnresolvedVar.new(var_sym)
      @unresolved.last[var_sym] = var
      var
    end
  end

  def inst_index
    @inst_index ||= 0
    @inst_index += 1
    @inst_index - 1
  end

  def method_missing(method, *args, &block)
    if Options.compiling && args.empty? && block.nil?
      method = method.to_sym
      var = UnresolvedVar.new(method)
      @unresolved.last[method] = var
      var
    else
      super
    end
  end

  def verify_labels()
    # verifies if labels mentioned in the code are explicitly defined
    # by means of :Label or :LabelGlobal
    @labels.each do |label|
      label.each do |k, v|
        if v.explicitly_defined == false
          raise "label '#{v.name}' is undefined in '#{@name}'"
        end
      end
    end
    @global_labels.each do |k, v|
      if v.explicitly_defined == false
          raise "global label '#{v.name}' is undefined in '#{@name}'"
      end
    end
  end

  def emit_body()
    if @regalloc_mask
      Output.println("GetGraph()->SetArchUsedRegs(~0x#{@regalloc_mask.value.to_s(16)});")
      Output << "// Regalloc mask: #{@regalloc_mask}"
    end
    Output.println(%Q[SetExternalFunctions({"#{@external_funcs.join('", "')}"});]) if @external_funcs
    @source_dirs.each_with_index do |dir, index|
      Output.println("uint32_t DIR_#{index} = AddSourceDir(\"#{dir}\");")
    end
    @source_files.each_with_index do |file, index|
      Output.println("uint32_t FILE_#{index} = AddSourceFile(\"#{file}\");")
    end
    Output.printlni("GRAPH(GetGraph()) {")
    if @mode.include?(:NativePlus)
      @arguments.each_with_index do |(v), i|
        v.set_parameter_index(i+1)
      end
      inst = create_instruction(:Parameter)
      inst.send(:ref)
      inst.set_parameter_index(0)
      @arguments.prepend(inst)
    end
    @arguments.each { |inst| inst.emit_ir }
    @constants.each { |_, inst| inst.emit_ir }
    @registers.each { |_, inst| inst.emit_ir }
    @basic_blocks.each {|bb| bb.emit_ir }
    Output.printlnd("}")
    Output.printlnd("}")
  end

  def emit_header(suffix, mode)
    Output.printlni("COMPILE(#{@name}#{suffix}) {")

    Output.println("if(GetGraph()->GetArch() != #{Options.cpp_arch}) LOG(FATAL, IRTOC) << \"Arch doesn't match\";")
    if suffix == "_LLVM"
      Output.println("GetGraph()->SetIrtocPrologEpilogOptimized();")
    end
    Output.println("GetGraph()->SetRelocationHandler(this);")
    Output.println("SetLanguage(SourceLanguage::#{@lang});")
    Output.println("[[maybe_unused]] auto *graph = GetGraph();")
    Output.println("[[maybe_unused]] auto *runtime = GetGraph()->GetRuntime();")
    Output.println("SetArgsCount(#{@params.length});")
    ss = "GetGraph()->SetMode(GraphMode(0)"
    @mode.each do |m|
      ss += " | GraphMode::#{m.to_s}(true)"
    end
    Output.println(ss + ");")
  end

  def emit_ir(suffix)
    raise "Compilation mode is not specified" unless @mode

    verify_labels()

    if @mode.include?(:FastPathPlus) # only FastPathPlus macro is supported for now
      @mode.delete(:FastPathPlus)
      @mode.push(:FastPath)
      emit_header(suffix, mode)
	  emit_body
      @mode.pop()
      @mode.push(:NativePlus)
      emit_header(suffix + 'NativePlus', mode)
	  emit_body
      @mode.pop()
      @mode.push(:FastPathPlus)
    else
	  emit_header(suffix, mode)
	  emit_body
  	end
  end

  def generate_builder
    params = "Graph* graph, [[maybe_unused]] InstBuilder *inst_builder, BasicBlock** cur_bb, #{@params.keys.map.with_index { |_, i| "Inst* p_#{i}" }.join(', ')}"
    params += ', ' unless @params.empty?
    params += 'size_t pc, Marker marker'
    Output.println("// NOLINTNEXTLINE(readability-function-size)")
    Output.printlni("[[maybe_unused]] static Inst* Build#{@name}(#{params}) {")
    Output.println("[[maybe_unused]] auto *runtime = graph->GetRuntime();")
    @constants.each { |_, inst| inst.generate_builder }
    Output.println("auto need_new_last_bb = (*cur_bb)->IsEmpty();  // create an empty BB or extract from an existing one?")
    Output.println("auto* bb_#{@basic_blocks.last.index} = need_new_last_bb ? graph->CreateEmptyBlock() : (*cur_bb)->SplitBlockAfterInstruction((*cur_bb)->GetLastInst(), false);")
    Output.println("ASSERT(bb_#{@basic_blocks.last.index}->GetGuestPc() == INVALID_PC);")
    Output.println("bb_#{@basic_blocks.last.index}->SetGuestPc(pc);")
    Output.println("bb_#{@basic_blocks.last.index}->CopyTryCatchProps(*cur_bb);")
    Output.println("if ((*cur_bb)->IsMarked(marker)) {")
    Output.println("    bb_#{@basic_blocks.last.index}->SetMarker(marker);")
    Output.println("}")
    Output.println("if (need_new_last_bb) {")
    Output.println("    for (auto succ : (*cur_bb)->GetSuccsBlocks()) {")
    Output.println("        succ->ReplacePred(*cur_bb, bb_#{@basic_blocks.last.index});")
    Output.println("    }")
    Output.println("    (*cur_bb)->GetSuccsBlocks().clear();")
    Output.println("}")
    Output.println("if ((*cur_bb)->IsLoopPreHeader()) {")
    Output.println("    (*cur_bb)->GetNextLoop()->SetPreHeader(bb_#{@basic_blocks.last.index});")
    Output.println("}")
    @basic_blocks[0...-1].each { |bb|
      Output.println("auto* bb_#{bb.index} = graph->CreateEmptyBlock();")
      Output.println("ASSERT(bb_#{bb.index}->GetGuestPc() == INVALID_PC);")
      Output.println("bb_#{bb.index}->SetGuestPc(pc);")
      Output.println("bb_#{bb.index}->CopyTryCatchProps(*cur_bb);")
      Output.println("if ((*cur_bb)->IsMarked(marker)) {")
      Output.println("    bb_#{bb.index}->SetMarker(marker);")
      Output.println("}")
    } if @basic_blocks.size > 1
    Output.println("(*cur_bb)->AddSucc(bb_#{@basic_blocks.first.index});")
    @basic_blocks.each do |bb|
      name = "bb_#{bb.index}"
      Output.println("#{name}->AddSucc(bb_#{bb.true_succ.index});") if bb.true_succ
      Output.println("#{name}->AddSucc(bb_#{bb.false_succ.index});") if bb.false_succ
    end
    Output.printlni('if ((*cur_bb)->GetLoop() != nullptr) {')
    Output.printlni('if (need_new_last_bb) {')
    Output.println("(*cur_bb)->GetLoop()->AppendBlock(bb_#{@basic_blocks.last.index});")
    Output.printlnd('}')
    @basic_blocks[0...-1].each { |bb| Output.println("(*cur_bb)->GetLoop()->AppendBlock(bb_#{bb.index});") }
    Output.printlnd('}')
    Output.println("(*cur_bb) = bb_#{@basic_blocks.last.index};")
    @basic_blocks.each(&:generate_builder)
    Output.printlnd('}')
  end

  def self.setup
    InstructionsData.instructions.each do |name, dscr|
      # Some of the opcodes were already defined manually, e.g. Intrinsic
      next if method_defined? name

      define_method(name) do |*inputs, &block|
        raise "Current basic block is nil" if @current_block.nil?
        inst = create_instruction(name, inputs)
        inst.annotation = Kernel.send(:caller)[0].split(':in')[0]
        caller_data = caller_locations[0]
        file_path = File.expand_path(caller_data.path)
        inst.debug_info.dir = add_source_dir(File.dirname(file_path))
        inst.debug_info.file = add_source_file(File.basename(file_path))
        inst.debug_info.line = caller_data.lineno
        process_inst(inst, &block)
        inst
      end
    end
  end

  def Intrinsic(name, *inputs)
    inst = create_instruction(:Intrinsic, inputs).IntrinsicId("RuntimeInterface::IntrinsicId::INTRINSIC_#{name}")
    caller_data = caller_locations[0]
    file_path = File.expand_path(caller_data.path)
    inst.debug_info.dir = add_source_dir(File.dirname(file_path))
    inst.debug_info.file = add_source_file(File.basename(file_path))
    inst.debug_info.line = caller_data.lineno
    process_inst(inst)
    inst
  end

  def StaticAssertEQ(*inputs)
        raise "Current basic block is nil" if @current_block.nil?
        if inputs.size != 2
          raise "expects 2 arguments, got #{inputs.size} "
        end
        inputs.each_with_index do |input, i|
          if !(input.is_a? Integer or input.is_a? Float or input.is_a? String)
            raise "expected constant argument, got \'#{input.name}\'"
          end
        end
        inst = create_instruction(:StaticAssertEQ, inputs)
        inst.annotation = Kernel.send(:caller)[0].split(':in')[0]
        caller_data = caller_locations[0]
        file_path = File.expand_path(caller_data.path)
        inst.debug_info.dir = add_source_dir(File.dirname(file_path))
        inst.debug_info.file = add_source_file(File.basename(file_path))
        inst.debug_info.line = caller_data.lineno
        process_inst(inst)
        inst
      end

  def AsGlobalLabel(name)
    global_label_name = "#{@name}_" + name.to_s
    if !(@global_labels.key? global_label_name)
      label = Label.new(global_label_name, nil)
      label.explicitly_defined = false
      @global_labels[global_label_name] = label
    end
    global_label_name
  end

  def GotoGlobal(name)
    if @global_labels.key? name.to_s
      label = @global_labels[name.to_s]
      if label.bb
        @current_block.set_true_succ(label.bb)
      else
        label.refs << @current_block
      end
    else
      raise "Global label is undefined: #{__method__}(#{name})"
    end
  end

  def LabelGlobal(name)
    last_bb = @current_block
    new_basic_block
    last_bb.set_true_succ(@current_block) if last_bb.true_succ.nil? && !last_bb.terminator?
    global_label_name = "#{@name}_" + name.to_s
    if @global_labels.key? global_label_name
      label = @global_labels[global_label_name]
      label.explicitly_defined = true
      label.bb = @current_block
      label.refs.each { |bb| bb.set_true_succ(@current_block) }
    else
      label = Label.new(global_label_name, @current_block)
      label.explicitly_defined = true
      label.refs.each { |bb| bb.set_true_succ(@current_block) }
      @global_labels[global_label_name] = label
    end
    # Create its local clone as one may need Goto(:name) locally (not from scoped_macro)
    label = @labels.last[name]
    if label
      label.bb = @current_block
      label.refs.each { |bb| bb.set_true_succ(@current_block) }
      label.explicitly_defined = true
    else
      label = Label.new(name, @current_block)
      label.explicitly_defined = true
      @labels.last[name] = label
    end
  end

  def Label(name)
    last_bb = @current_block
    new_basic_block
    last_bb.set_true_succ(@current_block) if last_bb.true_succ.nil? && !last_bb.terminator?
    label = @labels.last[name]
    if label
      label.bb = @current_block
      label.refs.each { |bb| bb.set_true_succ(@current_block) }
      label.explicitly_defined = true
    else
      new_lab = Label.new(name, @current_block)
      new_lab.explicitly_defined = true
      @labels.last[name] = new_lab
    end
  end

  def Goto(name)
    if @labels.last.key? name
      label = @labels.last[name]
      if label.bb
        @current_block.set_true_succ(label.bb)
      else
        label.refs << @current_block
      end
    else
      label = @labels.last[name] = Label.new(name)
      label.refs << @current_block
    end
    cblock = @cf_stack[-1]
    cblock.goto = true unless cblock.nil?
    process_inst nil
  end

  def LiveIn(value, type = 'ptr')
    value = regmap[value] if value.is_a?(Symbol)
    res = @registers[value]
    return res unless res.nil?
    inst = create_instruction(:LiveIn).DstReg(value)
    inst.send(type)
    @registers[value] = inst
  end

  def resolve_variables
    @instructions.each do |inst|
      inst.inputs.each_with_index do |input, i|
        if input.is_a? UnresolvedVar
          raise "Function: #{@name} Unresolved variable: #{input.name}" if input.inst.nil?
          inst.inputs[i] = input.inst
        elsif input.nil?
          raise "Input is nil for: #{inst}, input"
        end
      end
    end

    @unresolved.last.each_pair do |name, var|
      raise "Variable cannot be resolved: #{name}" if var.inst.nil?
    end
  end

  def get_if_block(else_depth)
    @cf_stack.reverse.each do |block|
      return block if block.depth == else_depth && block.kind == CfBlock::Kind::IfThen
    end
    raise "`If` block not found"
  end

  def process_inst(inst, &block)
    if !block.nil?
      @current_depth += 1
      cblock = CfBlock.new(CfBlock::get_kind(inst), @current_depth)
      cblock.head_bb = @current_block
      @cf_stack << cblock
      new_basic_block() unless @current_block.empty?

      if inst.IsIf? || inst.IsIfImm? || inst.IsWhile? || inst.IsAddOverflow? || inst.IsSubOverflow?
        cblock.head_bb.set_true_succ(@current_block)
      elsif inst.IsElse?
        get_if_block(@current_depth).head_bb.set_false_succ(@current_block)
      end

      instance_eval &block
      @current_depth -= 1

      cblock.bb = @current_block
      new_basic_block()

    else
      last_block = @cf_stack[-1]
      return if !last_block.nil? && @current_depth == last_block.depth && last_block.kind == CfBlock::Kind::IfElse
      @cf_stack.delete_if do |block|
        next if block.depth <= @current_depth

        if block.bb.true_succ.nil?
          if block.kind == CfBlock::Kind::While
            block.bb.set_true_succ(block.head_bb)
          else
            # If last instruction in the block is a terminator(e.g. Return), then don't set true successor, hence it
            # will point to the end block, that is required by IR design.
            block.bb.set_true_succ(@current_block) unless !block.bb.empty? && block.bb.last_instruction.terminator?
          end
        end


        if block.head_bb.false_succ.nil? && (block.kind == CfBlock::Kind::IfThen || block.kind == CfBlock::Kind::While)
          block.head_bb.set_false_succ(@current_block)
        end

        true
      end
    end
  end

  def new_basic_block
    @bb_index ||= 2
    @current_block = BasicBlock.new(@bb_index, self)
    @bb_index += 1
    @basic_blocks << @current_block
    @current_block
  end

  def add_source_dir(dir)
    index = @source_dirs.index(dir)
    return index unless index.nil?
    @source_dirs << dir
    @source_dirs.size - 1
  end

  def add_source_file(filename)
    raise "File name must be a string" unless filename.is_a? String
    index = @source_files.index(filename)
    return index unless index.nil?
    @source_files << filename
    @source_files.size - 1
  end

  # leave only inputs that are defined (pass inputs that can be missing as symbols)
  def load_phi_inputs(*inputs)
    inputs.reject { |x| (x.is_a? Symbol) && (!respond_to? x) }.map { |x| (x.is_a? Symbol) ? send(x) : x }
  end

end
