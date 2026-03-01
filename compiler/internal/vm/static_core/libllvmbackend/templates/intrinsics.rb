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

require 'delegate'

def llvm_type_getter(type, gc_space)
  @llvm_type_map ||= {
    'void' => 'Void',
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
  }
  if @llvm_type_map.key? type
    return 'llvm::Type::get' + @llvm_type_map[type] + 'Ty(ctx)'
  elsif type == 'void *' || type == 'ark::Method *'
    return 'llvm::PointerType::get(ctx, 0)'
  elsif type.include? '*'
    return 'llvm::PointerType::get(ctx, '+ gc_space + ')'
  else
    raise "Unexpected type required to lower into LLVM IR: #{type}"
  end
end

class Intrinsic < SimpleDelegator
  def need_abi_wrapper?
    false
  end

  def llvm_internal_name
    '__panda_intrinsic_' + name
  end

  def enum_name
    res = name.gsub(/([A-Z]+)([A-Z][a-z])/,'\1_\2')
    res = res.gsub(/([a-z\d])([A-Z])/,'\1_\2').upcase
    is_stub ? res : 'INTRINSIC_' + res
  end

  def wrapper_impl
    return impl + 'AbiWrapper' if need_abi_wrapper?
    impl
  end
end

module Runtime
  module_function

  def intrinsics
    @exclude_list = [
    ]

    @data.intrinsics.select { |i| !@exclude_list.include?(i.name) }.map do |intrinsic|
      Intrinsic.new(intrinsic)
    end
  end

  def intrinsics_namespace
    @data.intrinsics_namespace
  end

  def coretypes
    @data.coretypes
  end

  def wrap_data(data)
    @data = data
  end
end

def Gen.on_require(data)
  Runtime.wrap_data(data)
end
