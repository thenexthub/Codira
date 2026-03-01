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

require 'ostruct'
require 'yaml'

# Object of `CompiledMethod` is created for each method, defined in the `validation.yaml`, which looks as follows:
# AllocateObjectTlab:
#   spills_count_max: 0
#   code_size_max: 125
#
class CompiledMethod < OpenStruct
  def initialize
    super
  end

  def spills_count_max(value)
    real_spills_count = self.spills_count.to_i
    if real_spills_count > value
      raise "[Validation] `spills_count_max` failed for method `#{self.name}`: expected(#{value}) > real(#{real_spills_count})"
    end
  end

  def code_size_max(value)
    code_size_number = self.code_size.to_i
    if code_size_number > value
      raise "[Validation] `code_size_max` failed for method `#{self.name}`: expected(#{value}) > real(#{code_size_number})"
    end
  end
end

def parse_methods(dump_file)
  methods = []
  current_method = nil
  good_data = false
  File.open(dump_file).read.each_line do |line|
    if !line.start_with? " "
      if line.start_with? "METHOD_INFO:"
        methods << CompiledMethod.new
        good_data = true
        next
      elsif line.start_with? "CODE_STATS:"
        good_data = true
      else
        good_data = false
      end
      next
    end
    next if !good_data || line.start_with?("==")

    data = line.split(':', 2).map(&:strip)
    if data&.size == 2
      methods[-1][data[0]] = data[1]
    end
  end
  Hash[methods.map { |m| [m.name.split('::')[1].to_sym, m] } ]
end

def main
  validation_file = ARGV[0]
  dump_file = ARGV[1]

  methods = parse_methods(dump_file)

  YAML.load_file(validation_file).each do |method_name, data|
    method = methods[method_name.to_sym]
    raise "Method not found: #{method_name}" unless method
    data.each { |constraint, value| method.send(constraint.to_sym, value) }
  end
end

main