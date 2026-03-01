#!/usr/bin/env ruby
# Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

require 'optparse'
require 'ostruct'
require 'logger'
require 'fileutils'
require 'open3'

module BuildConfig
  module Static
    OBJECT_HEADER_SIZE = 8
    MANAGED_REFERENCE_SIZE = 4
  end

  module Hybrid
    OBJECT_HEADER_SIZE = 16
    MANAGED_REFERENCE_SIZE = 8
  end

  CONFIGS = [Static, Hybrid]
  CONFIGS_BY_NAME = CONFIGS.map {|c| [c.name.split('::')[1], c]}.to_h
  CONFIGS_NAMES = CONFIGS_BY_NAME.map {|k, v| k}
end

options = OpenStruct.new
OptionParser.new do |opts|
  opts.banner = 'Usage: checker.rb [options] TEST_FILE'

  opts.on('--run-prefix=PREFIX', 'Prefix that will be inserted before panda run command') do |v|
    v = v.gsub ',', ' '
    options.run_prefix = v.eql?("\'\'") ? "" : v
  end
  opts.on('--source=FILE', 'Path to source file')
  opts.on('--test-dir=DIR', 'Path to test directory') do |v|
    options.test_dir = v
  end
  opts.on('--test-file=FILE', 'Path to test file') do |v|
    options.test_file = v
  end
  opts.on('--panda=PANDA', 'Path to panda')
  opts.on('--paoc=PAOC', 'Path to paoc') do |v|
    options.paoc = v
  end
  opts.on('--frontend=FRONTEND', 'Path to frontend binary')
  opts.on('--panda-options=OPTIONS', 'Default options for panda run') do |v|
    options.panda_options = v.gsub ',', ' '
  end
  opts.on('--paoc-options=OPTIONS', 'Default options for paoc run') do |v|
    options.paoc_options = v.gsub ',', ' '
  end
  opts.on('--frontend-options=OPTIONS', 'Default options for frontend+bco run') do |v|
    options.frontend_options = v.gsub ',', ' '
  end
  opts.on('--method=METHOD', 'Method to optimize')
  opts.on('--command-token=STRING', 'String that is recognized as command start') do |v|
    options.command_token = v
  end
  opts.on('--release', 'Run in release mode. EVENT, INST and other will not be checked')
  opts.on('-v', '--verbose', 'Verbose logging')
  opts.on('--arch=ARCHITECTURE', 'Architecture of system where start panda')
  opts.on('--build-config=CONFIG', BuildConfig::CONFIGS_NAMES, "Build configuration to use: #{BuildConfig::CONFIGS_NAMES.join(', ')}") do |k|
      options.build_config = BuildConfig::CONFIGS_BY_NAME[k]
  end
  opts.on("--keep-data", "Do not remove generated data from disk") { |v| options.keep_data = true }
  opts.on("--with-llvm", "Tells checker that ARK was built with LLVM support") do |v|
    options.with_llvm = true
  end
  opts.on('--checker-filter=STRING', 'Run only checkers with filter-matched name') do |v|
    options.checker_filter = v
  end
  opts.on('--is-llvm=ISLLVM', 'Uranner option if --with-llvm should be added') do |v|
    options.with_llvm = v.eql?("true") ? true : false
  end
  opts.on('--is-verbose=ISVERBOSE', 'Uranner option if --verbose should be added') do |v|
    options.verbose = v.eql?("true") ? true : false
  end
  opts.on('--is-release=ISRELEASE', 'Uranner option if --release should be added') do |v|
    options.release = v.eql?("true") ? true : false
  end
  opts.on('--interop', 'Do interop-specific actions')
    options.interop = true
end.parse!(into: options)
$LOG_LEVEL = options.verbose ? Logger::DEBUG : Logger::ERROR
$curr_cmd = nil
$checker_counter = 0

def log
  @log ||= Logger.new($stdout, level: $LOG_LEVEL)
end

def raise_error(msg)
  log.error "Test failed: #{$checker_name}"
  if !$current_method.nil?
    log.error "Method: \"#{$current_method}\""
  end
  if !$current_pass.nil?
    log.error $current_pass
  end
  log.error msg
  log.error "Command to reproduce: #{$curr_cmd}"
  raise msg
end

def match_str(match)
  match.is_a?(Regexp) ? "/#{match.source}/" : match
end

def contains?(str, match)
  return str =~ match if match.is_a? Regexp

  raise_error "Wrong type for search: #{match.class}" unless match.is_a? String
  str.include? match
end

