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

require 'yaml'

class InstructionsData
  @@instructions = {}
  @@types = {}

  def self.setup(filename)
    if Gem::Version.new(RUBY_VERSION) < Gem::Version.new('3.1.0')
      yaml_data = YAML.load_file(filename)
    else
      yaml_data = YAML.load_file(filename, aliases: true)
    end

    yaml_data['instructions'].each do |inst|
      abort "Instruction description doesn't contain opcode field" unless inst.include? "opcode"
      @@instructions[inst["opcode"].to_sym] = inst
    end
    yaml_data['pseudo_instructions'].each do |inst|
      abort "Instruction description doesn't contain opcode field" unless inst.include? "opcode"
      @@instructions[inst["opcode"].to_sym] = inst
    end

    yaml_data['types'].each do |type|
      @@types[type["name"]] = type
    end
  end

  def self.instructions; @@instructions; end
  def self.types; @@types; end
end