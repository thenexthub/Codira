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

require 'erb'
require 'creator'
require 'calculator'

module GeneratorTags
  ACC_NUM = 65_536

  class InstructionBuild
    def initialize(instr, create)
      @instr = instr
      @create = create
    end

    def bind
      binding
    end
  end

  class Generator
    INT_VAL = 42
    FLOAT_VAL = 42.42

    def generate_input_number(type, in_reg_num)
      val = 0
      case type[0]
      when 'b'
        val = INT_VAL
      when 'i'
        val = INT_VAL
      when 'f'
        val = FLOAT_VAL
      when 'u'
        val = in_reg_num
      end
      val
    end

    def initialize(template)
      @template = template
      @command = {}
      @creator = Creator.new
      @calculator = Calculator.new
    end

    def skip?(inst)
      !inst.dst? || inst.dyn? || !inst.core?
    end

    def concat_args(instr, create)
      command = instr.mnemonic
      if create['ctor']
        command += " #{@command['ctor']}"
        command += ',' if instr.src? || @command['out_reg']
      end
      if @command['method_id']
        command += " #{@command['method_id']}"
        command += ',' if instr.src? || @command['out_reg']
      end
      if @command['out_reg']
        command += " #{@command['out_reg']}"
        command += ', ' if instr.reg_src? || @command['literalarray_id'] || @command['field_id']
      end
      iops = instr.in_ops

      iops[0].insert = 'v4' if create['virt']

      iops.each do |op|
        next if op.insert == ''

        command += " #{op.insert}"
        command += ', ' if iops.size != 1 && op != iops[-1] || @command['field_id']
      end
      if @command['type_id']
        command += ',' if @command['out_reg'] || !iops.empty?
        command += " #{@command['type_id']}"
      end
      command += " #{@command['string_id']}" if @command['string_id']
      command += " #{@command['field_id']}" if @command['field_id']
      command += " #{@command['literalarray_id']}" if @command['literalarray_id']
      command
    end

    def generate_file(new_file, instr)
      @calculator.refresh
      @command['fill_reg'] = []
      iops = instr.in_ops
      iops.each do |op|
        if op.imm?
          op.insert = generate_input_number(op.type, iops.size - 1)
        elsif op.reg? && op.src? && op.dst?
          op.insert = ''
        elsif op.type.include?('[]') # Array
          op.insert = 'v3'
        elsif op.type.include?('ref') # Object
          op.insert = 'v4'
        elsif op.reg?
          if !(op.type.include?('top') || op.type.include?('[]'))
            free_reg = @calculator.free_register
            @command['fill_reg'].append(free_reg)
          else
            free_reg = 'v3' # top[] case
          end
          op.insert = free_reg
        end
      end
      if instr.reg_dst?
        @command['dst_reg'] = 0
        @command['out_reg'] = 'v0'
      else
        @command['dst_reg'] = ACC_NUM
      end
      @command['expected_tag'] = @calculator.calculate_expected_tag(instr.dtype)
      @command, create_list = @creator.create(instr, @command)
      @command['sig'] = concat_args(instr, create_list)
      @command['in_ops'] = instr.in_ops
      templ_instr = InstructionBuild.new(@command, create_list)
      erb = ERB.new(@template)
      new_file << erb.result(templ_instr.bind)
      @command.clear
    end
  end
end
