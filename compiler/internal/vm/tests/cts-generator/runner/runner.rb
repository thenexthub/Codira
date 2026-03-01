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

module TestRunner
  ERROR_NODATA = 86
  ERROR_CANNOT_CREATE_PROCESS = 87
  ERROR_TIMEOUT = 88

  @@plugins = []
  @@runner_class = nil
  @@target = 'Host'

  def self.plugins
    @@plugins
  end

  def self.runner_class=(value)
    @@runner_class = value
  end

  def self.target
    @@target
  end

  def self.target=(value)
    @@target = value
  end

  class Plugin
    def name
      raise NotImplementedError, "#{self.class} does not implement name()."
    end

    def add_options(opts, options)
      false
    end

    def process(options)
      false
    end
  end

  def self.log(level, *args)
    puts args if level <= $VERBOSITY
  end

  def self.print_exception(exception)
    puts "Exception: exception class   : #{exception.class}"
    puts "           exception message : #{exception.message}"
    exception.backtrace.each do |t|
      puts "                       trace : #{t}"
    end
  end

  def self.split_separated_by_colon(string)
    string.split(':')
          .drop(1)
          .flat_map { |s| s.split(',').map(&:strip) }
  end

  def self.create_runner(file, id, reporter_factory, root_dir, report_dir)
    @@runner_class.new(
          file, id, reporter_factory, root_dir, report_dir)
  end

  class CommandRunner
    def initialize(command, reporter)
      @command = command
      @reporter = reporter
    end

    def dump_output(t, output_err, output)
      start = Time.now

      while (Time.now - start) <= $TIMEOUT
        Kernel.select([output_err], nil, nil, 1)
        begin
          output << output_err.read_nonblock(2048)
        rescue IO::WaitReadable
        rescue EOFError
          return true # finished normally
        rescue StandardError => e
          output << "\nUnexpected exception when reading from pipe: #{e.class.name}, #{e.message}"
          break
        end
      end
      !t.alive? # finished on timeout
    end

    def start_process_timeout
      input, output_err, t = if $enable_core
                               Open3.popen2e(@command, pgroup: true)
                             else
                               Open3.popen2e(@command, pgroup: true, rlimit_core: 0)
                             end
      pid = t[:pid]
      output = ''
      finished = dump_output t, output_err, output

      input.close
      output_err.close

      unless finished
        output << "\nProcess hangs for #{$TIMEOUT}s '#{@command}'" \
                  "\nKilling pid:#{pid}"
        begin
          Process.kill('-TERM', Process.getpgid(pid))
        rescue Errno::ESRCH
        rescue Exception => e
          TestRunner.print_exception e
        end
        return output.strip, ERROR_TIMEOUT, false
      end

      if t.value.exited?
        exitstatus = t.value.exitstatus
      elsif t.value.signaled?
        output << t.value.inspect
        exitstatus = 128 + t.value.termsig
      else
        output << t.value.inspect
        exitstatus = 254 # fallback exit code for an unexpected abnormal exit
      end

      [output.strip, exitstatus, t.value.coredump?]
    rescue Errno::ENOENT => e
      ["Failed to start #{@command} - no executable", ERROR_CANNOT_CREATE_PROCESS, false]
    rescue StandardError
      ["Failed to start #{@command}", ERROR_CANNOT_CREATE_PROCESS, false]
    end

    def run_cmd
      @reporter.log_start_command @command
      start_process_timeout
    end
  end # Runner
end # module
