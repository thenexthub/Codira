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
require 'ostruct'
require 'yaml'

options = OpenStruct.new
optparser = OptionParser.new do |opts|
  opts.banner = 'Usage: gen_ruby.rb [options]'

  opts.on('-d', '--data FILE', 'Logger data file in YAML format')
  opts.on('-p', '--plugins FILE', 'Plugins data file in YAML format')
  opts.on('-o', '--output FILE', 'Output file (default is stdout)')

  opts.on('-h', '--help', 'Prints this help') do
    puts opts
    exit
  end
end
optparser.parse!(into: options)

exit unless options.data
exit unless options.plugins

data = YAML.load_file(File.expand_path(options.data))
plugins_data = YAML.load_file(File.expand_path(options.plugins))

if plugins_data['plugins']
  plugins_data['plugins'].each do |plugins|
    name = plugins.keys.first
    plugin_data = plugins[name]['logger']
    data['components'] += plugin_data['components'] if plugin_data
  end
end

output = options.output ? File.open(File.expand_path(options.output), 'w') : $stdout
output.write(data.to_yaml)
output.close
