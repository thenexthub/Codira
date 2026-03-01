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

class Regmap
  attr_reader :data

  def initialize(data, direct: false)
    if direct
      @data = data
    else
      @data = data[Options.arch]
      raise "Regmap doesn't contain data for required arch" unless @data
    end

  end

  def self.from_hash(data)
    Regmap.new(data, direct: true)
  end

  def [](v)
    @data[v]
  end

  def +(other)
    if other.is_a? Regmap
      Regmap.new(@data.merge(other.data), direct: true)
    elsif other.is_a? Hash
      Regmap.new(@data.merge(other), direct: true)
    else
      raise "Unsupported type: #{other.class}"
    end
  end

  def -(other)
    if other.is_a? Regmap
      Regmap.new(@data.select { |k, _| !other.data.key?(k) }, direct: true)
    elsif other.is_a? Hash
      Regmap.new(@data.select { |k, _| !other.key?(k) }, direct: true)
    else
      raise "Unsupported type: #{other.class}"
    end
  end

  def ==(other)
    if other.is_a? Regmap
      @data == other.data
    elsif other.is_a? Hash
      @data == other
    else
      false
    end
  end

  def to_s
    @data.to_s
  end

  def inspect
    @data.inspect
  end
end
