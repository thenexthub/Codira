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

require_relative 'runner'
require_relative 'result'
require_relative 'reporters/test_reporter'

module TestRunner
  class SingleTestRunner
    def initialize(file, id, reporter_class, root_dir, report_dir)
      @pa_file = file
      @bin_file = "#{$tmp_dir}#{File.basename(file)}.bin"
      @id = id
      @repro_cmds = []
      @test_failed = false
      @reporter = reporter_class.new root_dir, @pa_file, report_dir
      @result = TestRunner::Result.new @reporter
      lines = File.foreach(@pa_file)
                  .grep(/^\s*##\s*runner-option\s*:\s*[\S*\s*]*\s*$/)
                  .flat_map { |s| s.split(':', 2).map(&:strip) }
                  .reject { |s| s.match(/^#/) }
                  .uniq
      @runner_options = lines.each_with_object({}) do |k, h|
        if k.include? 'tags:'
          tags = TestRunner.split_separated_by_colon k
          h['tags'] = tags
        elsif k.include? 'bugid:'
          bugs = TestRunner.split_separated_by_colon k
          h['bug_ids'] = bugs
          @reporter.log_bugids(bugs)
        else
          h[k] = true
        end
      end
      @pa_options = if @runner_options.include? 'use-pa'
                        '--load-runtimes="core" ' \
                        '--boot-panda-files=' \
                        "#{$path_to_panda}/pandastdlib/arkstdlib.abc"
                      else
                        ''
                      end

      @verifier_options = $verbose_verifier ? '--log-components=verifier --log-level=debug ' : ''

      @verifier_config_args = if (@runner_options.key?('verifier-config') || verifier_forced?) && !$verifier_config.empty?
                                "--config-file=#{$verifier_config} "
                              else
                                ''
                              end

      if @runner_options.include? 'main-exitcode-wrapper'
        @main_function = '_GLOBAL::main_exitcode_wrapper'
        # Exit code value for wrapped main
        # this value is used to determine false-positive cases
        @expected_passed_exit_code = 80
        @expected_failed_exit_code = 81
      else
        @main_function = '_GLOBAL::main'
        # Default exit code value for main function
        @expected_passed_exit_code = 0
        @expected_failed_exit_code = 1
      end

      @test_panda_options = ''
      previous_line_empty = false # exit loop on two consecutive empty lines
      File.foreach(@pa_file) do |line|
        if match = line.match(/^## panda-options: (.+)\s*$/)
          @test_panda_options = match.captures[0]
          break # the line we are looking for is found
        elsif line.strip.empty?
          break if previous_line_empty

          previous_line_empty = true
        else
          previous_line_empty = false
        end
      end
    end

    def error?(status)
      [
        ERROR_NODATA,
        ERROR_CANNOT_CREATE_PROCESS,
        ERROR_TIMEOUT
      ].include? status
    end

    def in_exclude_list?
      !(@runner_options['tags'] & $exclude_list || []).empty?
    end

    def in_include_list?
      !(@runner_options['tags'] & $include_list || []).empty?
    end

    def in_bugids_list?
      !(@runner_options['bug_ids'] & $bug_ids || []).empty?
    end

    def compile_only?
      (@runner_options.include? 'compile-failure' or
        @runner_options.include? 'compile-only')
    end

    def verifier_only?
      (@runner_options.include? 'verifier-failure' or
        @runner_options.include? 'verifier-only')
    end

    def verifier_forced?
      ($force_verifier and
        !(@runner_options.include? 'verifier-failure' or
          @runner_options.include? 'verifier-only' or
          @runner_options.include? 'compile-failure'))
    end

    def test_failed?
      @test_failed
    end

    def test_failed
      @test_failed = true
    end

    def cleanup
      @reporter.verbose_log "# Cleanup - remove #{@bin_file}, if exists"
      FileUtils.rm(@bin_file) if File.exist? @bin_file
      FileUtils.rm("#{@bin_file}.aot") if $paoc && File.exist?("#{@bin_file}.aot")
    end

    def compilation
      output, status, _core = run_pandasm
      case status
      when 0
        if @runner_options['compile-failure']
          @result.update_negative_passed_compilation output, @pa_file
          test_failed
        elsif @runner_options['compile-only']
          @result.update_compilation_passed output, @pa_file
        end
      when 1
        if @runner_options['compile-failure']
          @result.update_failed_negative_compilation output, @pa_file
        else
          @result.update_failed_compilation output, @pa_file
          test_failed
        end
      else
        @result.update_failed_compilation output, @pa_file
        test_failed
      end
    end

    def run_pandasm
      run_command "#{$pandasm} #{@pa_file} #{@bin_file}"
    end

    def verification
      output, status, core = run_verifier
      case status
      when 0
        if @runner_options['verifier-failure']
          @result.update_verifier_negative_failure output, status, @pa_file
          test_failed
        else
          @result.update_passed output, status, @pa_file
        end
      when 255
        if @runner_options['verifier-failure']
          @result.update_passed output, status, @pa_file
        else
          @result.update_verifier_failure output, status, @pa_file, core
          test_failed
        end
      else
        @result.update_verifier_failure output, status, @pa_file, core
        test_failed
      end
    end

    def run_verifier
      run_command "#{$verifier} #{@verifier_options} #{@verifier_config_args} #{@pa_options} #{@bin_file}"
    end

    def execution
      output, status, core = run_panda
      case status
      when @expected_passed_exit_code
        if @runner_options['run-failure']
          @result.update_run_negative_failure output, status, @pa_file
          test_failed
        else
          @result.update_passed output, status, @pa_file
        end
      when @expected_failed_exit_code, 1 # '1' for the case when Exception is not caught in test
        if @runner_options['run-failure']
          @result.update_passed output, status, @pa_file
        else
          @result.update_run_failure output, status, @pa_file, core
          test_failed
        end
      else
        # Test ended abnormally
        @result.update_run_failure output, status, @pa_file, core
        test_failed
      end
    end

    def run_panda
      aot = if $paoc
              "--aot-file=#{@bin_file}.aot"
            else
              ''
            end
      run_command "#{$panda} #{@pa_options} #{$panda_options.join ' '} #{@test_panda_options} #{aot} " \
            "#{@bin_file} #{@main_function}"
    end

    def aot_compilation
      output, status, _core = run_paoc
      return if status.zero?

      @result.update_failed_compilation output, @pa_file
      test_failed
    end

    def run_paoc
      run_command "#{$paoc} #{@pa_options} --paoc-panda-files #{@bin_file} --paoc-output #{@bin_file}.aot"
    end

    def quickening
      output, status, _core = run_quickener
      return if status.zero?

      @result.update_failed_quickening output, @pa_file
      test_failed
    end

    def run_quickener
      run_command "#{$quickener} #{@bin_file} #{@bin_file}"
    end

    def run_command(cmd)
      @repro_cmds << "#{cmd} ; echo __$?__"
      TestRunner::CommandRunner.new(cmd, @reporter).run_cmd
    end

    def process_single
      @reporter.prologue
      process_single_inner
      @reporter.epilogue
    end

    def process_single_inner
      @reporter.verbose_log '# List of runner options'
      @reporter.verbose_log "verifier-failure      = #{@runner_options['verifier-failure']}"
      @reporter.verbose_log "verifier-only         = #{@runner_options['verifier-only']}"
      @reporter.verbose_log "compiler-failure      = #{@runner_options['compile-failure']}"
      @reporter.verbose_log "compiler-only         = #{@runner_options['compile-only']}"
      @reporter.verbose_log "failure               = #{@runner_options['run-failure']}"
      @reporter.verbose_log "use-pa     = #{@runner_options['use-pa']}"
      @reporter.verbose_log "bugid                 = #{@runner_options['bug_ids']}"
      @reporter.verbose_log "tags                  = #{@runner_options['tags']}"
      @reporter.verbose_log "ignore                = #{@runner_options['ignore']}"
      @reporter.verbose_log "verifier-config       = #{@runner_options['verifier-config']}"
      @reporter.verbose_log "main-exitcode-wrapper = #{@runner_options['main-exitcode-wrapper']}"

      # 1) Check step
      if in_exclude_list?
        @reporter.log_exclusion
        @result.update_excluded @pa_file
        return
      end

      if $include_list != [] && !in_include_list?
        @reporter.log_skip_include
        return
      end

      # Check for bugid
      if $bug_ids != [] && !in_bugids_list?
        @reporter.log_skip_bugid
        return
      end

      run_all_and_ignored = $run_all | $run_ignore

      if @runner_options['ignore'] & !run_all_and_ignored
        @reporter.log_skip_ignore
        return
      end

      if @runner_options['use-pa'] & !run_all_and_ignored
        @reporter.log_skip_ignore
        return
      end

      if !@runner_options['ignore'] & $run_ignore & !$run_all
        @reporter.log_skip_only_ignore
        return
      end

      @reporter.log_ignore_ignored if @runner_options['ignore'] & ($run_all | $run_ignore)
      @reporter.verbose_log ''
      cleanup

      # 2) Compilation
      compilation
      return if test_failed? || compile_only?

      # 3) Quickening
      if $quckener
        quickening
        return if test_failed?
      end

      # 4) Verification
      if verifier_only? || verifier_forced?
        verification
        return
      end

      # 5) AOT Compilation
      if $paoc
        if @runner_options['run-failure']
          @reporter.log_exclusion
          @result.update_excluded @pa_file
          return
        else
          aot_compilation
          return if test_failed?
        end
      end

      # 6) Execution
      execution
    ensure
      if test_failed?
        @reporter.log_repro_commands @repro_cmds
      else
        cleanup
      end
    end
  end
end