# Contains info about frontend usage
module FrontendData
  class << self
    attr_accessor :frontendelements, :checker_frontend
    
    def reset
      @frontendelements = {}
      @checker_frontend = {}
    end
  end
end

# Provides methods to search lines in a given array
class SearchScope

  attr_reader :lines
  attr_reader :init_lines
  attr_reader :current_index

  def initialize(lines, name)
    @init_lines = lines
    @lines = lines
    @name = name
    @current_index = 0
  end

  def find_method_dump(match)
    @lines = @lines.drop(@current_index)
    @current_index = 0
    find(match)
    @lines = @lines.drop(@current_index - 1)
    @current_index = 0
    find(/}$/)
    @lines = @lines.slice(0, @current_index)
    @current_index = 0
  end

  def find_block(match, nth = 1)
    @lines = @init_lines
    @current_index = 0
    i = nth
    begin
      while i > 0 do
        find(match)
        i -= 1
        @current_index = @current_index - 1 if i == 0
        @lines = @lines.drop(@current_index)
        @current_index = 0
        if i == 0
#          log.debug "Block found: #{@lines[@current_index]}, #{@lines[@current_index + 1]}"
          find(/succs:/)
          @lines = @lines.slice(0, @current_index)
          @current_index = 0
#        elsif
#          log.debug "Block found by regexp: #{match_str(match)}, #{i} matches to find"
        end
      end
    rescue RuntimeError
      raise_error "Block not found by regexp: #{match_str(match)} , #{i} matches to find"
    end
  end

  def self.from_file(fname, name)
    SearchScope.new(File.readlines(fname), name)
  end

  def find(match)
    return if match.nil?
    @current_index = @lines.index { |line| contains?(line, match) }
    raise_error "#{@name} not found: #{match_str(match)}" if @current_index.nil?
    @current_index += 1
  end

  def find_next(match)
    return if match.nil?

    index = @lines.drop(@current_index).index { |line| contains?(line, match) }

    raise_error "#{@name} not found: #{match_str(match)}" if index.nil?
    @current_index += index + 1
  end

  def find_not(match)
    return if match.nil?

    @lines.each do |line|
      raise_error "#{@name} should not occur: #{match_str(match)}" if contains?(line, match)
    end
  end

  def find_next_not(match)
    return if match.nil?

    @lines.drop(@current_index).each do |line|
      raise_error "#{@name} should not occur: #{match_str(match)}" if contains?(line, match)
    end
  end

  def to_s
    "Scope '#{@name}', current=#{@current_index}\n#{@lines.join}"
  end
end

