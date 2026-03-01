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
require 'delegate'

class Level
  attr_reader :name
  attr_reader :value
  attr_reader :enum
  attr_reader :tag

  def initialize(dscr)
     @dscr = dscr
     @name = dscr['name']
     @value = dscr['value']
     @enum = dscr['enum'] || @name.upcase
     @tag = dscr['tag'] || @name[0].upcase
  end
end

class Component
  attr_reader :name
  attr_reader :enum

  def initialize(dscr)
    @dscr = dscr
    @name = dscr['name']
    @enum = dscr['enum'] || @name.upcase
  end
end

module LoggerData
  module_function

  def levels
    @levels
  end

  def components
    @components
  end

  def dfx_components
    @dfx_components
  end

  def wrap_data(data)
    @data = data
    @levels = data.levels.map { |op| Level.new(op) }
    @components = data.components.map { |op| Component.new(op) }
    @dfx_components = data.dfx_components.map { |op| Component.new(op) }
  end
end

def Gen.on_require(data)
  LoggerData.wrap_data(data)
end
