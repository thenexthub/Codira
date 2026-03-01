# Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

require 'yaml'

def llvm_type_getter(type, gc_space)
  @llvm_type_map ||= {
    'void' => 'Void',
    'bool' => 'Int8',
    'int8_t' => 'Int8',
    'uint8_t' => 'Int8',
    'int16_t' => 'Int16',
    'uint16_t' => 'Int16',
    'int32_t' => 'Int32',
    'uint32_t' => 'Int32',
    'int64_t' => 'Int64',
    'uint64_t' => 'Int64',
    'float' => 'Float',
    'double' => 'Double',
    'ark::FileEntityId' => 'Int64',
  }
  if ['size_t', 'ssize_t', 'uintptr_t', 'size_t pid'].include? type
    # Predefined variable. Type of 'size_t' is implementation defined
    return 'size_type'
  elsif @llvm_type_map.key? type
    return 'llvm::Type::get' + @llvm_type_map[type] + 'Ty(ctx)'
  elsif type.include?('ObjectHeader**')
    return 'llvm::PointerType::get(ctx, 0)'
  elsif type.include?('*') && type.include?('coretypes') || type.include?('ObjectHeader*')
    return 'llvm::PointerType::get(ctx, ' + gc_space + ')'
  elsif type.include? '*'
    return 'llvm::PointerType::get(ctx, 0)'
  else
    raise "Unexpected type required to lower into LLVM IR: #{type}"
  end
end

class String
  def snakecase
    self.gsub(/::/, '/').
    gsub(/([A-Z]+)([A-Z][a-z])/,'\1_\2').
    gsub(/([a-z\d])([A-Z])/,'\1_\2').
    tr("-", "_").
    downcase
  end
end

class Entrypoint
  def initialize(dscr)
    @dscr = dscr
  end

  def name
    @dscr['name']
  end

  def llvm_internal_name
    '__panda_entrypoint_' + name
  end

  def enum_name
    @dscr['name'].snakecase.upcase
  end

  def entrypoint_name
    @dscr.entrypoint.nil? ? "#{name}Entrypoint" : @dscr.entrypoint
  end

  def get_entry
    "#{name}Entrypoint"
  end

  def signature
    @dscr['signature']
  end

  def variadic?
    signature.include? '...'
  end

  def has_property? prop
    @dscr['properties']&.include? prop
  end

  def bridge
    @dscr['bridge'].upcase
  end

end

module Compiler
  module_function

  def entrypoints
    @entrypoints ||= @data['entrypoints'].map {|x| Entrypoint.new x }
  end

  def entrypoints_crc32
    require "zlib"
    Zlib.crc32(entrypoints.map(&:signature).join)
  end

  def wrap_data(data)
    @data = data
  end
end

def Gen.on_require(data)
  Compiler.wrap_data(data)
end
