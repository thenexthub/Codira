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
require 'optparse'
require 'ostruct'
require 'generator'

def check_option(optparser, options, key)
  return if options[key]
  
  puts "Missing option: --#{key}"
  puts optparser
  exit false
end

options = OpenStruct.new
optparser = OptionParser.new do |opts|
  opts.banner = 'Usage: generate-tag.rb [options]'
  opts.on('-t', '--template FILE', 'Path to template erb file to generate tests (required)')
  opts.on('-y', '--yaml FILE', 'Path to template yaml file to generate tests (required)')
  opts.on('-i', '--isapi ISAPI_FILE', 'Path to isapi.rb file to use isapi (required)')
  opts.on('-o', '--output DIR', 'Path to directory where tests will be generated (required)')
  opts.on('-s', '--isa FILE', 'Path to isa.rb file to connect with api (required)')
  opts.on('-h', '--help', 'Prints this help') do
    puts opts
    exit
  end
end

optparser.parse!(into: options)
check_option(optparser, options, 'template')
check_option(optparser, options, 'yaml')
check_option(optparser, options, 'isapi')
check_option(optparser, options, 'output')
check_option(optparser, options, 'isa')

isa_path = options['isa']
require_relative isa_path

isapi_path = options['isapi']
yaml_path = options['yaml']
ISA.setup(yaml_path, isapi_path)
require 'tag-isapi'

template = File.read(options['template'])
generator = GeneratorTags::Generator.new(template)

Panda.instructions.each do |instr|
  next if generator.skip?(instr)
  
  new_file = File.open("#{options['output']}/tag_#{instr.opcode}.pa", 'w+')
  generator.generate_file(new_file, instr)
  new_file.close
end
