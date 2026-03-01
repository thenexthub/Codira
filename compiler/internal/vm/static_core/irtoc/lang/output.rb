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


class Output
  @@indent = 0
  @@indent_str = ""
  @@file = nil
  STEP = 4

  def self.setup(file = nil)
    @@file = file.nil? ? $stdout : File.open(file, 'w')
  end

  def self.write(line)
    @@file.write line
  end

  def self.println(line)
    @@file.write @@indent_str
    @@file.puts line
  end

  def self.puts(line = '')
    println(line)
  end

  def self.<<(line)
    println(line)
    self
  end

  def self.printlni(line)
    println line
    indent_up
  end

  def self.printlnd(line)
    indent_down
    println line
  end

  def self.scoped_puts(start_line, end_line = '}')
    puts start_line
    indent_up
    yield
    indent_down
    puts end_line
  end

  def self.indent_up
    change_indent(STEP)
  end
  def self.indent_down
    change_indent(-STEP)
  end

private
  def self.change_indent(step = 4)
    @@indent += step
    raise "Wrong indent" if @@indent < 0
    @@indent_str = " " * @@indent
  end
end
