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
require 'ostruct'

class Entrypoint < OpenStruct
  def initialize(dscr)
    super(dscr)
    if self.entrypoint.nil? && !self.external?
      raise "Entrypoint description must contains 'entrypoint' field: #{dscr.marshal_dump}"
    end
    abort "ERROR: 'bridge' field must be specified for #{self.entrypoint}" unless dscr['bridge'] # !self.respond_to? 'bridge'
  end

  def enum_name
    self.name.snakecase.upcase
  end

  def bridge_name
    return nil unless self.has_bridge?
    "#{self.name}Bridge"
  end

  def has_bridge?
    return !self.bridge.nil? && self.bridge != 'none'
  end

  def external?
    has_property? 'external'
  end

  def has_property? prop
    self.properties&.include? prop
  end

  def return_type
    self.signature[0]
  end

  def general_return_type
    ret = self.return_type
    return 'VOID' if ret == 'void'
    return 'FLOAT' if ['float', 'double'].include? ret
    return 'INTEGER'
  end

end

module Compiler
  module_function

  def entrypoints
    @entrypoints ||= @data['entrypoints'].map {|x| Entrypoint.new x }
  end

  def entrypoints_crc32
    require "zlib"
    Zlib.crc32(entrypoints.map(&:signature).join)
  end

  def environment_checksum(cross_values_h)
    require "zlib"
    cross_values = File.read(cross_values_h)
    cross_values_crc32 = Zlib.crc32(cross_values)
    combined_crc32 = Zlib.crc32_combine(Compiler::entrypoints_crc32, cross_values_crc32, cross_values.length)
  end

  def wrap_data(data)
    @data = data
  end
end

def Gen.on_require(data)
  Compiler.wrap_data(data)
end
