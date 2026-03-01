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

require 'test/unit'
require_relative 'checker'

class CheckerTest < Test::Unit::TestCase
  def setup; end
  def teardown; end

  def test_disasm
    disasm = %(
    dummy
METHOD_INFO:
  name: _GLOBAL::main
  frame_size: 12
DISASSEMBLY:
    asm ---
    asm 000
  # IR inst 1
    asm 111
      asm 222
      # Tag
        asm 333
    # Tag
      asm 444
    asm 555
  # IR inst 2
    asm 666
METHOD_INFO:
  name: _GLOBAL::foo
  frame_size: 0
DISASSEMBLY:
  asm 777
)
    # Checker will clear working directory, so create it before we create disasm.txt file
    checker = $CheckerForTest.new(OpenStruct.new({arch: 'x64'}), "")

    File.write('disasm.txt', disasm)
    checker.ASM_METHOD(/main/)
    checker.ASM(x64: '---')
    checker.ASM(x64: '000')
    checker.ASM(x64: '333')
    checker.ASM(x64: '666')
    checker.ASM_NOT(x64: '777')
    checker.ASM_INST('IR inst 1')
    checker.ASM(x64: '111')
    checker.ASM_NOT(x64: '666')
    checker.ASM_NEXT(x64: '222')
    checker.ASM_NEXT_NOT(x64: '777')
    checker.ASM_NEXT(x64: '333')
    checker.ASM_NEXT(x64: '444')
    checker.ASM_NEXT(x64: '555')
    checker.ASM_NEXT_NOT(x64: '666')
    checker.ASM_INST('IR inst 2')
    checker.ASM_NEXT(x64: '666')
    checker.ASM_NEXT_NOT(x64: /.*/)
    checker.ASM_RESET
    checker.ASM(x64: '666')
    checker.ASM(x64: '---')
    checker.ASM(x64: '333')
    checker.ASM_METHOD('foo')
    checker.ASM(x64: '777')
    checker.ASM_NEXT_NOT(x64: /.*/)
  end
end
