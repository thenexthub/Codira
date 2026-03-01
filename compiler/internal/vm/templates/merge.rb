#!/usr/bin/env ruby
# Copyright (c) 2022-2022 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Huawei Technologies Co.,Ltd.

require 'json'
require 'optparse'
require 'set'
require 'yaml'

def check_option(optparser, options, key)
  return if options[key]

  puts "Missing option: --#{key}"
  puts optparser
  exit false
end

def check_version
  major, minor, = RUBY_VERSION.split('.').map(&:to_i)
  major > 2 || (major == 2 && minor >= 5)
end

raise "Update your ruby version, #{RUBY_VERSION} is not supported" unless check_version

options = OpenStruct.new

optparser = OptionParser.new do |opts|
  opts.banner = 'Usage: merge.rb [options]'

  opts.on('-d', '--data foo.yaml,bar.yaml,baz.yaml', Array, 'List of source data in YAML format (required)')
  opts.on('-o', '--output FILE', 'Output file (default is stdout)')

  opts.on('-h', '--help', 'Prints this help') do
    puts opts
    exit
  end
end
optparser.parse!(into: options)

check_option(optparser, options, :data)

# Merge yamls into 'options_hash'
options_hash = Hash.new("")
options.data.each do |options_yaml|
  data = YAML.load_file(File.expand_path(options_yaml))
  data = JSON.parse(data.to_json)
  data["options"].each do |option|
    name = option["name"]
    if !options_hash.has_key?(name)
      options_hash[name] = option
      next
    end

    assert_eq = -> (key) {
      raise "Option '#{name}' conflicts for key '#{key}'" unless options_hash[name][key] == option[key]
    }

    merge_arrays = -> (key) {
      options_hash[name][key] = (Set.new(options_hash[name][key]) + Set.new(option[key])).to_a
    }

    assert_eq.call("type")
    assert_eq.call("description")

    if !option["possible_values"].nil?
      merge_arrays.call("possible_values")
    end

    if !option["lang"].nil?
      merge_arrays.call("lang")
    end

    if !option["default"].nil?
      if option["type"] == "arg_list_t"
        merge_arrays.call("default")
      else
        assert_eq.call("default")
      end
    end
  end
end

# Dump resulted 'options_hash' to yaml output
data = YAML.load_file(File.expand_path(options.data[0]))
data = JSON.parse(data.to_json)
data["options"] = options_hash.values
output = options.output ? File.open(File.expand_path(options.output), 'w') : $stdout
output_yaml = YAML.dump(JSON.load(data.to_json))
output.write(output_yaml)
output.close
