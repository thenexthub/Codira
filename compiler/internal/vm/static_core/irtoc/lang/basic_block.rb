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

require_relative 'output'

class BasicBlock
  attr_reader :index, :function, :preds, :instructions
  attr_reader :true_succ, :false_succ

  def initialize(index, function)
    @instructions = []
    @index = index
    @function = function
    @true_succ = nil
    @false_succ = nil
    @preds = []
    @variables = {}
  end

  def last_instruction
    @instructions[-1]
  end

  def empty?
    @instructions.empty?
  end

  def terminator?
    !empty? && @instructions[-1].terminator?
  end

  def append(inst)
    raise "It is not allowed to add nil instruction" if inst.nil?
    if inst.IsWhilePhi?
      @instructions.prepend(inst)
    else
      @instructions << inst
    end
  end

  def set_variable(var, inst)
    @variables[var.to_sym] = inst
  end

  def set_successor(dir, bb)
    if dir
      @true_succ = bb
    else
      @false_succ = bb
    end
    bb.add_predecessor(self)
  end

  def set_true_succ(bb)
    set_successor(true, bb)
  end

  def set_false_succ(bb)
    set_successor(false, bb)
  end

  def add_predecessor(bb)
    @preds << bb
  end

  def emit_ir
    strue_str = @true_succ.nil? ? "-1" : @true_succ.index.to_s
    sfalse_str = @false_succ.nil? ? "" : ", #{@false_succ.index}"
    Output.printlni("BASIC_BLOCK(#{index}, #{strue_str}#{sfalse_str}) {")
    @instructions.each(&:emit_ir)
    Output.printlnd("}")
  end

  def generate_builder
    @instructions.each(&:generate_builder)
  end
end
