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

require_relative 'regmap'

# RegMask class is like bitset
class RegMask
  attr_reader :value, :regmap

  def initialize(*args)
    @value = 0
    if Options.arch == :arm64
      @size = 31
    elsif Options.arch == :arm32
      @size = 16
    elsif Options.arch == :x86_64
      @size = 16
    end
    return if args.empty?

    if args[0].is_a?(Regmap)
      @regmap = args[0]
      args = args[1..-1]
    elsif args[0].nil?
      args = args[1..-1]
    elsif args[0].is_a?(RegMask)
      @regmap = args[-0].regmap
      @value = args[0].value
      return
    end

    args.each do |value|
      if value.is_a?(Symbol)
        raise "RegMask is initialized with symbol, but Regmap wasn't specified" unless @regmap
        raise "Regmap doesn't contain value for #{value}" unless @regmap.data.keys.include? value
        value = @regmap[value]
      end
      raise "RegMask is initialized with wrong type: #{value.class}" unless value.is_a?(Integer)
      self[value] = true
    end
  end

  def self.from_value(regmap, value)
    mask = RegMask.new(regmap)
    mask.set_value(value)
    mask
  end

  def self.from_regmap(regmap, map)
    mask = RegMask.new(regmap)
    map.data.each { |_, v| mask[v] = true }
    mask
  end

  def set_value(value)
    raise "RegMask is initialized with wrong type: #{value.class}" unless value.is_a?(Integer)
    @value = value
  end

  def []=(position, value)
    position = @regmap[position] if position.is_a? Symbol
    raise "Wrong `position` type" unless position.is_a? Integer
    if value
      @value |= (1 << position)
    else
      @value &= ~(1 << position)
    end
  end

  def [](position)
    position = @regmap[position] if position.is_a? Symbol
    raise "Wrong `position` type" unless position.is_a? Integer
    @value & (1 << position) != 0
  end

  def +(other)
    if other.is_a? RegMask
      self.class.from_value(@regmap, @value | other.value)
    elsif other.is_a? Regmap
      other_value = other.data.values.inject(0) { |res, x| res | (1 << x) }
      self.class.from_value(@regmap, @value | other_value)
    elsif other.is_a? Integer
      self.class.from_value(@regmap, @value | (1 << other))
    elsif other.is_a? Symbol
      raise "Symbol argument is only allowed if Regmap was specified" unless @regmap
      value = @regmap[other]
      raise "Register '#{other}' is not found in regmap" unless value
      self.class.from_value(@regmap, @value | (1 << value))
    else
      raise "Unsupported type: #{other.class}"
    end
  end

  def -(other)
    if other.is_a? RegMask
      self.class.from_value(@regmap, @value & ~other.value)
    elsif other.is_a? Regmap
      other_value = other.data.values.inject(0) { |res, x| res | (1 << x) }
      self.class.from_value(@regmap, @value & ~other_value)
    elsif other.is_a? Integer
      self.class.from_value(@regmap, @value & ~(1 << other))
    elsif other.is_a? Symbol
      raise "Symbol argument is only allowed if Regmap was specified" unless @regmap
      value = @regmap[other]
      raise "Register '#{other}' is not found in regmap" unless value
      self.class.from_value(@regmap, @value & ~(1 << value))
    else
      raise "Unsupported type: #{other.class}"
    end
  end

  def ~
    value = ~@value & ((1 << @size) - 1)
    self.class.from_value(@regmap, value)
  end

  def ==(other)
    @value == other.value
  end

  def each(&block)
    if block
      (0..@size - 1).select { |x| self[x] }.each { |x| yield x }
    else
       (0..@size - 1).select { |x| self[x] }
    end
  end

  def to_s
    "[#{each.join(', ')}]"
  end
end
