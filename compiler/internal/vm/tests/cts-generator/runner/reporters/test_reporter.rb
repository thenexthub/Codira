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

require_relative 'string_logger'
require_relative 'base_test_reporter'

module TestRunner
  # LogTestReporter is reporter to console, default reporter
  class LogTestReporter < BaseTestReporter
    @@mutex = Mutex.new

    def initialize(_root_dir, pa_file, _report_dir)
      super()
      @pa_file = pa_file
      @logger = Reporters::StringLogger.new
    end

    def prologue
      @logger.log 2
      @logger.log 2, '----------------------------------------'
      @logger.log 2
      @logger.log 2, "Test file: #{@pa_file}"
    end

    def epilogue
      output = @logger.string
      unless output.empty?
        @@mutex.synchronize do
          puts output
        end
      end
      @logger.close
    end

    def log_exclusion
      @logger.log 2, "Skip excluded test #{@pa_file} by tag"
    end

    def log_skip_include
      @logger.log 2, "Skip not included test #{@pa_file} by tag"
    end

    def log_skip_bugid
      @logger.log 2, "Skip test #{@pa_file} since bug id do not match"
    end

    def log_skip_ignore
      @logger.log 2, "Skip test #{@pa_file}, because 'runner_option: ignore' tag is defined"
    end

    def log_skip_only_ignore
      @logger.log 2, "Skip test #{@pa_file}, because run only test with 'runner_option: ignore' tag"
    end

    def log_ignore_ignored
      @logger.log 2, "Execute test #{@pa_file}, since 'runner_option: ignore' tag is ignored"
    end

    def log_start_command(cmd)
      @logger.log 3, "Start: #{cmd}"
    end

    def log_failed_compilation(output)
      @logger.log 1, "TEST FAILED: #{@pa_file}"
      @logger.log 1, 'Compilation failed.'
      @logger.log 1, output unless output.empty?
    end

    def log_failed_quickening(output)
      @logger.log 1, "TEST FAILED: #{@pa_file}"
      @logger.log 1, 'Quickening failed.'
      @logger.log 1, output unless output.empty?
    end

    def log_negative_passed_compilation(output)
      @logger.log 1, "TEST FAILED: #{@pa_file}"
      @logger.log 1, 'Compilation succeeded, but compilation failure expected.'
      @logger.log 1, output unless output.empty?
    end

    def log_failed_negative_compilation(output)
      @logger.log 2, "TEST PASSED: #{@pa_file}"
      @logger.log 2, 'Compilation failed, as expected.'
      @logger.log 3, output unless output.empty?
    end

    def log_compilation_passed(output)
      @logger.log 2, "TEST PASSED: #{@pa_file}"
      @logger.log 2, 'Compilation-only test.'
      @logger.log 3, output unless output.empty?
    end

    def log_run_negative_failure(output, status)
      @logger.log 1, "TEST FAILED: #{@pa_file}"
      @logger.log 1, "Ark exit code: #{status}, but expected ark failure."
      @logger.log 1, output unless output.empty?
    end

    def log_verifier_negative_failure(output, status)
      @logger.log 1, "TEST FAILED: #{@pa_file}"
      @logger.log 1, "Verifier exit code: #{status}, but expected verifier failure."
      @logger.log 1, output unless output.empty?
    end

    def log_run_failure(output, status, core)
      @logger.log 1, "TEST FAILED: #{@pa_file}"
      @logger.log 1, "Ark exit code: #{status}"
      @logger.log 1, 'Core dump was created' if core
      @logger.log 1, output unless output.empty?
    end

    def log_verifier_failure(output, status, core)
      @logger.log 1, "TEST FAILED: #{@pa_file}"
      @logger.log 1, "Verifier exit code: #{status}"
      @logger.log 1, 'Core dump was created' if core
      @logger.log 1, output unless output.empty?
    end

    def log_passed(output, status)
      @logger.log 2, "TEST PASSED: #{@pa_file}"
      @logger.log 2, "Ark exit code: #{status}"
      @logger.log 3, output unless output.empty?
    end

    def log_excluded
      @logger.log 3, "TEST EXCLUDED: #{@pa_file}"
    end

    def verbose_log(status)
      @logger.log 3, status
    end

    def log_bugids(bugids) end

    def log_repro_commands(cmds)
      return unless cmds.length.positive?

      @logger.log 1, 'Commands to reproduce:'
      @logger.log 1, cmds
      @logger.log 1, ''
    end
  end
end
