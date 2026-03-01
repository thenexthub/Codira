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

require 'ostruct'
require_relative 'codegen_arm64'
require 'delegate'

module Tokens
  module_function

  module Types
    INT8 = 'i8'
    INT16 = 'i16'
    INT32 = 'i32'
    INT64 = 'i64'
    UINT8 = 'u8'
    UINT16 = 'u16'
    UINT32 = 'u32'
    UINT64 = 'u64'
    FLOAT32 = 'f32'
    FLOAT64 = 'f64'
    BOOL = 'bool'
    REF = 'ref'
    PTR = 'ptr'
    VOID = 'void'
    IMM = 'imm'
    INTEGER = 'int'
    FLOAT = 'float'
    NUMBER = 'number'
    REAL = 'real'
    ANY = 'any'
    ACC = 'acc'
    STRING = 'string_id'
    METHOD = 'method_id'
    SAVE_STATE = 'save_state'
  end

  module Other
    DST = 'd'
    DYNAMIC = 'dyn'
    PSEUDO = 'pseudo'
    NULL_CHECK = 'nc'
    ZERO_CHECK = 'zc'
    NEGATIVE_CHECK = 'ngc'
    BOUNDS_CHECK = 'bc'
  end

end

class Operand
  attr_accessor :tokens, :types

  TYPE_ALIASES = {
    # We add `bool` type into the `int`, because IR uses same constant instructions for bool and integer types, i.e.
    # same constant instruction can be definition for bool and integer instructions simultaneously.
    "int"    => %w[bool i8 i16 i32 i64 u8 u16 u32 u64],
    "float"  => %w[f32 f64],
    "number" => %w[bool i8 i16 i32 i64 u8 u16 u32 u64 f32 f64],
    "real"   => %w[bool i8 i16 i32 i64 u8 u16 u32 u64 f32 f64 ref ptr]
  }

  CPP_IR_TYPES = {
    Tokens::Types::INT8 => "DataType::INT8",
    Tokens::Types::INT16 => "DataType::INT16",
    Tokens::Types::INT32 => "DataType::INT32",
    Tokens::Types::INT64 => "DataType::INT64",
    Tokens::Types::UINT8 => "DataType::UINT8",
    Tokens::Types::UINT16 => "DataType::UINT16",
    Tokens::Types::UINT32 => "DataType::UINT32",
    Tokens::Types::UINT64 => "DataType::UINT64",
    Tokens::Types::BOOL => "DataType::BOOL",
    Tokens::Types::FLOAT32 => "DataType::FLOAT32",
    Tokens::Types::FLOAT64 => "DataType::FLOAT64",
    Tokens::Types::REF => "DataType::REFERENCE",
    Tokens::Types::PTR => "DataType::POINTER",
    Tokens::Types::VOID => "DataType::VOID",
    Tokens::Types::ANY => "DataType::ANY"
  }

  def initialize(descr)
    @tokens = descr.split('-')
    @types = []
    @aux_types = []
    @tokens.each do |token|
      if IR::types.include?(token)
        @types << token
        next
      end
      resolved = TYPE_ALIASES[token]
      if resolved
        @types += resolved
        next
      end
      @aux_types << token
    end
  end

  def has(type)
    @tokens.include? type
  end

  def types_string
    @tokens.join(', ') + (@tokens.include?(Tokens::Other::DYNAMIC) ? ", ..." : "")
  end

  def is_dst?
    has(Tokens::Other::DST)
  end

  def is_dyn?
    has(Tokens::Other::DYNAMIC)
  end

  def pseudo?
    has(Tokens::Other::PSEUDO)
  end

  def self.cpp_type(t)
    raise "No cpp token for type #{t}" unless CPP_IR_TYPES.include?(t.to_s)
    CPP_IR_TYPES[t]
  end
end

class Instruction < SimpleDelegator
  attr_reader :operands, :inputs

  def initialize(data)
    data.modes ||= IR::modes.each_pair.map { |key, value| key.to_s }
    super(data)
    @operands = signature.map { |sgn| Operand.new(sgn) }
    if @operands.empty?
      @inputs = []
    else
      @inputs = @operands.drop(@operands.first.is_dst? ? 1 : 0)
    end
    raise "Destination can be only first operand" if inputs.any? { |x| x.is_dst? }
  end

  def has_dst?
    !operands.empty? && operands.first.is_dst?
  end

  def dst
    @operands.first
  end

  def has_inputs?
    !inputs.empty?
  end

  def is_call?
    flags.include?("call")
  end

  def resolve_item(item)
    if item.is_a? Array
      return item.map { |v| (v.start_with? '$') ? IR::templates[v[1..-1]] : v }
    elsif item.is_a? Hash
      return item.map { |k, v| (v.start_with? '$') ? [k, IR::templates[v[1..-1]]] : [k, v] }
    else
      return item
    end
  end

  def resolve
    self['verification'] = resolve_item(verification) if respond_to? 'verification'
    self
  end
end

module IR
  module_function

  def instructions
    @instructions ||= @data['instructions'].map do |inst_dscr|
      Instruction.new(OpenStruct.new(inst_dscr)).resolve
    end
  end

  def flags
    @data['flags']
  end

  def abckit_flags
    @data['abckit_flags']
  end

  def modes
    @data['modes']
  end

  def templates
    @data['templates']
  end

  def arch_info
    @data['arch_info']
  end

  def legend
    @data['legend']
  end

  def wrap_data(data)
    @data = data
  end

  def types
    @types ||= @data['types'].map { |x| x.name }
  end

  def codegen
    @@cg ||= Codegen.new
  end

  def get_ir_type(type)
    @type_map ||= {
      'i8' => 'DataType::INT8',
      'i16' => 'DataType::INT16',
      'i32' => 'DataType::INT32',
      'i64' => 'DataType::INT64',
      'u8' => 'DataType::UINT8',
      'u16' => 'DataType::UINT16',
      'u32' => 'DataType::UINT32',
      'u64' => 'DataType::UINT64',
      'f32' => 'DataType::FLOAT32',
      'f64' => 'DataType::FLOAT64',
      'obj' => 'DataType::REFERENCE',
      'any' => 'DataType::ANY',
      'acc' => 'DataType::ACC',
      'string_id' => 'DataType::UINT32',
      'method_id' => 'DataType::UINT32',
      'none' => 'DataType::NO_TYPE'}
    @type_map[type]
  end

end

def Gen.on_require(data)
  IR.wrap_data(data)
end
