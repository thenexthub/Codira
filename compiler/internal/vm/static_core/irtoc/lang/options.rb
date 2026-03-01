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

require 'optparse'
require 'ostruct'
require 'yaml'

Options = OpenStruct.new
class << Options

  attr_accessor :compiling

  def parse
    OptionParser.new do |opts|
      opts.banner = "Usage: irtoc.rb [options] INPUT_FILE"

      opts.on("--ark_source_dir=PATH", "Path to panda source code") { |v| self.ark_source_dir = v }
      opts.on("--output=PATH", "Output file") { |v| self.output_files = v.split(':') }
      opts.on("--input=PATH", "Input files, separated by ':' symbol") { |v| self.input_files = v.split(':') }
      opts.on('-D=STR', 'C++ definitions that will be used for compiling output file') { |v| (self.definitions ||= []) << v }
      opts.on("--arch=ARCH", "Target architecture") { |v| self.arch = v }
      opts.on('--cpu-features=FEATURE,FEATURE', 'Supported CPU Features') { |v| self.cpu_features = v.split(',').map { |f| f.to_sym }}
      opts.on('--ir-api=API', 'API to emit during C++ code generation') { |v| self.ir_api = v }
      opts.on('--isa=PATH', 'ISA YAML file') { |v| self.isa = v }
      opts.on('--plugins=PATH', 'Plugins file') { |v| self.plugins = v }
      opts.on('--working-dir=PATH', 'Run in specified working directory') { |v| self.working_dir = v }
    end.parse!

    if Options.working_dir
      Dir.mkdir(Options.working_dir) unless File.exist?(Options.working_dir)
      Dir.chdir(Options.working_dir)
    end

    self.instructions_yaml = "#{self.ark_source_dir}/compiler/optimizer/ir/instructions.yaml"
    self.isapi = "#{self.ark_source_dir}/isa/isapi.rb"
    self.commonapi = "#{self.ark_source_dir}/templates/common.rb"

    # Collect compiler definitions
    if self.definitions
      new_definitions = OpenStruct.new
      self.definitions.each do |define|
        vals = define.split('=')
        new_definitions[vals[0]] = vals.size > 1 ? vals[1] : true
      end
      self.definitions = new_definitions
    else
      self.definitions = OpenStruct.new
    end
    self.definitions.DEBUG = !self.definitions.NDEBUG

    # Collect plugins files
    if self.plugins
      new_plugins = Hash.new { |h, k| h[k] = [] }
      File.open(self.plugins).readlines.each do |plugin|
        next if plugin.strip.empty?
        full_plugin_path = "#{self.ark_source_dir}/#{plugin}".chop
        line_matches = File.open(full_plugin_path).to_a.first.match(/# +plugin +(.*)/)
        raise "Irtoc plugin doesn't specifiy its name: #{full_plugin_path}" unless line_matches
        plugin_name = line_matches[1]
        new_plugins[plugin_name] << full_plugin_path
      end
      self.plugins = new_plugins
    else
      self.plugins = Hash.new { |h, k| h[k] = [] }
    end

    # Determine target architecture
    if self.arch.nil?
      arch_str = self.definitions.marshal_dump.keys.grep(/PANDA_TARGET_(AMD64|ARM64|ARM32)/)[0]
      self.arch = arch_str.to_s.sub('PANDA_TARGET_','').downcase
    end
    self.arch = self.arch.to_sym
    if self.arch == :amd64 || self.arch == :x64
      self.arch = :x86_64
    end
    if self.arch == :arm
      self.arch = :arm32
    end
    possible_arch = %w[arm64 arm32 x86_64 x86]
    raise "Wrong arch: #{arch_str}" unless possible_arch.include?(self.arch.to_s)

    # Read compiler arch info
    arch_info = YAML.load_file("#{self.ark_source_dir}/compiler/optimizer/ir/instructions.yaml")['arch_info']
    raise "Compiler config doesn't contain `arch_info`" unless arch_info
    arch_info = arch_info[arch_info.index { |x| x['name'].to_sym == self.arch }]
    raise "Arch info not found for #{self.arch}" unless arch_info
    self.arch_info = OpenStruct.new(arch_info)

    # Determine CPU features. Notice: default-enabled CPU features ('sse42') must be reflected here.
    # See the 'compiler/compiler.yaml' file.
    if self.cpu_features.nil?
      self.cpu_features = [:sse42]
    end
    possible_features = %w[none crc32 sse42 jscvt atomics]
    self.cpu_features.each do |feature|
      raise "Wrong cpu feature: #{feature}" unless possible_features.include?(feature.to_s)
    end

    raise 'Supported IR APIs: ir-constructor, ir-builder, ir-inline' unless self.ir_api =~ /^ir-(constructor|builder|inline)$/
  end

  def arch_64_bits?
    self.arch == :x86_64 || self.arch == :arm64
  end

  def arm64?
    self.arch == :arm64
  end

  def cpp_arch
    @cpp_arch_map ||= {
      arm32: "Arch::AARCH32",
      arm64: "Arch::AARCH64",
      x86_64: "Arch::X86_64"
    }
    @cpp_arch_map[self.arch]
  end

  def crc32_feature_enabled?
    self.cpu_features.include?(:crc32)
  end

  def sse42_feature_enabled?
    self.cpu_features.include?(:sse42)
  end

  def jscvt_feature_enabled?
    self.cpu_features.include?(:jscvt)
  end

  def atomics_feature_enabled?
    self.cpu_features.include?(:atomics)
  end

end
