# Copyright (c) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# ISAPI specialization for bytecode optimizer

Instruction.class_eval do
  def src_acc_kind
    op = acc_and_operands.select { |op| op.acc? && op.src? }.first
    raise "There is no src acc for #{mnemonic}" unless op
    data_kind_helper(op)
  end

  def dst_acc_kind
    op = acc_and_operands.select { |op| op.acc? && op.dst? }.first
    raise "There is no dst acc for #{mnemonic}" unless op
    data_kind_helper(op)
  end

  private
  # return one of 32, 64, 'ref'
  def data_kind_helper(op)
    m = /[fiub](?<size>\d+)/.match(op.type)
    if m
      size = m[:size].to_i
      if size == 64
        return 64
      else
        return 32
      end
    end
    return 'ref' if op.type == 'ref'
    raise "Unexpected operand type #{op.type} in data_kind_helper"
  end
end

def instruction_hash
  unless defined? @instruction_hash
    @instruction_hash = Hash.new { |_, key| raise "No instruction with '#{key}' mnemonic" }
    Panda.instructions.each { |insn| @instruction_hash[insn.mnemonic] = insn }
  end
  @instruction_hash
end

# Classes for bytecode description
Visitor = Struct.new(:ir_op, :switch)
Switch = Struct.new(:expr, :cases) do
  def encode
    res = "switch (#{expr}) {\n"
    cases.each do |c|
      res << c.encode
    end
    res << "default:
std::cerr << \"Codegen for \" << compiler::GetOpcodeString(inst->GetOpcode()) << \" failed\";
enc->success_ = false;
}"
    res
  end

  def check_width
    res = "switch (#{expr}) {\n"
    cases.each do |c|
      res << c.check_width
    end
    res << "default:
std::cerr << \"CheckWidth for \" << compiler::GetOpcodeString(inst->GetOpcode()) << \" failed\";
re->success_ = false;
}"
    res
  end
end

Case = Struct.new(:types, :node) do
  def proxy(method)
    res = types.map { |type| "case #{type}:" }.join("\n")
    res << " {\n"
    res << node.send(method)
    res << "break;\n}\n"
    res
  end

  def encode
    proxy(:encode)
  end

  def check_width
    proxy(:check_width)
  end
end

Leaf = Struct.new(:instruction, :args) do
  def encode
    res = ""
    args_str = args.join(",\n")
    if instruction.acc_read?
      res << do_lda(instruction)
      res << "\n"
    end
    res << "enc->result_.emplace_back(pandasm::Create_#{instruction.asm_token}(\n"
    res << args_str
    res << "\n));\n"
    if instruction.acc_write?
      res << do_sta(instruction)
      res << "\n"
    end
    res
  end

  def check_width
    reg = instruction.operands.select(&:reg?).first
    if reg
      "re->Check#{reg.width}Width(inst);\n"
    else
      "return;\n"
    end
  end
end

Empty = Struct.new(:dummy) do
  def encode; end
  def check_width; end
end

# Sugar for bytecode description
def visit(ir_op)
  @table ||= []
  @table << Visitor.new(ir_op, yield)
end

def visitors
  @table
end

def switch(expr, cases)
  Switch.new(expr, cases)
end

def plain(opcode, *args)
  Leaf.new(instruction_hash[opcode], args.to_a)
end

def empty
  Empty.new
end

def prefixed_case(prefix, types, node)
  types = types.map { |t| "#{prefix}#{t}" }
  Case.new(types, node)
end

def case_(types, opcode, *args)
  prefixed_case("compiler::DataType::", types, plain(opcode, *args))
end

def cc_case(types, opcode, *args)
  prefixed_case("compiler::CC_", types, plain(opcode, *args))
end

def case_switch(types, condition, inner_cases)
  prefixed_case("compiler::DataType::", types, switch(condition, inner_cases))
end

def case_true(opcode, *args)
  Case.new(['1'], plain(opcode, *args))
end

def case_false(opcode, *args)
  Case.new(['0'], plain(opcode, *args))
end

# Type/cc cases for instruction selection
def i32_types
  @i32_types ||= %w[BOOL UINT8 INT8 UINT16 INT16 UINT32 INT32]
end

def i64_types
  @i64_types ||= %w[INT64 UINT64]
end

def f32_types
  @f32_types ||= %w[FLOAT32]
end

def f64_types
  @f64_types ||= %w[FLOAT64]
end

def b64_types
  @b64_types ||= i64_types + f64_types
end

def b32_types
  @b32_types ||= i32_types + f32_types
end

def void_types
  @void_types ||= %w[VOID]
end

def cc_cases
  @cc_cases ||= %w[EQ NE LT LE GT GE]
end

# Switch condition printers
def type
  'inst->GetType()'
end

def src_type
  'inst->GetInputType(0)'
end

# we could use switch on 'bool' type for if-else purposes, but that hurts clang_tidy
def if_acc?(reg)
  "static_cast<int>(#{reg} == compiler::GetAccReg())"
end

def if_fcmpg?
  'static_cast<int>(inst->IsFcmpg())'
end

# Operand printers
def dst_r
  'inst->GetDstReg()'
end

def r(num)
  "inst->GetSrcReg(#{num})"
end

def imm
  'static_cast<int32_t>(inst->GetImm() & 0xffffffff)'
end

def label
  'LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId())'
end

def string_id
  'enc->ir_interface_->GetStringIdByOffset(inst->GetTypeId())'
end

def literalarray_id
  'enc->ir_interface_->GetLiteralArrayIdByOffset(inst->GetTypeId()).value()'
end

def type_id
  'enc->ir_interface_->GetTypeIdByOffset(inst->GetTypeId())'
end

def field_id
  'enc->ir_interface_->GetFieldIdByOffset(inst->GetTypeId())'
end

# Lda/Sta printers
def do_lda(instruction)
  lda = case instruction.src_acc_kind
        when 32
          instruction_hash['lda']
        when 64
          instruction_hash['lda.64']
        when 'ref'
          instruction_hash['lda.obj']
        end
  reg_num = if instruction.mnemonic.include?('ldarr') || instruction.mnemonic.include?('stobj') || instruction.mnemonic.include?('ststatic')
              1
            elsif instruction.mnemonic.include?('starr')
              2
            else
              0
            end
  "if (inst->GetSrcReg(#{reg_num}) != compiler::GetAccReg()) {
    enc->result_.emplace_back(pandasm::Create_#{lda.asm_token}(inst->GetSrcReg(#{reg_num})));
  }"
end

def do_sta(instruction)
  sta = case instruction.dst_acc_kind
        when 32
          instruction_hash['sta']
        when 64
          instruction_hash['sta.64']
        when 'ref'
          instruction_hash['sta.obj']
        end
  "if (inst->GetDstReg() != compiler::GetAccReg()) {
    enc->result_.emplace_back(pandasm::Create_#{sta.asm_token}(inst->GetDstReg()));
  }"
end

# Misc printers
def visitor_sig(op_name, with_class = true)
  "void #{'CodeGenDynamic::' if with_class}Visit#{op_name}(GraphVisitor* v, Inst* instBase)"
end

# Bytecode description itself

# Wrap all `insn` declaration in a function to call from template
# (because Panda::instructions is initialized only in templates)
def call_me_from_template
  # Empty visitors for IR instructions we want to ignore
  # (Add missing IRs on demand)
  %w[Phi Try SaveState Parameter].each do |op|
    visit(op) do
      empty
    end
  end
end
