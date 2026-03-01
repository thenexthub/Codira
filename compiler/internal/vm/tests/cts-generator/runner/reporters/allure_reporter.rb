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

require 'pathname'
require 'json'
require 'securerandom'
require 'digest/md5'
require_relative 'string_logger'
require_relative 'base_test_reporter'

module TestRunner
  class AllureTestReporter < BaseTestReporter
    def time_now_millisec
      (Time.now.to_f * 1000).to_i
    end

    def set_failure(message)
      @status = 'failed'
      @report['statusDetails'] = {
        'known' => false, 'muted' => false, 'flaky' => false,
        'message' => message, 'trace' => @output
      }
    end

    def initialize(root_dir, pa_file, report_dir)
      root = Pathname.new(root_dir)
      file = Pathname.new(pa_file) # path to file
      actual_file = file.relative_path_from(root)
      report_dir = Pathname.new(report_dir)
      actual_file = root.basename if root.file?
      pa_file = actual_file.to_s
      # Making it 'flat': all json files in same dir
      report_file = report_dir + Pathname.new(
        pa_file.gsub('/', '_').gsub(/.pa$/, '-result.json')
      )
      report_file.dirname.mkpath
      FileUtils.rm report_file if report_file.exist?
      @logger = Reporters::SeparateFileLogger.new(report_file)
      @status = 'failed'
      @output = ''
      feature = TestRunner.target
      @report = {
        'name' => pa_file,
        'status' => 'failed',
        'stage' => 'finished',
        'description' => pa_file,
        'start' => time_now_millisec,
        'uuid' => SecureRandom.uuid,
        'historyId' => Digest::MD5.hexdigest(actual_file.to_s),
        'fullName' => actual_file,
        'labels' => [
          { 'name' => 'suite', 'value' => 'CTS' },
          { 'name' => 'feature', 'value' => "CTS #{feature}" }
        ],
        'links' => []
      }
    end

    def prologue; end

    def epilogue
      @report['stop'] = time_now_millisec
      @report['status'] = @status
      @logger.log 1, JSON[@report]
      @logger.close
    end

    def log_exclusion
      @status = 'skipped'
    end

    def log_skip_include
      @status = 'skipped'
    end

    def log_skip_bugid
      @status = 'skipped'
    end

    def log_skip_ignore
      @status = 'skipped'
    end

    def log_skip_only_ignore
      @status = 'skipped'
    end

    def log_ignore_ignored; end

    def log_start_command(cmd)
      @output << "\ncommand = #{cmd}"
    end

    def log_failed_compilation(output)
      @output << "\n" << output
      set_failure('Failed to compile.')
    end

    def log_negative_passed_compilation(output)
      @output << "\n" << output
      set_failure('Test is compiled, but should be rejected.')
    end

    def log_failed_negative_compilation(output)
      @status = 'passed'
      @output << "\n" << output
    end

    def log_compilation_passed(output)
      @status = 'passed'
      @output << "\n" << output
    end

    def log_run_negative_failure(output, status)
      @output << "\n" << output
      set_failure("Exit code: #{status}, but expected failure.")
    end

    def log_verifier_negative_failure(output, status)
      @output << "\n" << output
      set_failure("Verifier exit code: #{status}, but expected failure.")
    end

    def log_run_failure(output, status, core)
      @output << "\nCore dump was created." if core
      @output << "\n" << output
      set_failure("Exit code: #{status}.")
    end

    def log_verifier_failure(output, status, core)
      @output << "\nCore dump was created." if core
      @output << "\n" << output
      set_failure("Verifier exit code: #{status}.")
    end

    def log_passed(output, _status)
      @status = 'passed'
      @output << "\n" << output
    end

    def log_excluded
      @status = 'skipped'
    end

    def verbose_log(status)
      @output << "\n" << status
    end

    def log_bugids(bugids)
      @report['links'] = bugids.map do |x|
        {
          'name' => x, 'type' => 'issue',
          'url': 'https://rnd-gitlab-msc.huawei.com/rus-os-team/virtual-machines-and-tools/panda/-/issues'
        }
      end
    end

    def log_repro_commands(_cmds); end
  end
end
