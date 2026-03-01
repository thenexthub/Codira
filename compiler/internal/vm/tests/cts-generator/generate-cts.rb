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

# Huawei Technologies Co.,Ltd.

# frozen_string_literal: true

require 'yaml'
require 'logger'
require 'optparse'
require 'ostruct'

require_relative 'generator/generator'

def check_option(optparser, options, key)
  return if options[key]

  puts "Missing option: --#{key}"
  puts optparser
  exit false
end

def get_chunk(tests, chunk, chunks)
  chunk_size = tests.length / chunks
  if chunk_size > tests.length || chunk_size < 1 || chunk.negative?
    raise "Chunk arguments are not valid: #{chunk} of #{chunks}"
  end

  result = []
  last_idx = chunk >= chunks - 1 ? tests.length : (chunk + 1) * chunk_size
  i = chunk * chunk_size
  while i < last_idx
    result.push(tests[i])
    i += 1
  end
  result
end

options = OpenStruct.new
options[:chunk] = 0
options[:chunks] = 1

optparser = OptionParser.new do |opts|
  opts.banner = 'Usage: generate-cts.rb [options]'
  opts.on('-t', '--template FILE', 'Path to template yaml file to generate tests (required)')
  opts.on('-s', '--schema FILE', 'Path to json schema for template yaml (required)')
  opts.on('-k', '--skip', 'Skip yaml schema validation')
  opts.on('-o', '--output DIR', 'Path to directory where tests will be generated (required)')
  opts.on('--skip-header', 'Do not generate test headers')
  opts.on('-f', '--file FILE', 'Generate tests from the specified yaml file only, for example: fmod2.64.yaml')
  opts.on('--chunk INTEGER', Integer, 'Chunk to process, starting from 0 (0 by default)')
  opts.on('--chunks INTEGER', Integer, 'Number of chunks (1 by default)')
  opts.on('-h', '--help', 'Prints this help') do
    puts opts
    exit
  end
end

optparser.parse!(into: options)
check_option(optparser, options, 'template')
check_option(optparser, options, 'schema')
check_option(optparser, options, 'output')
template_path = options['template']
schema_path = options['schema']
output = options['output']
skip = options['skip']
skip_header = options['skip-header']
chunk = options['chunk']
chunks = options['chunks']
file = options['file']

LOG = Logger.new(STDOUT)
LOG.level = Logger::DEBUG

LOG.info "Loading '#{template_path}'"

data = YAML.load_file(template_path)

data['tests'] = get_chunk data['tests'], chunk, chunks

data['tests'] = [{"include" => "#{file}"}] if file

generator = Generator::Parser.new data, output, File.dirname(template_path), skip_header

# Validate test template

unless skip
  require 'json-schema'
  res = JSON::Validator.fully_validate(schema_path, data)
  unless res.empty?
    puts "Template '#{template_path}' contains several errors:"
    puts res
    raise 'Schema validation error, please update template to match schema to generate tests'
  end
end

generator.parse skip
