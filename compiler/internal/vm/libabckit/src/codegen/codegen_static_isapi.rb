# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
Visitor = Struct.new(:ir_op, :cpp, :switch)
Switch = Struct.new(:expr, :cases) do
  def encode
    res = "switch (#{expr}) {\n"
    cases.each do |c|
      res << c.encode
    end
    res << "default:
std::cerr << \"Codegen for \" << compiler::GetOpcodeString(inst->GetOpcode()) << \" failed\\n\";
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
    std::cerr << \"CheckWidth for \" << compiler::GetOpcodeString(inst->GetOpcode()) << \" failed\\n\";
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

If = Struct.new(:condition, :true_expr, :false_expr) do
  def proxy(method)
    res = "if (#{condition}) {\n"
    res << true_expr.send(method)
    res << "} else {\n"
    res << false_expr.send(method)
    res << "}\n"
    res
  end

  def encode
    proxy(:encode)
  end

  def check_width
    proxy(:check_width)
  end
end

Empty = Struct.new(:dummy) do
  def encode; end
  def check_width; end
end


def visit(ir_op, cpp = nil)
  @table ||= []
  @table << Visitor.new(ir_op, cpp, yield)
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

def if_(condition, true_expr, false_expr)
  If.new(condition, true_expr, false_expr)
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

def case_value(value, node)
  Case.new([value], node)
end

def case_true(opcode, *args)
  case_value('1', plain(opcode, *args))
end

def case_true_node(node)
  case_value('1', node)
end

def case_false(opcode, *args)
  case_value('0', plain(opcode, *args))
end

def case_false_node(node)
  case_value('0', node)
end

def generate_arith_op(op)
  switch("static_cast<int>(#{dst_r} != #{r(0)} && #{dst_r} != #{r(1)})",
    [case_true(op, r(0), r(1)),
     case_false_node(is_commutative?(op) ?
      if_("#{dst_r} == #{r(0)}",
        plain("#{op}v", r(0), r(1)),
        plain("#{op}v", r(1), r(0))) :
      plain(op, r(0), r(1))
     )
    ]
  )
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
def is_acc?(reg)
  "#{reg} == compiler::GetAccReg()"
end

def is_not_acc?(reg)
  "#{reg} != compiler::GetAccReg()"
end

def is_compact?(reg)
  "#{reg} < NUM_COMPACTLY_ENCODED_REGS"
end

def is_not_compact?(reg)
  "#{reg} >= NUM_COMPACTLY_ENCODED_REGS"
end

def is_fcmpg?
  'static_cast<int>(inst->IsFcmpg())'
end

def is_inci?
  "static_cast<int>(CanConvertToIncI(inst))"
end

def is_commutative?(op)
  ["add", "mul", "and", "or", "xor"].include?(op)
end

def is_arith_iv?
  "#{is_not_acc?(r(0))} && #{is_not_acc?(dst_r)} && (#{is_compact?(r(0))} || #{is_compact?(dst_r)})"
end

def cast_to_int(val)
  "static_cast<int>(#{val})"
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
  'enc->irInterface_->GetStringIdByOffset(inst->GetImm(0))'
end

def literalarray_id
  'enc->irInterface_->GetLiteralArrayIdByOffset(inst->GetImm(0))'
end

def type_id(idx)
  "enc->irInterface_->GetTypeIdByOffset(inst->GetImm(#{idx}))"
end

def field_id
  'enc->irInterface_->GetFieldIdByOffset(inst->GetTypeId())'
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
  reg_num = 0
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
  "void #{'CodeGenStatic::' if with_class}Visit#{op_name}(GraphVisitor* v, Inst* instBase)"
end

# Bytecode description itself

