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
require 'set'
require 'delegate'

def get_shorty_type(type)
  @shorty_type_map ||= {
    'void' => 'VOID',
    'u1' => 'U1',
    'i8' => 'I8',
    'u8' => 'U8',
    'i16' => 'I16',
    'u16' => 'U16',
    'i32' => 'I32',
    'u32' => 'U32',
    'i64' => 'I64',
    'u64' => 'U64',
    'f32' => 'F32',
    'f64' => 'F64',
    'any' => 'TAGGED',
    'acc' => 'TAGGED',
    'string_id' => 'U32',
    'method_id' => 'U32',
  }
  @shorty_type_map[type] || 'REFERENCE'
end

def primitive_type?(type)
  @primitive_types ||= Set[
    'void',
    'u1',
    'i8',
    'u8',
    'i16',
    'u16',
    'i32',
    'u32',
    'i64',
    'u64',
    'f32',
    'f64',
    'any',
    'acc',
    'string_id',
    'method_id',
  ]
  @primitive_types.include?(type)
end

def get_primitive_descriptor(type)
  @primitive_type_map ||= {
    'u1' => 'Z',
    'i8' => 'B',
    'u8' => 'H',
    'i16' => 'S',
    'u16' => 'C',
    'i32' => 'I',
    'u32' => 'U',
    'i64' => 'J',
    'u64' => 'Q',
    'f32' => 'F',
    'f64' => 'D',
    'any' => 'A',
    'acc' => 'A',
    'string_id' => 'S',
    'method_id' => 'M',
  }
  @primitive_type_map[type]
end

def object_type?(type)
  !primitive_type?(type)
end

def union_type?(type)
  type[0,2] == '{U'
end

def get_object_descriptor_impl(type)
  rank = type.count('[')
  type = type[0...-(rank * 2)] if rank > 0
  if primitive_type?(type)
    res = '[' * rank + get_primitive_descriptor(type)
    return res, type.length + rank * 2
  elsif not union_type?(type)
    res = '[' * rank + 'L' + type.gsub('.', '/') + ';'
    return res, type.length + rank * 2
  end
  res = '{U'
  idx = 2
  while type[idx] != '}' do
    if type[idx] == ','
      idx += 1
      next
    end
    if type[idx] == '{'
      elem, len = get_object_descriptor_impl(type[idx..-1])
      res += elem
      idx += len
      next
    end
    comma_pos = type[idx..-1].index(',')
    if not comma_pos
      comma_pos = type[idx..-1].index('}')
    end
    component, len = get_object_descriptor_impl(type[idx..comma_pos+idx-1])
    res += component
    idx += len
  end
  res = '[' * rank + res + '}'
  return res, idx + 1 + rank*2
end

def get_object_descriptor(type)
  res, len = get_object_descriptor_impl(type)
  return res
end

def floating_point_type?(type)
  @fp_types ||= Set[
    'f32',
    'f64',
  ]
  @fp_types.include?(type)
end

class Intrinsic < SimpleDelegator
  def enum_name
    res = name.gsub(/([A-Z]+)([A-Z][a-z])/,'\1_\2')
    res.gsub(/([a-z\d])([A-Z])/,'\1_\2').upcase
  end

  def need_abi_wrapper?
    defined? orig_impl
  end

  def has_impl?
    respond_to?(:impl)
  end

  def is_irtoc?
    class_name == 'Irtoc'
  end
end

module Runtime
  module_function

  def intrinsics
    @data.intrinsics.map do |intrinsic|
      Intrinsic.new(intrinsic)
    end
  end

  def include_headers
    @data.include_headers
  end

  def wrap_data(data)
    @data = data
  end
end

def Gen.on_require(data)
  Runtime.wrap_data(data)
end
