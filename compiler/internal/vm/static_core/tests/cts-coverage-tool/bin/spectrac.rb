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
# frozen_string_literal: true

# Huawei Technologies Co.,Ltd.
require 'optparse'
require 'yaml'
require 'json'
require 'ostruct'

Dir[File.join(__dir__, '..', 'lib', '*.rb')].each { |file| require file } # rubocop:disable Lint/NonDeterministicRequireOrder

def check_file(file)
  raise OptionParser::InvalidOption, "File #{file} not found" unless File.exist? File.expand_path(file)
end

def check_files(arr)
  raise OptionParser::InvalidOption, 'No ISA spec files found' if arr.length.zero?

  arr.each do |f|
    check_file(f)
  end
end

def check_dir(dir)
  raise OptionParser::InvalidOption, "Directory #{dir} not found." unless File.directory? File.expand_path(dir)
end

options = OpenStruct.new
optparser = OptionParser.new do |opts|
  opts.banner = 'Usage: spectrac.rb [options]'
  opts.on('-r', '--report [FILE]', 'Output the test coverage summary report in yaml')
  opts.on('-d', '--testdir DIR', 'Directory with the test files (required)')
  opts.on('-g', '--testglob GLOB', 'Glob for finding test files in testdir (required)')
  opts.on('-s', '--spec FILE1,FILE2,FILE3', Array, 'ISA spec file(s) (at least one required)')
  opts.on('-n', '--non_testable [FILE]', 'Non testable assertions')
  opts.on('-u', '--uncovered [FILE]', 'Output yaml document with ISA spec areas not covered by tests')
  opts.on('-U', '--uncovered_md [FILE]', 'Output markdown document with ISA spec areas not covered by tests')
  opts.on('-o', '--orphaned [FILE]', 'Output yaml file with the list of tests not relevant to the spec')
  opts.on('-O', '--orphaned_md [FILE]', 'Output markdown file with the list of tests not relevant to the spec')
  opts.on('-f', '--full [FILE]', 'Output spec file with additional coverage-specific fields in yaml')
  opts.on('-F', '--full_md [FILE]', 'Output spec file with additional coverage-specific fields in markdown')
  opts.on('-h', '--help', 'Prints this help') do
    puts opts
    exit
  end
end

begin
  optparser.parse!(into: options)

  # check that required arguments aren't missing
  missing = %i[spec testdir testglob].select { |param| options[param].nil? }
  raise OptionParser::MissingArgument, missing.join(', ') unless missing.empty?

  # check that specified paths are valid
  check_files(options.spec)
  check_dir(options.testdir)
  check_file(options.non_testable) if options.non_testable
rescue OptionParser::InvalidOption, OptionParser::MissingArgument => e
  puts e
  puts optparser
  exit false
end

spec = options.spec.map { |f| YAML.load_file(File.expand_path(f)) }

fullspec = Spec.new(spec)
fullspec.load_non_testable(YAML.load_file(File.expand_path(options.non_testable))) if options.non_testable
fullspec.load_tests(options.testdir, options.testglob)

summary = Summary.new(fullspec)
summary.compute

File.write(options.report, summary.report.to_yaml) if options.report
File.write(options.uncovered, summary.uncovered.to_yaml) if options.uncovered
File.write(options.full, fullspec.data.to_yaml) if options.full
File.write(options.orphaned, fullspec.orphaned.to_yaml) if options.orphaned

ReportMd.new(summary.report).generate
UncoveredMd.new(summary.uncovered).generate(options.uncovered_md) if options.uncovered_md
FullMd.new(fullspec.data).generate(options.full_md) if options.full_md
OrphanedMd.new(fullspec.orphaned).generate(options.orphaned_md) if options.orphaned_md
