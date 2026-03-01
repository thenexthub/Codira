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

Instruction.class_eval do
  def reg_dst?
    dst = false
    ops = operands
    ops.each do |op|
      dst = true if op.dst?
    end
    dst
  end

  def dst?
    dst = false
    ops = acc_and_operands
    ops.each do |op|
      dst = true if op.dst?
    end
    dst
  end

  def reg_src?
    src = false
    ops = operands
    ops.each do |op|
      next if op.id?

      src = true if op.src?
    end
    src
  end

  def src?
    src = false
    ops = acc_and_operands
    ops.each do |op|
      next if op.id?

      src = true if op.src?
    end
    src
  end

  def dyn?
    properties.include?('dynamic') && !properties.include?('maybe_dynamic')
  end

  def in_ops
    ops = operands
    iops = []
    ops.each do |op|
      next if op.id?

      iops << op if op.src?
    end
    iops
  end

  def in_acc_and_ops
    ops = acc_and_operands
    iops = []
    ops.each do |op|
      next if op.id?

      iops << op if op.src?
    end
    iops
  end

  def core?
    namespace == 'core'
  end

  def field_id?
    sig.include?('field_id')
  end

  def literalarray_id?
    sig.include?('literalarray_id')
  end

  def string_id?
    sig.include?('string_id')
  end

  def static?
    sig.include?('static')
  end

  def call?
    sig.include?('call')
  end

  def type_id?
    sig.include?('type_id')
  end

  def virt?
    sig.include?('virt')
  end

  def isinstance?
    sig.include?('isinstance')
  end

  def lda_type?
    sig.include?('lda_type')
  end

  def newobj?
    sig.include?('newobj')
  end

  def initobj?
    sig.include?('initobj')
  end

  def ldobj_obj?
    sig.include?('ldobj.obj')
  end

  def ldstatic_obj?
    sig.include?('ldstatic.obj')
  end

  def ldobj_v_obj?
    sig.include?('ldobj.v.obj')
  end

  def static_obj?
    sig.include?('static.obj')
  end
end

Operand.class_eval do
  attr_accessor :insert
end
