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

require 'optparse'
require 'yaml'
require 'json'
require 'erb'
require 'ostruct'

# Extend Module to implement a decorator for ISAPI methods
class Module
  def cached(method_name)
    definer = instance_methods.include?(method_name) ? :define_method : :define_singleton_method
    noncached_method = instance_method(method_name)
    send(definer, method_name) do
      unless instance_variable_defined? "@#{method_name}"
        instance_variable_set("@#{method_name}", noncached_method.bind(self).call)
      end
      instance_variable_get("@#{method_name}").freeze
    end
  end
end

# Gen.on_require will be called after requiring scripts.
# May (or even should) be redefined in scripts.
module Gen
  def self.on_require(data); end
end

def create_sandbox
  # nothing but Ruby core libs and 'required' files
  binding
end

def check_option(optparser, options, key)
  return if options[key]

  puts "Missing option: --#{key}"
  puts optparser
  exit false
end

def major_minor_version
  major, minor, = RUBY_VERSION.split('.').map(&:to_i)
  [major, minor]
end

def check_version(min_major, min_minor)
  major, minor = major_minor_version
  major > min_major || (major == min_major && minor >= min_minor)
end

def erb_new(str, trim_mode: nil)
  if check_version(2, 6)
    ERB.new(str, trim_mode: trim_mode)
  else
    ERB.new(str, nil, trim_mode)
  end
end

raise "Update your ruby version, #{RUBY_VERSION} is not supported" unless check_version(2, 5)

options = OpenStruct.new

optparser = OptionParser.new do |opts|
  opts.banner = 'Usage: gen.rb [options]'

  opts.on('-t', '--template FILE', 'Template for file generation (required)')
  opts.on('-d', '--data FILE1,FILE2,...', Array, 'Source data files in YAML format')
  opts.on('-q', '--api FILE1,FILE2,...', Array, 'Ruby files providing api for data in YAML format')
  opts.on('-o', '--output FILE', 'Output file (default is stdout)')
  opts.on('-a', '--assert FILE', 'Go through assertions on data provided and exit')
  opts.on('-r', '--require foo,bar,baz', Array, 'List of additional Ruby files to be required for generation')
  opts.on('--target [arch]', 'Currently selected target')

  opts.on('-h', '--help', 'Prints this help') do
    puts opts
    exit
  end
end
optparser.parse!(into: options)

# export the current panda target for the scripts that need it
$selected_target = options['target'].sub('x86_64', 'amd64').sub('aarch64', 'arm64') if options['target']

check_option(optparser, options, :data)
check_option(optparser, options, :api)
raise "'api' and 'data' must be of equal length" if options.api.length != options.data.length
options.api.zip(options.data).each do |api_file, data_file|
  if Gem::Version.new(RUBY_VERSION) < Gem::Version.new('3.1.0')
    data = YAML.load_file(File.expand_path(data_file))
  else
    data = YAML.load_file(File.expand_path(data_file), aliases: true)
  end
  data = JSON.parse(data.to_json, object_class: OpenStruct)
  require File.expand_path(api_file)
  Gen.on_require(data)
  data.freeze
end
options.require&.each { |r| require File.expand_path(r) }

if options.assert
  require options.assert
  exit
end

check_option(optparser, options, :template)
template = File.read(File.expand_path(options.template))
output = options.output ? File.open(File.expand_path(options.output), 'w') : $stdout
t = erb_new(template, trim_mode: '%-')
t.filename = options.template
output.write(t.result(create_sandbox))
output.close
