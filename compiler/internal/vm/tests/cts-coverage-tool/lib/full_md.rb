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

# frozen_string_literal: true

#
# Huawei Technologies Co.,Ltd.
require 'optparse'
require 'yaml'
require 'erb'

class FullMd
  attr_accessor :full, :properties_hash, :exceptions_hash, :verification_hash

  def initialize(spec)
    @template_file = File.join(__dir__, 'templates', 'full_md.erb')
    @full = spec
    @exceptions_hash = convert_to_hash(@full['exceptions'])
    @properties_hash = convert_to_hash(@full['properties'])
    @verification_hash = convert_to_hash(@full['verification'])
  end

  def generate(file)
    File.open(file, 'w+') do |f|
      f.write(render)
    end
  end

  private

  def format_array(instr)
    instr.nil? ? '' : instr['format'].join(', ').gsub(/_/, '\\_')
  end

  def covered_description(desc)
    non_testable = desc['non_testable'] ? ' - Non-testable' : ''
    "#{md(desc['assertion'])} [#{test_list(desc['tests'])}]#{non_testable}"
  end

  def props(properties)
    "[#{md(properties.join(', '))}]"
  end

  def verification_entry(ver)
    "#{md(@verification_hash[ver['verification']])} [#{test_list(ver['tests'])}]"
  end

  def exception_entry(entry)
    "#{md(@exceptions_hash[entry['exception']])} [#{test_list(entry['tests'])}]"
  end

  def test_list(tests)
    tests.any? ? tests.join(', ') : '\`not covered\`'
  end

  def md(str)
    str.gsub(/_/, '\\_').gsub(/\n/, ' ').rstrip
  end

  def render
    @template = File.read(@template_file)
    ERB.new(@template, nil, '%-').result(binding)
  end

  def convert_to_hash(arr)
    arr.map { |i| [i['tag'], i['description']] }.to_h
  end
end
