/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */


#ifndef RUNTIME_CONTEXT_OFFSET_H
#define RUNTIME_CONTEXT_OFFSET_H

#include "offset_macro.h"

#define CONTEXT_X18 0x0
#define CONTEXT_X19 0x8
#define CONTEXT_X20 0x10
#define CONTEXT_X21 0x18
#define CONTEXT_X22 0x20
#define CONTEXT_X23 0x28
#define CONTEXT_X24 0x30
#define CONTEXT_X25 0x38
#define CONTEXT_X26 0x40
#define CONTEXT_X27 0x48
#define CONTEXT_X28 0x50
#define CONTEXT_X29_FP 0x58
#define CONTEXT_X30_LR 0x60
#define CONTEXT_PC 0x68
#define CONTEXT_SP 0x70

#define CONTEXT_D8 0x78
#define CONTEXT_D9 0x80
#define CONTEXT_D10 0x88
#define CONTEXT_D11 0x90
#define CONTEXT_D12 0x98
#define CONTEXT_D13 0xa0
#define CONTEXT_D14 0xa8
#define CONTEXT_D15 0xb0

#define CONTEXT_FPCR 0xb8

#endif // RUNTIME_CONTEXT_OFFSET_H
