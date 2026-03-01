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

def cmp_opcode(imm)
  if imm <= 255
    return "cmp r10, ##{imm}"
  end
  # We need to save imm to temporary register before cmp,
  # because it is too large for ARM instruction format
  return "mov r3, ##{imm}
    cmp r10, r3"
end

def jump_eq(target)
  return "beq #{target}"
end

def jump(target)
  return "b #{target}"
end

def save_param(value)
  return "mov r10, ##{value}"
end

def handler_path(name)
  return "bridge/arch/arm/handle_#{name}_armhf.S"
end
