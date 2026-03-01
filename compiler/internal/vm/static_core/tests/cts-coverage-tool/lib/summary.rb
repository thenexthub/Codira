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

#
# Huawei Technologies Co.,Ltd.
class Summary
  def initialize(spec)
    @spec = spec

    # Tests
    @total = 0
    @accepted = []
    @orphaned = []

    # Assertions
    @testable = 0
    @non_testable = 0
    @covered = 0
    @not_covered = 0

    # Coverage
    @coverage_by_groups = []
    @coverage_metric = 0

    @uncovered_assertions = []
  end

  def report
    {
      'tests' => {
        'total' => @total,
        'counted_for_coverage' => @accepted.length,
        'orphaned' => @orphaned.length
      },
      'assertions' => {
        'testable' => @testable,
        'non_testable' => @non_testable,
        'covered' => @covered,
        'not_covered' => @not_covered
      },
      'coverage_by_groups' => @coverage_by_groups,
      'coverage_metric' => @testable.zero? ? 0 : (@covered.to_f / @testable).round(2)
    }
  end

  def uncovered
    { 'groups' => @uncovered_assertions }
  end

  def compute
    @spec.data['groups'].each do |g|
      compute_for_group(g)
    end

    @accepted = @accepted.uniq
    @orphaned = @spec.orphaned.map { |f| f['file'] }.uniq
    @total = (@accepted + @orphaned).uniq.length
  end

  private

  def compute_for_group(group)
    testable_count = 0
    non_testable_count = 0
    covered_count = 0
    not_covered_count = 0
    uncovered_for_group = { 'title' => group['title'] } # object stores uncovered assertions for the group

    %w[instructions description_tests exceptions_tests verification_tests].each do |k|
      not_covered_assertions = []

      group[k]&.each do |assertion|
        if assertion['non_testable']
          non_testable_count += 1
        else
          testable_count += 1
          if assertion['tests'].length.positive?
            covered_count += 1
            @accepted += assertion['tests']
          else
            not_covered_count += 1
            not_covered_assertions << assertion
          end
        end
      end

      send(k, not_covered_assertions, uncovered_for_group) if not_covered_assertions.length.positive?
    end

    # collect uncovered assertions
    @uncovered_assertions << uncovered_for_group if not_covered_count.positive?

    # update coverage metric for the group
    group_metric = testable_count.positive? ? (covered_count.to_f / testable_count).round(2) : 'Not testable'
    group['coverage_metric'] = group_metric
    @coverage_by_groups << { 'title' => group['title'], 'coverage_metric' => group_metric }

    # update counters
    @testable += testable_count
    @non_testable += non_testable_count
    @covered += covered_count
    @not_covered += not_covered_count
  end

  # The following methods make not covered assertions look like in ISA
  # by formatting and removing extra fields added by us.
  def instructions(not_covered_assertions, uncovered_for_group)
    uncovered_for_group['instructions'] = not_covered_assertions.map { |a| except(a, %w[tests non_testable]) }
  end

  def description_tests(not_covered_assertions, uncovered_for_group)
    uncovered_for_group['description'] = not_covered_assertions.map do |a|
      a['assertion']
    end.join('. ').gsub(/\n/, ' ').rstrip
  end

  def exceptions_tests(not_covered_assertions, uncovered_for_group)
    uncovered_for_group['exceptions'] = not_covered_assertions.map { |a| a['exception'] }
  end

  def verification_tests(not_covered_assertions, uncovered_for_group)
    uncovered_for_group['verification'] = not_covered_assertions.map { |a| a['verification'] }
  end

  # delete specified keys from hash
  def except(hash, keys)
    hash.dup.tap do |x|
      keys.each { |key| x.delete(key) }
    end
  end
end
