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

module GeneratorTags
  class Creator
    def create(instr, command)
      @create_list = {}
      iops = instr.in_acc_and_ops
      iops.each do |op|
        type = op.type
        if type.include?('ref') && !type.include?('[]')
          @create_list['obj'] = true
        elsif type.include?('[]')
          @create_list['arr'] = create_arr(type)
        end
      end
      create_id(instr, command)
      create_spec(instr, command)
      if instr.static?
        command['field_id'] = 'R.field0'
        @create_list['static'] = true
      end
      @create_list['func'] = instr.call? && !instr.virt?
      @create_list['virt'] = instr.virt?
      [command, @create_list]
    end

    def create_id(instr, command)
      command['string_id'] = '"\\n"' if instr.string_id?
      if instr.field_id?
        command['field_id'] = 'R.field1'
        @create_list['obj'] = true
      end
      if instr.literalarray_id?
        command['literalarray_id'] = 'array'
        @create_list['const_arr'] = {}
        type = 'i64'
        @create_list['const_arr']['type'] = type
      end
      if instr.type_id?
        if instr.newobj? || instr.lda_type?
          command['type_id'] = 'R'
          @create_list['type'] = true
        else
          command['type_id'] = 'i64[]'
        end
      end
    end

    def create_spec(instr, command)
      if instr.initobj?
        command['ctor'] = 'R.ctor'
        @create_list['ctor'] = true
      end
      if instr.isinstance?
        @create_list['isinstance'] = true
        @create_list['arr'] = create_arr('i64[]')
      end
      if instr.ldobj_obj? || instr.ldstatic_obj? || instr.ldobj_v_obj?
        command['field_id'] = 'R.field3'
        command['field_id'] = 'R.field4' if instr.static_obj?
      end
      if instr.call?
        command['method_id'] = "foo_#{instr.opcode}"
        command['method_id'] = "A.#{command['method_id']}" if instr.virt?
      end
    end

    def create_arr(type)
      array_params = {}
      array_params['size'] = 10
      type = 'i64[]' if ['top[]', 'ref[]'].include?(type)
      array_params['type'] = type
      array_params
    end
  end
end
