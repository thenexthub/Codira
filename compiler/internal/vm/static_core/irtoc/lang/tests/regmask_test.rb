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

Options = Object.new
class << Options
  def arch
    :x86_64
  end
end

require_relative '../regmask'
require 'test/unit'

class RegMaskTest < Test::Unit::TestCase
  def setup
    # Do nothing
  end

  def teardown
    # Do nothing
  end

  def test_regmap
    r1 = Regmap.new({ arm64: {a: 1}, arm32: {a: 2}, x86_64: {a: 3} })
    r2 = Regmap.new({ arm64: {b: 4}, arm32: {b: 5}, x86_64: {b: 6} })

    r3 = r1 + r2
    assert_equal(r3, Regmap.new({ arm64: {a: 1, b: 4}, arm32: {a: 2, b: 5}, x86_64: {a: 3, b: 6} }))
    r4 = r3 - r1
    assert_equal(r4, r2)
    r5 = r3 + {c: 7}
    assert_equal(r5, {a: 3, b: 6, c: 7})
    r6 = r5 - {a: 2, c: 7}
    assert_equal(r6, {b: 6})

    # Added Regmap should overwrite keys with the same name
    r7 = r1 + {a: 11}
    assert_equal(r7, {a: 11})
  end

  def test_regmask
    assert_raise(RuntimeError) { RegMask.new('a') }
    assert_raise(RuntimeError) { RegMask.new(:a) }

    map = Regmap.from_hash({a: 1, b: 2, c: 3, d: 4, e: 5, f: 6})
    mask = RegMask.new(map, :a, :b, :c, :d, :e, :f)
    m1 = RegMask.new(map[:b], map[:d])
    m2 = RegMask.new(map, :b, :d)
    assert_equal(m1, m2)
    m3 = RegMask.new(map, :a, :c, :e)
    m4 = m1 + m3
    assert_equal(m4, RegMask.new(map, :a, :b, :c, :d, :e))
    assert_equal(m4 - m1, m3)
    assert_equal(m3 - m3, RegMask.new())
    assert_equal(RegMask.new(map, :a) + map[:b], RegMask.new(map, :a, :b))
    assert_equal(RegMask.new(map, :a, :b) - map[:b], RegMask.new(map, :a))

    # Symbols addition/substraction
    assert_equal(RegMask.new(map, :b, :d) + :a + :c + :e + :f, mask)
    assert_equal(mask - :c - :e - :f, RegMask.new(map, :a, :b, :d))

    # Regmap addition/substraction
    assert_equal(RegMask.new(map, :b, :d) + Regmap.from_hash({a: 1, e: 5}), RegMask.new(map, :a, :b, :d, :e))
    assert_equal(mask - Regmap.from_hash({a: 1, e: 5}), RegMask.new(map, :b, :c, :d, :f))
    assert_equal(mask - map, RegMask.new(map))

    # Symbol is not in the regmap
    assert_raise(RuntimeError) { RegMask.new(map, :aaa) }

    # Test `each` method and [] opertors
    m = RegMask.new(map, :b, :d)
    arr = []
    m.each {|x| arr << x}
    assert_equal(arr, [2, 4])
    m[1] = true
    assert_equal(m, RegMask.new(map, :a, :b, :d))
    assert_true(m[:a])
    assert_true(m[1])
    m[:c] = true
    assert_equal(m, RegMask.new(map, :a, :b, :c, :d))
    assert_true(m[:c])
    assert_true(m[:b])
    m[:b] = false
    assert_false(m[:b])
    assert_equal(m, RegMask.new(map, :a, :c, :d))
    assert_true(m[1])
    m[1] = false
    assert_false(m[1])
    assert_equal(m, RegMask.new(map, :c, :d))
  end
end