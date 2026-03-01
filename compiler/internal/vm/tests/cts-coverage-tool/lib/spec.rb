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

# Class represents full specification with additional information needed for
# coverage computation
class Spec
  attr_reader :data, :orphaned

  def initialize(arr)
    # Full spec object
    @data = merge_specs(arr)

    # Array of tests not matching the spec
    @orphaned = []

    prepare_spec
  end

  def load_non_testable(nt_data)
    nt_data['groups']&.each do |ntg|
      spec_group = @data['groups'].find { |sg| sg['title'] == ntg['title'] }
      if spec_group.nil?
        warn "Non testable group \"#{ntg['title']}\" not found in ISA."
      else
        process_non_testable_group(spec_group, ntg)
      end
    end
  end

  def load_tests(testdir, testglob)
    Dir.glob(testglob, base: testdir) do |f|
      process_test_file(testdir, f)
    rescue StandardError => e
      @orphaned << { 'file' => f, 'error' => e, 'comment' => e.backtrace }
    end
  end

  private

  def merge_specs(sources)
    target = sources[0]
    sources.drop(1).each { |f| merge_spec(target, f) }
    target
  end

  def merge_spec(target, src)
    %w[prefixes groups properties exceptions verification version min_version chapters].each do |attr|
      if target[attr]
        target[attr] += src[attr] if src[attr]
      else
        target[attr] = src[attr]
      end
    end
  end

  def prepare_spec
    @data['groups'].each do |g|
      prepare_spec_group(g)
    end
  end

  def prepare_spec_group(grp)
    prepare_description(grp)
    prepare_instructions(grp)
    prepare_exceptions(grp)
    prepare_verifications(grp)
  end

  def prepare_instructions(grp)
    grp['instructions'].each do |i|
      i['tests'] = []
      i['non_testable'] = false
    end
  end

  def prepare_description(grp)
    grp['description_tests'] = split(grp['description']).map do |da|
      { 'assertion' => da, 'tests' => [], 'non_testable' => false }
    end
  end

  def prepare_exceptions(grp)
    grp['exceptions_tests'] = grp['exceptions'].map do |e|
      { 'exception' => e, 'tests' => [], 'non_testable' => false }
    end
  end

  def prepare_verifications(grp)
    grp['verification_tests'] = grp['verification'].map do |v|
      { 'verification' => v, 'tests' => [], 'non_testable' => false }
    end
  end

  def process_non_testable_group(spec_group, ntg)
    process_non_testable_description(spec_group, ntg)
    process_non_testable_instructions(spec_group, ntg)
    process_non_testable_exceptions(spec_group, ntg)
    process_non_testable_verifications(spec_group, ntg)
  end

  def process_non_testable_description(spec_group, ntg)
    ntg['description'] && split(ntg['description']).each do |ntda|
      spec_description = spec_group['description_tests'].find { |sd| same?(sd['assertion'], ntda) }
      if spec_description.nil?
        warn "Non testable description \"#{ntda}\" in group \"#{ntg['title']}\" not found in iSA."
      else
        spec_description['non_testable'] = true
      end
    end
  end

  def process_non_testable_instructions(spec_group, ntg)
    ntg['instructions']&.each do |nti|
      spec_instruction = spec_group['instructions'].find { |si| si['sig'] == nti['sig'] }
      if spec_instruction.nil?
        warn "Non testable instruction \"#{nti['sig']}\" in group \"#{ntg['title']}\" not found in ISA."
      else
        spec_instruction['non_testable'] = true
      end
    end
  end

  def process_non_testable_exceptions(spec_group, ntg)
    ntg['exceptions']&.each do |nte|
      spec_exception = spec_group['exceptions_tests'].find { |se| se['exception'] == nte }
      if spec_exception.nil?
        warn "Non testable exception \"#{nte}\" in group \"#{ntg['title']}\" not found in ISA."
      else
        spec_exception['non_testable'] = true
      end
    end
  end

  def process_non_testable_verifications(spec_group, ntg)
    ntg['verification']&.each do |ntv|
      spec_verification = spec_group['verification_tests'].find { |sv| sv['verification'] == ntv }
      if spec_verification.nil?
        warn "Non testable verification \"#{ntv}\" in group \"#{ntg['title']}\" not found in ISA."
      else
        spec_verification['non_testable'] = true
      end
    end
  end

  # split long-text description into array of assertions
  def split(description) # rubocop:disable Metrics
    result = []
    small = false
    description.split(/\./).each do |p|
      if small
        result[-1] += ".#{p}"
        small = false if p.length > 5
      elsif p.length > 5
        result << p.lstrip
      else
        if result.length.zero?
          result << p.lstrip
        else
          result[-1] += ".#{p}"
        end
        small = true
      end
    end
    result
  end

  def same?(str1, str2)
    str1.tr('^A-Za-z0-9', '').downcase == str2.tr('^A-Za-z0-9', '').downcase
  end

  def process_test_file(testdir, file)
    raw = read_test_data(File.join(testdir, file))
    tdata = YAML.safe_load(raw)

    if tdata.class != Array || tdata.length.zero?
      @orphaned << { 'file' => file, 'error' => 'Bad test format, expected array of titles', 'comment' => raw }
      return
    end

    tdata.each do |tg|
      process_test_data(tg, file)
    end
  end

  def read_test_data(filename)
    lines_array = []
    started = false
    File.readlines(filename).each do |line|
      started = true if line[0..3] == '#---'
      lines_array << line[1..-1] if started
      break if line[0] != '#'
    end
    lines_array.join("\n")
  end

  def process_test_data(test_group, file)
    spec_group = @data['groups'].find { |g| g['title'] == test_group['title'] }
    if spec_group.nil?
      @orphaned << { 'file' => file, 'error' => 'Group with given title not found in the ISA', 'comment' => test_group }
      return
    end

    assertions = proc_test_instructions(test_group, spec_group, file) +
                 proc_test_descriptions(test_group, spec_group, file) +
                 proc_test_exceptions(test_group, spec_group, file) +
                 proc_test_verifications(test_group, spec_group, file)
    if assertions.zero?
      @orphaned << { 'file' => file, 'error' => 'Test header doesn\'t match any assertions in ISA',
                     'comment' => test_group }
    end
  end

  def proc_test_instructions(test_group, spec_group, file)
    cnt = 0
    test_group['instructions']&.each do |ti|
      cnt += proc_test_instruction(ti, spec_group, file)
    end
    cnt
  end

  def proc_test_instruction(test_instr, spec_group, file)
    gi = spec_group['instructions'].find { |x| x['sig'] == test_instr['sig'] }
    if gi.nil?
      @orphaned << { 'file' => file, 'error' => 'Given instruction not found in the ISA', 'comment' => test_instr }
      return 0
    end
    if gi['non_testable']
      @orphaned << { 'file' => file, 'error' => 'Given instruction is non-testable', 'comment' => test_instr }
      return 0
    end
    gi['tests'] << file
    1
  end

  def proc_test_descriptions(test_group, spec_group, file)
    cnt = 0
    test_group['description'] && split(test_group['description']).each do |tda|
      cnt += proc_test_description(tda, spec_group, file)
    end
    cnt
  end

  def proc_test_description(test_descr, spec_group, file)
    sd = spec_group['description_tests']&.find { |sda| same?(sda['assertion'], test_descr) }
    if sd.nil?
      @orphaned << { 'file' => file, 'error' => 'Given description assertion not found in the spec',
                     'comment' => test_descr }
      return 0
    end
    if sd['non_testable']
      @orphaned << { 'file' => file, 'error' => 'Given description is non-testable', 'comment' => test_descr }
      return 0
    end
    sd['tests'] << file
    1
  end

  def proc_test_exceptions(test_group, spec_group, file)
    cnt = 0
    test_group['exceptions']&.each do |te|
      cnt += proc_test_exception(te, spec_group, file)
    end
    cnt
  end

  def proc_test_exception(test_exc, spec_group, file)
    se = spec_group['exceptions_tests'].find { |x| x['exception'] == test_exc }
    if se.nil?
      @orphaned << { 'file' => file, 'error' => 'Given exception assertion not found in the spec',
                     'comment' => test_exc }
      return 0
    end
    if se['non_testable']
      @orphaned << { 'file' => file, 'error' => 'Given exception assertion is non-testable', 'comment' => test_exc }
      return 0
    end
    se['tests'] << file
    1
  end

  def proc_test_verifications(test_group, spec_group, file)
    cnt = 0
    test_group['verification']&.each do |tv|
      cnt += proc_test_verification(tv, spec_group, file)
    end
    cnt
  end

  def proc_test_verification(test_ver, spec_group, file)
    sv = spec_group['verification_tests'].find { |x| x['verification'] == test_ver }
    if sv.nil?
      @orphaned << { 'file' => file, 'error' => 'Given verification assertion not found in the spec',
                     'comment' => test_ver }
      return 0
    end
    if sv['non_testable']
      @orphaned << { 'file' => file, 'error' => 'Given verification assertion is non-testable', 'comment' => test_ver }
      return 0
    end
    sv['tests'] << file
    1
  end
end
