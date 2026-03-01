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

class CppFunction
  attr_reader :variants

  def initialize(name)
    @name = name
    @variants = []
    @cpp_params = {}
  end

  def condition(cond)
    @current_variant.cond = cond
  end

  def code(&block)
    @current_variant.func = Function.new(@current_variant.name, params: @params, mode: [:IrInline], &block)
    @current_variant.func.compile
  end

  def params(**kwargs)
    @params = kwargs
  end

  def cpp_params(**kwargs)
    @cpp_params = kwargs
  end

  def return_type(type)
    @return_type = type
  end

  def variant(name, &block)
    @current_variant = OpenStruct.new({condition: nil, name: name, func: nil})
    @variants << @current_variant
    self.instance_eval(&block)
  end

  def generate_ir(gen)
    @variants.each { |x| gen.generate_function(x.func) }

    param_list = @params.keys.map { |x| "[[maybe_unused]] AnyBaseType #{x}"} + @cpp_params.map { |name, type| "[[maybe_unused]] #{type} #{name}"}
    params = (["IntrinsicInst *inst"] + param_list).join(', ')
    Output.scoped_puts "inline #{@return_type} #{@name}(#{params}) {" do
      @variants.each do |variant|
        next if variant.cond == :default
        Output.scoped_puts "if (#{variant.cond}) {" do
          Output << "return #{variant.name}(inst);"
        end
      end
      defaults = @variants.select { |x| x.cond == :default }
      raise "Default condition can be only once" if defaults.size > 1
      if defaults.empty?
        Output << "return false;"
      else
        Output << "return #{defaults[0].name}(inst);"
      end

    end
  end
end