class Checker
  protected
  attr_writer :name
  attr_accessor :aot_mode

  public
  attr_reader :name
  attr_reader :code

  module AotMode
    PAOC = 1
    LLVM = 2
    ALL  = 3
  end

  Code = Struct.new('Code', :source, :filename, :line_no)

  def initialize(options, name, line_no: 0)
    @name = name
    @code = Code.new('', options.source, line_no)
    @cwd = "#{Dir.getwd}/#{name.gsub(/[ -:()<>]/, '_')}"
    @options = options
    @args = ''
    @ir_files = []
    @architecture = options.arch
    @build_config = options.build_config
    @profdata_file = nil
    @aot_file = ''
    @aot_mode = nil

    # Events scope for 'events.csv'
    @events_scope = nil
    # IR scope for IR dumps files 'ir_dump/*.ir'
    @ir_scope = nil

    # Disassembly file lines, that were read from 'disasm.txt'
    @disasm_lines = nil
    # Currently processing disasm method
    @disasm_method_scope = nil
    # Current search scope
    @disasm_scope = nil

    @run_idx = 0
  end

  def init_run
    Dir.mkdir(@cwd) unless File.exist?(@cwd)
    if @options.interop
      # module directory need only for interop with ArkJSVM
      module_dir_path = File.join(@cwd, "module")
      Dir.mkdir(module_dir_path) unless Dir.exist?(module_dir_path)
    end
    clear_data
  end

  def clone_with(name:, aot_mode:)
    that = clone
    that.aot_mode = aot_mode
    that.name = name
    that
  end

  def populate
    checks = []
    if @aot_mode == AotMode::ALL
      checks << clone_with(aot_mode: AotMode::PAOC, name: "#{@name} [PAOC]")
      checks << clone_with(aot_mode: AotMode::LLVM, name: "#{@name} [LLVM]")
    else
      checks << self
    end
    checks
  end

  def match_filter?
    @options.checker_filter.nil? or @name.match? @options.checker_filter
  end

  def is_hybrid_config?
    @options.build_config == BuildConfig::Hybrid
  end

  def append_line(line)
    @aot_mode = AotMode::LLVM if line.include? "RUN_AOT"
    @code.source << line + "\n"
  end

  def RUN(**args)
    expected_result = 0
    aborted_sig = 0
    entry = '_GLOBAL::main'
    env = ''
    @args = []
    args.each do |name, value|
      case name
      when :force_jit
        next unless value
        @args << '--compiler-hotness-threshold=0 --no-async-jit=true --compiler-enable-jit=true'
      when :force_profiling
        next unless value
        @args << '--profiler-enabled=true --compiler-profiling-threshold=0 --compiler-enable-jit=false'
      when :pgo_emit_profdata
        next unless value
        @profdata_file = "#{@cwd}/#{File.basename(@options.test_file, File.extname(@options.test_file))}.profdata"
        @args << "--profilesaver-enabled=true --profile-output=#{@profdata_file}"
      when :options
        @args << value
      when :entry
        entry = value
      when :result
        expected_result = value
      when :abort
        aborted_sig = value
      when :env
        env = value
      when :aot_file
        @aot_file = value
      end
    end
    raise ":abort and :result cannot be set at the same time, :abort = #{aborted_sig}, :result = #{expected_result}" if aborted_sig != 0 && expected_result != 0

    clear_data
    @args = @args.join(' ')
    aot_arg = @aot_file.empty? ? '' : "--aot-file #{@aot_file}"

    compiler_dump = @options.interop ? "--compiler-dump:folder=./ir_dump" : "--compiler-dump"

    cmd = "#{@options.run_prefix} #{@options.panda} --compiler-queue-type=simple --compiler-ignore-failures=false #{@options.panda_options} \
                #{aot_arg} #{@args} --events-output=csv #{compiler_dump} --compiler-disasm-dump:single-file #{@options.test_file} #{entry}"
    $curr_cmd = "#{env} #{cmd}"
    log.debug "Panda command: #{$curr_cmd}"

    # See note on exec in RUN_PAOC
    output, status = Open3.capture2e("#{env} exec #{cmd}", chdir: @cwd.to_s)
    if aborted_sig != 0 && !status.signaled?
      puts output
      log.error "Expected ark to abort with signal #{aborted_sig}, but ark did not signal"
      raise_error "Test '#{@name}' failed"
    end
    if status.signaled?
      if status.termsig != aborted_sig
        puts output
        log.error "ark aborted with signal #{status.termsig}, but expected #{aborted_sig}"
        raise_error "Test '#{@name}' failed"
      end
    elsif status.exitstatus != expected_result
      puts output
      log.error "ark returns code #{status.exitstatus}, but expected #{expected_result}"
      raise_error "Test '#{@name}' failed"
    end
    log.debug output
    File.open("#{@cwd}/console.out", "w") { |file| file.write(output) }

    @events_scope = SearchScope.from_file("#{@cwd}/events.csv", 'Events')
  end

  def WRITE_FILE(**args)
    inputs = ""
    outputs = ""
    ext = ""
    args.each do |name, value|
      case name
      when :inputs
        inputs << value
      when :outputs
        outputs << value
      when :ext
        ext << value
      end
    end

    File.open("#{@cwd}/#{outputs}.#{ext}", "w") { |file| file.write(inputs) }
  end

  def RUN_PAOC(**args)
    @aot_file = "#{@cwd}/#{File.basename(@options.test_file, File.extname(@options.test_file))}.an"

    inputs = @options.test_file
    aot_output_option = '--paoc-output'
    output = @aot_file
    options = []
    env = ''
    aborted_sig = 0
    result = 0

    args.each do |name, value|
      case name
      when :options
        options << value
      when :boot
        next unless value
        aot_output_option = '--paoc-boot-output'
      when :pgo_use_profdata
        next unless value
        raise "call RUN with `pgo_emit_profdata: true` (or RUN_PGO_PROF) before :pgo_use_profdata" unless @profdata_file
        options << "--paoc-use-profile:path=#{@profdata_file},force"
        options << "--panda-files=#{@options.test_file}"  # NOTE (urandon): this is required for compiler's runtime now
      when :env
        env = value
      when :inputs
        inputs = value
      when :abort
        aborted_sig = value
      when :output
        output = value
      when :result
        result = value
      end
    end
    raise ":abort and :result cannot be set at the same time, :abort = #{aborted_sig}, :result = #{result}" if aborted_sig != 0 && result != 0

    paoc_args = "--paoc-panda-files #{inputs} --events-output=csv --compiler-dump #{options.join(' ')} #{aot_output_option} #{output}"

    clear_data

    cmd = "#{@options.run_prefix} #{@options.paoc} --compiler-ignore-failures=false --compiler-disasm-dump:single-file --compiler-dump #{@options.paoc_options} #{paoc_args}"
    $curr_cmd = "#{env} #{cmd}"
    log.debug "Paoc command: #{$curr_cmd}"

    # Using exec to pass signal info to the parent process.
    # Ruby invokes a process using /bin/sh if the curr_cmd has a metacharacter in it, for example '*', '?', '$'.
    # If an invoked process signals, then the status.signaled? check below returns different values depending on the shell.
    # For bash it is true, for dash it is false, because bash propagates a flag, whether the process has signalled or not.
    # When we use 'exec' we will propagate the signal too
    output, status = Open3.capture2e("#{env} exec #{cmd}", chdir: @cwd.to_s)
    if aborted_sig != 0 && !status.signaled?
      puts output
      log.error "Expected ark_aot to abort with signal #{aborted_sig}, but ark_aot did not signal"
      raise_error "Test '#{@name}' failed"
    end
    if status.signaled?
      if status.termsig != aborted_sig
        puts output
        log.error "ark_aot aborted with signal #{status.termsig}, but expected #{aborted_sig}"
        raise_error "Test '#{@name}' failed"
      end
    elsif status.exitstatus != result
      puts output
      log.error "ark_aot returns code #{status.exitstatus}, but expected #{result}"
      raise_error "Test '#{@name}' failed"
    end
    log.debug output
    File.open("#{@cwd}/console.out", "w") { |file| file.write(output) }

    @events_scope = SearchScope.from_file("#{@cwd}/events.csv", 'Events')
  end

  def RUN_AOT(**args)
    raise 'aot_mode cannot be nil' if @aot_mode.nil?
    case @aot_mode
    when AotMode::PAOC
      RUN_PAOC(**args)
    when AotMode::LLVM
      RUN_LLVM(**args)
    when AotMode::ALL
      raise 'Checker not populated, run populate()'
    end
  end

  def RUN_PGO_PROF(**args)
    RUN(force_profiling: true, pgo_emit_profdata: true, **args)
  end

  def RUN_PGO_PAOC(**args)
    RUN_PAOC(pgo_use_profdata: true, **args)
  end

  def RUN_FRONTEND(**args)
    frontend = @options.frontend
    if frontend.include? "es2panda"
      RUN_FRONTEND_ETS(**args)
    elsif frontend.include? "ark_asm"
      RUN_FRONTEND_PA(**args)
    end
  end

  def RUN_FRONTEND_ETS(**args)
    inputs = @options.source
    output = prepare_frontend_out
    @args = ''
    # handles evaluation of expressions in arguments such as #{@options.test_file}
    # specifically to use them in ets test annotations
    args.each do |name, value|
      case name
      when :options
        @args << eval("\"" + value + "\"")
      when :inputs
        # assume inputs provides a path to dependency files relative to source file
        dir_path = File.dirname(@options.source)
        inputs = dir_path + "/" + eval("\"" + value + "\"")
      when :output
        dir_path = File.dirname(@options.test_file)
        output = dir_path + "/" + eval("\"" + value + "\"")
      end
    end

    if !@args.include? "--opt-level"
      @args << " --opt-level=2"
    end
    if !@args.include? "--ets-strings-concat"
      @args << " --ets-strings-concat=true"
    end

    clear_data
    $curr_cmd = "#{@options.frontend} #{@options.frontend_options} #{@args} --output=#{output} #{inputs}"
    log.debug "Frontend command: #{$curr_cmd}"

    exec_cmd
  end

  def RUN_FRONTEND_PA(**args)
    inputs = @options.source
    output = prepare_frontend_out
    @args = ''

    args.each do |name, value|
      case name
      when :options
        @args << eval("\"" + value + "\"")
      when :inputs
        inputs = eval("\"" + value + "\"")
      when :output
        output = eval("\"" + value + "\"")
      end
    end

    clear_data
    $curr_cmd = "#{@options.run_prefix} #{@options.frontend} #{@options.frontend_options} #{inputs} #{output}"
    log.debug "Frontend command: #{$curr_cmd}"

    exec_cmd
  end

  def RUN_BCO(**args)
    inputs = @options.source
    output = prepare_frontend_out
    @args = ''
    # handles evaluation of expressions in arguments such as #{@options.test_file}
    # specifically to use them in ets test annotations
    args.each do |name, value|
      case name
      when :options
        @args << eval("\"" + value + "\"")
      when :inputs
        inputs = eval("\"" + value + "\"")
      when :output
        output = eval("\"" + value + "\"")
      when :method
        @args << " --bco-optimizer --method-regex=#{value}:.*"
      end
    end

    clear_data
    $curr_cmd = "#{@options.frontend} --opt-level=2 --dump-assembly --bco-compiler --compiler-dump \
            #{@options.frontend_options} #{@args} --output=#{output} #{inputs}"
    log.debug "Frontend command: #{$curr_cmd}"

    exec_cmd
  end

  def RUN_LLVM(**args)
    raise SkipException unless @options.with_llvm

    args[:options] = '' unless args.has_key? :options
    args[:options] << " --paoc-mode=llvm "
    RUN_PAOC(**args)
  end

  def EVENT(match)
    return if @options.release

    @events_scope.find(match)
  end

  def EVENT_NEXT(match)
    return if @options.release

    @events_scope.find_next(match)
  end

  def EVENT_COUNT(match)
    return 0 if @options.release

    @events_scope.lines.count { |event| contains?(event, match) }
  end

  def EVENT_NOT(match)
    return if @options.release

    @events_scope.find_not(match)
  end

  def EVENT_NEXT_NOT(match)
    return if @options.release

    @events_scope.find_next_not(match)
  end

  def EVENTS_COUNT(match, count)
    return if @options.release

    res = @events_scope.lines.count { |event| contains?(event, match) }
    raise_error "Events count missmatch for #{match}, expected: #{count}, real: #{res}" unless res == count
  end

  def TRUE(condition)
    return if @options.release

    raise_error "Not true condition: \"#{condition}\"" unless condition
  end

  class SkipException < StandardError
  end

  def SKIP_IF(condition)
    raise SkipException if condition
  end

  def IR_COUNT(match)
    return 0 if @options.release
    @ir_scope.lines.count { |inst| contains?(inst, match) && !contains?(inst, /^Method:/) }
  end

  def BLOCK_COUNT
    IR_COUNT('BB ')
  end

  def INST(match)
    return if @options.release

    @ir_scope.find(match)
  end

  def INST_NEXT(match)
    return if @options.release

    @ir_scope.find_next(match)
  end

  def INST_NOT(match)
    return if @options.release
    @ir_scope.find_not(match)
  end

  def INST_NEXT_NOT(match)
    return if @options.release

    @ir_scope.find_next_not(match)
  end

  def INST_COUNT(match, count)
    return if @options.release

    real_count = IR_COUNT(match)
    raise_error "IR_COUNT mismatch for #{match}: expected=#{count}, real=#{real_count}" unless real_count == count
  end

  def IN_BLOCK(match, nth = 1)
    return if @options.release

    @ir_scope.find_block(/prop: #{match}/, nth)
  end

  def LLVM_METHOD(match)
    return if @options.release

    @ir_scope.find_method_dump(match)
  end

  def BC_METHOD(match)
    return if @options.release

    READ_FILE "console.out"
    @ir_scope.find_method_dump(/^\.function.*#{match.gsub('.', '-')}/)
  end

  module SearchState
    NONE = 0
    SEARCH_BODY = 1
    SEARCH_END = 2
  end

  def ASM_METHOD(match)
    ensure_disasm
    state = SearchState::NONE
    start_index = nil
    end_index = -1
    @disasm_lines.each_with_index do |line, index|
      case state
      when SearchState::NONE
        if line.start_with?('METHOD_INFO:') && contains?(@disasm_lines[index + 1].split(':', 2)[1].strip, match)
          state = SearchState::SEARCH_BODY
        end
      when SearchState::SEARCH_BODY
        if line.start_with?('DISASSEMBLY')
          start_index = index + 1
          state = SearchState::SEARCH_END
        end
      when SearchState::SEARCH_END
        if line.start_with?('METHOD_INFO:')
          end_index = index - 1
          break
        end
      end
    end
    raise "Method not found: #{match_str(match)}" if start_index.nil?

    @disasm_method_scope = SearchScope.new(@disasm_lines[start_index..end_index], "Method: #{match_str(match)}")
    @disasm_scope = @disasm_method_scope
  end

  def ASM_INST(match)
    ensure_disasm
    state = SearchState::NONE
    start_index = nil
    end_index = -1
    prefix = nil
    @disasm_method_scope.lines.each_with_index do |line, index|
      case state
      when SearchState::NONE
        if contains?(line, match)
          prefix = line.sub(/#.*/, '#').gsub("\n", '')
          start_index = index + 1
          state = SearchState::SEARCH_END
        end
      when SearchState::SEARCH_END
        if line.start_with?(prefix)
          end_index = index - 1
          break
        end
      end
    end
    raise "Can not find asm instruction: #{match}" if start_index.nil?

    @disasm_scope = SearchScope.new(@disasm_method_scope.lines[start_index..end_index], "Inst: #{match_str(match)}")
  end

  def ASM_RESET
    @disasm_scope = @disasm_method_scope
  end

  def ASM(**kwargs)
    ensure_disasm
    @disasm_scope.find(select_asm(kwargs))
  end

  def ASM_NEXT(**kwargs)
    ensure_disasm
    @disasm_scope.find_next(select_asm(kwargs))
  end

  def ASM_NOT(**kwargs)
    ensure_disasm
    @disasm_scope.find_not(select_asm(kwargs))
  end

  def ASM_NEXT_NOT(**kwargs)
    ensure_disasm
    @disasm_scope.find_next_not(select_asm(kwargs))
  end

  def select_asm(kwargs)
    kwargs[@options.arch.to_sym]
  end

  def ensure_disasm
    @disasm_lines ||= File.readlines("#{@cwd}/disasm.txt")
  end

  def METHOD(method)
    return if @options.release
    @ir_files = Dir["#{@cwd}/ir_dump/*#{method.gsub(/::|[<>]|\.|-/, '_')}*.ir"]
    @ir_files.sort!
    raise_error "IR dumps not found for method: #{method.gsub(/::|[<>]|\.|-/, '_')}" if @ir_files.empty?
    $current_method = method
    @current_file_index = 0
  end

  def PASS_AFTER(pass)
    return if @options.release

    $current_pass = "Pass after: #{pass}"
    @current_file_index = @ir_files.index { |x| File.basename(x).include? pass }
    raise_error "IR file not found for pass: #{pass}. Possible cause: you forgot to select METHOD first" unless @current_file_index
    @ir_scope = SearchScope.from_file(@ir_files[@current_file_index], 'IR')
  end

  def PASS_AFTER_NEXT(pass)
    return if @options.release

    $current_pass = "Pass after next: #{pass}"
    index = @ir_files[(@current_file_index + 1)..-1].index { |x| File.basename(x).include? pass }
    raise_error "IR file not found for pass: #{pass}. Possible cause: you forgot to select METHOD first" unless index
    @current_file_index += 1 + index
    @ir_scope = SearchScope.from_file(@ir_files[@current_file_index], 'IR')
  end

  def PASS_AFTER_NTH(pass, nth = 1)
    return if @options.release

    $current_pass = "Pass after #{nth}th occurrence of: #{pass}"
    matches = @ir_files.each_with_index.select { |x, i| File.basename(x).include? pass }
    raise_error "IR file not found for pass: #{pass}. Possible cause: you forgot to select METHOD first" if matches.empty?
    raise_error "Not enough occurrences (#{matches.size}) of pass #{pass} (requested #{nth})" if matches.size < nth

    @current_file_index = matches[nth-1][1]
    @ir_scope = SearchScope.from_file(@ir_files[@current_file_index], 'IR')
  end

  def PASS_BEFORE_NTH(pass, nth = 1)
    return if @options.release

    $current_pass = "Pass before #{nth}th occurrence of: #{pass}"
    matches = @ir_files.each_with_index.select { |x, i| File.basename(x).include? pass }
    raise_error "IR file not found for pass: #{pass}. Possible cause: you forgot to select METHOD first" if matches.empty?
    raise_error "Not enough occurrences (#{matches.size}) of pass #{pass} (requested #{nth})" if matches.size < nth

    @current_file_index = matches[nth-1][1] - 1
    raise_error "No pass before first occurrence of #{pass}" if @current_file_index < 0
    @ir_scope = SearchScope.from_file(@ir_files[@current_file_index], 'IR')
  end

  def PASS_BEFORE(pass)
    return if @options.release

    $current_pass = "Pass before: #{pass}"
    @current_file_index  = @ir_files.index { |x| File.basename(x).include? pass }
    raise_error "IR file not found for pass: #{pass}. Possible cause: you forgot to select METHOD first" unless @current_file_index
    @ir_scope = SearchScope.from_file(@ir_files[@current_file_index - 1], 'IR')
  end

  def READ_FILE(filename)
    path = "#{@cwd}/#{filename}"
    raise_error "File `#{filename}` not found" unless File.file?(path)
    @ir_scope = SearchScope.from_file(path, 'Plain text')
  end

  def run
    unless match_filter?
      log.info "Filtered-out: \"#{@name}\""
      return
    end

    log.info "Running \"#{@name}\""
    init_run
    $checker_name = @name
    begin
      $checker_counter += 1
      # This block processes checkers that have //! DEFINE_FRONTEND_OPTIONS blocks. If used block already compiled .abc we use its path. Otherwise we add compilation steps to checker and afterwards save path to .abc
      if FrontendData.checker_frontend.key?(@name)
        frontend_name = FrontendData.checker_frontend[@name]
        frontendelement = FrontendData.frontendelements[frontend_name]
        if frontendelement["path"]
          @options.test_file = frontendelement["path"]
          @code.source.sub! frontend_name, ""
        else
          @code.source.sub! frontend_name, frontendelement["exec"]
        end
      end
      self.instance_eval(@code.source, @code.filename, @code.line_no)
      frontendelement["path"] = @options.test_file if frontendelement and !frontendelement["path"]
    rescue SkipException
      log.info "Skipped: \"#{@name}\""
    else
      log.info "Success: \"#{@name}\""
    end
    clear_data
  end

  def exec_cmd
    # See note on exec in RUN_PAOC
    output, err_output, status = Open3.capture3("exec #{$curr_cmd}", chdir: @cwd.to_s)
    if status.signaled?
      if status.termsig != 0
        puts output
        log.error "#{@options.frontend} aborted with signal #{status.termsig}, but expected 0"
        raise_error "Test '#{@name}' failed"
      end
    elsif status.exitstatus != 0
      puts output
      log.error "#{@options.frontend} returns code #{status.exitstatus}, but expected 0"
      raise_error "Test '#{@name}' failed"
    elsif !err_output.empty?
      log.error "Bytecode optimizer failed, logs:"
      puts err_output
      raise_error "Test '#{@name}' failed"
    end
    File.open("#{@cwd}/console.out", "w") { |file| file.write(output) }
    Open3.capture2e("cat #{@cwd}/console.out")
    FileUtils.touch("#{@cwd}/events.csv")
  end

  def prepare_frontend_out
   output = File.join(@options.test_dir,"#{File.basename(@options.test_file).split('.')[0]}.#{$checker_counter}.abc")
   @options.test_file = output
   output
  end

  def clear_data
   $current_method = nil
   $current_pass = nil
   if !@options.keep_data
      FileUtils.rm_rf("#{@cwd}/events.csv")
      FileUtils.rm_rf("#{@cwd}/disasm.txt")
      FileUtils.rm_rf("#{@cwd}/console.out")
   else
      @run_idx += 1
      FileUtils.mv "#{@cwd}/events.csv", "#{@cwd}/events-#{@run_idx}.csv", force: true
      FileUtils.mv "#{@cwd}/disasm.txt", "#{@cwd}/disasm-#{@run_idx}.txt", force: true
      FileUtils.mv "#{@cwd}/console.out", "#{@cwd}/console-#{@run_idx}.out", force: true
   end
  end
end

def read_checks(options)
  FrontendData.reset
  checks = []
  checks_recheck = {}
  check = nil
  checkgroups = {}
  checkgroup = nil
  frontendelement_name = nil
  frontendelements = {}
  checker_frontend = {}  # Hash of "name of checker": "name of his frontendelement"
  command_token = /[ ]*#{options.command_token}(.*)/
  checker_start = /[ ]*#{options.command_token} CHECKER[ ]*(.*)/
  disabled_checker_start = /[ ]*#{options.command_token} DISABLED_CHECKER[ ]*(.*)/
  checkgroup_start = /[ ]*#{options.command_token} CHECK_GROUP[ ]*(.*)/
  frontend_start = /[ ]*#{options.command_token} DEFINE_FRONTEND_OPTIONS[ ]*(.*)/
  File.readlines(options.source).each_with_index do |line, line_no|
    if check || checkgroup || frontendelement_name
      unless line.start_with? command_token
        check = nil
        checkgroup = nil
        frontendelement_name = nil
        next
      end
      raise "No space between two checkers: '#{line.strip}'" if line.start_with? checker_start or (checkgroup and line.start_with? checkgroup_start) or (frontendelement_name and line.start_with? frontend_start)
      command_text = command_token.match(line)[1]
      if check
        if line.start_with? checkgroup_start
          if checkgroups.key?(command_text)
            check.append_line(checkgroups[command_text])
            next
          else
            unless checks_recheck.key?(check.name)
              checks_recheck[check.name] = []
            end
            checks_recheck[check.name].append(command_text)
          end
        end
        if line.start_with? frontend_start
            unless checker_frontend.key?(command_text)
              checker_frontend[check.name] = command_text
            end
        end
        check.append_line(command_text) unless check == :disabled_check
      elsif checkgroup
        checkgroups[checkgroup] += command_text
        checkgroups[checkgroup] += "\n"
      elsif frontendelement_name
        frontendelements[frontendelement_name]["exec"] += command_text
        frontendelements[frontendelement_name]["exec"] += "\n"
      end
    else
      next unless line.start_with? command_token
      if line.start_with? checker_start
        name = command_token.match(line)[1]
        raise "Checker with name '#{name}'' already exists" if checks.any? { |x| x.name == name }

        check = Checker.new(options, name, line_no: line_no)
        check.append_line(" SKIP_IF      @architecture == \"arm32\"") if name.include? "AOT"
        checks << check

      elsif line.start_with? checkgroup_start
        checkgroup = command_token.match(line)[1]
        raise "CheckGroup with name '#{checkgroup}'' already exists" if checkgroups.key?(checkgroup)

        checkgroups[checkgroup] = ""
      elsif line.start_with? frontend_start
        frontendelement_name = command_token.match(line)[1]
        raise "FRONTEND  with name '#{frontendelement_name}' already exists" if frontendelements.key?(frontendelement_name)
        frontendelements[frontendelement_name] = Hash.new()
        frontendelements[frontendelement_name]["exec"] = ""
        frontendelements[frontendelement_name]["path"] = nil
      else
        raise "Line '#{line.strip}' does not belong to any checker" unless line.start_with? disabled_checker_start
        check = :disabled_check
        next
      end
    end
  end
  checks_recheck.each do |ch_name, d_name_lst|
    checks.each do |check|
      if check.name == ch_name
        d_name_lst.each do |d_name|
          raise "CheckGroup with name '#{d_name}' not specified" unless checkgroups.key?(d_name)

          check.code.source.sub! d_name, checkgroups[d_name]
        end
      end
    end
  end

  if options.test_dir  # meaning it is a urunner test that needs to be compiled by this script
    run_frontend_str = "RUN_FRONTEND"
    def_frontend_str = "DEFINE_FRONTEND_OPTIONS"
    default_args_str = "DEFAULT ARGS"
    checks.each do |check|
      if ![run_frontend_str, def_frontend_str, "RUN_BCO"].any? { |frontend_mark| check.code.source.include?(frontend_mark) }
        frontend_default_names = frontendelements.keys.select do |frontend_name|
          tmp = frontend_name.dup
          tmp.sub! def_frontend_str, ""
          tmp = tmp.strip
          tmp == default_args_str
        end
        raise "'#{check.name}' has more than 1 '#{def_frontend_str} #{default_args_str}' blocks" if frontend_default_names.length > 1

        if frontend_default_names.length == 1
          frontend_default_name = frontend_default_names[0]
        else
          frontend_default_name = " #{def_frontend_str}            #{default_args_str}"
          frontendelements[frontend_default_name] = Hash.new()
          frontendelements[frontend_default_name]["exec"] = " #{run_frontend_str}            options: \"--opt-level=2 --ets-strings-concat=true\"\n"
          frontendelements[frontend_default_name]["path"] = nil
        end
        check.code.source.prepend("#{frontend_default_name}\n")
        checker_frontend[check.name] = frontend_default_name
      end
    end

    checker_frontend.each do |ch_name, frontend_name|
      raise "'#{frontend_name}' is not defined for checker '#{ch_name}'" if !frontendelements.key?(frontend_name)
    end
  end

  FrontendData.frontendelements = frontendelements
  FrontendData.checker_frontend = checker_frontend
  checks
end

def main(options)
  if options.test_dir
    FileUtils.rm_rf(options.test_dir)
    FileUtils.mkdir_p options.test_dir
    Dir.chdir options.test_dir
  end
  read_checks(options).flat_map(&:populate).each(&:run)
  0
end

if __FILE__ == $PROGRAM_NAME
  main(options)
end

# Somehow ruby resolves `Checker` name to another class in a Testing scope, so make this global
# variable to refer to it from unit tests. I believe there is more proper way to do it, but I
# didn't find it at first glance.
$CheckerForTest = Checker
