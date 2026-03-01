/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "arch/asm_support.h"

.set ALIGNMENT_MASK, ~(TLAB_ALIGNMENT - 1)

// Load tlab start and tlab size from TLAB structure. Relies on fact that tlab start and tlab end are neighbours in TLAB structure.
.macro LOAD_TLAB_INFO reg_tlab_start, reg_tlab_size
  .if TLAB_CUR_FREE_POSITION_OFFSET - TLAB_MEMORY_END_ADDR_OFFSET != 8
    // we need it to be able to load pair of values with one instruction
    .err
  .endif

  ldr \reg_tlab_size, [THREAD_REG, #MANAGED_THREAD_TLAB_OFFSET]
  ldp \reg_tlab_size, \reg_tlab_start, [\reg_tlab_size, #TLAB_MEMORY_END_ADDR_OFFSET]
  sub \reg_tlab_size, \reg_tlab_size, \reg_tlab_start
.endm

// store class pointer, next free TLAB pointer and assign return register value with TLAB allocated space.
.macro FINISH_TLAB_ALLOCATION reg_class_pointer, reg_new_string, reg_next_free_tlab_pointer
  str \reg_class_pointer, [\reg_new_string, #OBJECT_HEADER_CLASS_POINTER_OFFSET]
  mov x0, \reg_new_string
  // Store next free TLAB pointer
  ldr \reg_new_string, [THREAD_REG, #MANAGED_THREAD_TLAB_OFFSET]
  add x20, \reg_new_string, #TLAB_CUR_FREE_POSITION_OFFSET
  stlr \reg_next_free_tlab_pointer, [x20]
.endm
