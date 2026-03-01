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

require 'ostruct'
require 'delegate'

class Pass < SimpleDelegator
  attr_reader :operands, :inputs

  def initialize(data)
    super(data)
  end

  def verify_type
    possible_types = PassRegistry::pass_types
    for pass_type in self['type']
      if not possible_types.include?(pass_type)
        raise "Unknown pass-type `#{pass_type}` in pass `#{self['name']}` description."
      end
    end
    self
  end
end

module PassRegistry
  module_function

  def pass_types
    @possible_types ||= @data.pass_types.each_pair.map { |key, value| key.to_s }
  end

  def llvm_passes
    @passes ||= @data.llvm_passes.map do |pass|
      Pass.new(OpenStruct.new(pass)).verify_type
    end
  end

  def passes_namespace
    @data.passes_namespace
  end

  def wrap_data(data)
    @data = data
  end
end
  
def Gen.on_require(data)
  PassRegistry.wrap_data(data)
end
