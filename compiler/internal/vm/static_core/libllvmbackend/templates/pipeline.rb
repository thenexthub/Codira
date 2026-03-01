#!/usr/bin/env ruby
# Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

require "ostruct"
require 'optparse'

options = OpenStruct.new

optparser = OptionParser.new do |opts|
  opts.banner = 'Usage: gen.rb [options]'

  opts.on('-d', '--data FILE', 'Source data (required)')
  opts.on('-o', '--output FILE', 'Output file')
  opts.on('-s', '--suffix STRING', 'Suffix for generated variable')

  opts.on('-h', '--help', 'Prints this help') do
    puts opts
    exit
  end
end
optparser.parse!(into: options)

lines = File.readlines(options.data)
lines.map! {|line| line.gsub(/#.*/, '').strip }
lines = lines.join("")
lines = "constexpr std::string_view PIPELINE#{options.suffix} = R\"__MAGIC__(#{lines})__MAGIC__\";"

output = options.output ? File.open(File.expand_path(options.output), 'w') : $stdout
output.write(lines)
output.close