# Wrap all `insn` declaration in a function to call from template
# (because Panda::instructions is initialized only in templates)
def call_me_from_template
  %w[And Xor Or Shl Shr AShr].each do |op|
    visit(op) do
      op = op.downcase
      switch(type,
        [case_switch(i32_types, cast_to_int(is_not_compact?(r(0))),
          [case_true_node(if_(is_acc?(dst_r),
            plain("#{op}2", r(1)),
            plain("#{op}2v", dst_r, r(1)))),
           case_false_node(if_("#{is_compact?(dst_r)}",
            generate_arith_op(op),
            plain(op, r(0), r(1))))]),
         case_switch(i64_types, cast_to_int(is_acc?(dst_r)),
          [case_true("#{op}2.64", r(1)),
           case_false("#{op}2v.64", dst_r, r(1))])]
      )
    end
  end

  %w[add sub mul div mod].each do |op|
    visit(op.capitalize) do
      switch(type,
        [case_switch(i32_types, cast_to_int(is_not_compact?(r(0))),
          [case_true_node(if_(is_acc?(dst_r),
            plain("#{op}2", r(1)),
            plain("#{op}2v", dst_r, r(1)))),
           case_false_node(if_("#{is_compact?(dst_r)}",
            generate_arith_op(op),
            plain(op, r(0), r(1))))]),
         case_switch(i64_types, cast_to_int(is_acc?(dst_r)),
          [case_true("#{op}2.64", r(1)),
           case_false("#{op}2v.64", dst_r, r(1))]),
         case_switch(f32_types, cast_to_int(is_acc?(dst_r)),
          [case_true("f#{op}2", r(1)),
           case_false("f#{op}2v", dst_r, r(1))]),
         case_switch(f64_types, cast_to_int(is_acc?(dst_r)),
          [case_true("f#{op}2.64", r(1)),
           case_false("f#{op}2v.64", dst_r, r(1))])]
      )
    end
  end

  visit('AddI') do
    switch(type,
      [case_switch(i32_types, is_inci?,
         [case_true('inci', r(0), imm),
          case_false_node(if_(is_arith_iv?,
            plain("addiv", dst_r, r(0), imm),
            plain("addi", imm)))])]
    )
  end

  visit('SubI') do
    switch(type,
      [case_switch(i32_types, is_inci?,
         [case_true('inci', r(0), "-(#{imm})"),
          case_false_node(if_(is_arith_iv?,
            plain("subiv", dst_r, r(0), imm),
            plain("subi", imm)))])]
    )
  end

  visit('Not') do
    switch(type,
      [case_(i32_types, 'not'),
       case_(i64_types, 'not.64')]
    )
  end

  visit('Neg') do
    switch(type,
      [case_(i32_types, 'neg'),
       case_(i64_types, 'neg.64'),
       case_(f32_types, 'fneg'),
       case_(f64_types, 'fneg.64')]
    )
  end

  visit('Cmp') do
    switch('inst->GetOperandsType()',
      [case_(%w[UINT8 UINT16 UINT32], 'ucmp', r(1)),
       case_(%w[INT64], 'cmp.64', r(1)),
       case_(%w[UINT64], 'ucmp.64', r(1)),
       case_switch(['FLOAT32'], is_fcmpg?,
                     [case_true('fcmpg', r(1)),
                      case_false('fcmpl', r(1))]),
       case_switch(['FLOAT64'], is_fcmpg?,
                     [case_true('fcmpg.64', r(1)),
                      case_false('fcmpl.64', r(1))])]
    )
  end

  visit('ReturnVoid') do
    plain('return.void')
  end

  visit('ThrowIntrinsic') do
    plain('throw', r(0))
  end

  visit('NullPtr') do
    switch(cast_to_int(is_acc?(dst_r)),
      [case_true('lda.null'),
       case_false('mov.null', dst_r)]
    )
  end

  visit('LoadConstArrayIntrinsic') do
    plain('lda.const', dst_r, literalarray_id)
  end

  visit('LoadStringIntrinsic') do
    plain('lda.str', string_id)
  end

  visit('NewArrayIntrinsic') do
    plain('newarr', dst_r, r(0), type_id(0))
  end

  visit('LenArray') do
    plain('lenarr', r(0))
  end

  visit('LoadArrayIntrinsic') do
    switch(type,
      [case_(['INT8'], 'ldarr.8', r(1)),
       case_(['UINT8'], 'ldarru.16', r(1)),
       case_(['INT16'], 'ldarr.16', r(1)),
       case_(['UINT16'], 'ldarru.16', r(1)),
       case_(['INT32', 'UINT32'], 'ldarr', r(1)),
       case_(['INT64', 'UINT64'], 'ldarr.64', r(1)),
       case_(['REFERENCE'], 'ldarr.obj', r(1)),
       case_(['FLOAT32'], 'fldarr.32', r(1)),
       case_(['FLOAT64'], 'fldarr.64', r(1))]
    )
  end

  visit('StoreArrayIntrinsic') do
    switch(type,
      [case_(['INT8', 'UINT8'], 'starr.8', r(1), r(2)),
       case_(['INT16', 'UINT16'], 'starr.16', r(1), r(2)),
       case_(['INT32', 'UINT32'], 'starr', r(1), r(2)),
       case_(['INT64', 'UINT64'], 'starr.64', r(1), r(2)),
       case_(['REFERENCE'], 'starr.obj', r(1), r(2)),
       case_(['FLOAT32'], 'fstarr.32', r(1), r(2)),
       case_(['FLOAT64'], 'fstarr.64', r(1), r(2))]
    )
  end

  visit('CheckCastIntrinsic') do
    plain('checkcast', type_id(0))
  end

  visit('IsInstanceIntrinsic') do
    plain('isinstance', type_id(0))
  end

  visit('NewObjectIntrinsic') do
    plain('newobj', dst_r, type_id(0))
  end

  visit('IsNullValueIntrinsic') do
    plain('ets.isnullvalue')
  end

  visit('NullCheckIntrinsic') do
    plain('ets.nullcheck')
  end

  visit('EtsIstrueIntrinsic') do
    plain('ets.istrue', r(0))
  end

  visit('EtsTypeofIntrinsic') do
    plain('ets.typeof', r(0))
  end

  visit('EtsLdObjByNameIntrinsic') do
    field_id_from_imm = 'enc->irInterface_->GetFieldIdByOffset(static_cast<uint32_t>(inst->GetImms()[0]))'
    switch(type,
      [case_(b32_types, 'ets.ldobj.name', r(0), field_id_from_imm),
       case_(b64_types, 'ets.ldobj.name.64', r(0), field_id_from_imm),
       case_(['REFERENCE'], 'ets.ldobj.name.obj', r(0), field_id_from_imm)]
    )
  end
  # Empty visitors for IR instructions we want to ignore
  # (Add missing IRs on demand)
  %w[NullCheck BoundsCheck ZeroCheck NegativeCheck SafePoint
     InitClass SaveStateDeoptimize RefTypeCheck Phi
     Try SaveState LoadClass LoadAndInitClass Parameter LoadRuntimeClass].each do |op|
    visit(op) do
      empty
    end
  end

  %w[MulI DivI ModI ShlI ShrI AShrI AndI OrI XorI].each do |op|
    visit(op) do
      switch(type,
         [case_switch(i32_types, cast_to_int(is_arith_iv?),
          [case_true("#{op.downcase}v", dst_r, r(0), imm),
           case_false("#{op.downcase}", imm)])]
      )
    end
  end
end
