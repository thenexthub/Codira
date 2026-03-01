#!/usr/bin/env ruby
# Copyright (c) 2026 Huawei Device Co., Ltd.
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

require 'optparse'
require 'ostruct'
require 'fileutils'
require 'open3'

options = {}

OptionParser.new do |opts|
  opts.banner = 'Usage: gen_pcre2_newline.rb --source SRC --patch PATCH --output STAMP'

  opts.on('--source PATH', 'Path to pcre2_newline.c') do |v|
    options[:source] = v
  end

  opts.on('--patch PATH', 'Path to pcre2_newline.patch') do |v|
    options[:patch] = v
  end

  opts.on('--output PATH', 'Path to build stamp (in out/)') do |v|
    options[:output] = v
  end
end.parse!

source = File.expand_path(options.fetch(:source))
patch  = File.expand_path(options.fetch(:patch))
output = File.expand_path(options.fetch(:output))

permanent_stamp = File.join(File.dirname(source), File.basename(output))

puts "gen_pcre2_newline:"
puts "  source           = #{source}"
puts "  patch            = #{patch}"
puts "  build stamp(out) = #{output}"
puts "  permanent stamp  = #{permanent_stamp}"

if File.exist?(permanent_stamp)
  FileUtils.mkdir_p(File.dirname(output))
  FileUtils.touch(output)
  puts "gen_pcre2_newline: permanent stamp exists, skip patch."
  exit 0
end

FileUtils.mkdir_p(File.dirname(output))

cmd = ['patch', '-N', source, patch]
puts "gen_pcre2_newline: apply patch: #{cmd.join(' ')}"

stdout, stderr, status = Open3.capture3(*cmd)

if status.success?
  FileUtils.touch(output)
  FileUtils.touch(permanent_stamp)
  puts "gen_pcre2_newline: patch applied successfully."
  puts "gen_pcre2_newline: stamps generated:"
  puts "  - #{output}"
  puts "  - #{permanent_stamp}"
  exit 0
end

warn "gen_pcre2_newline: patch failed, status=#{status.exitstatus}"
warn stdout unless stdout.empty?
warn stderr unless stderr.empty?
exit status.exitstatus
