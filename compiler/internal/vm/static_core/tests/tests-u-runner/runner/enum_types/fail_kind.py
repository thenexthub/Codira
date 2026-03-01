#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
#

from enum import Enum


class FailKind(Enum):
    ES2PANDA_FAIL = 1
    ES2PANDA_TIMEOUT = 4
    ES2PANDA_OTHER = 7

    RUNTIME_FAIL = 2
    RUNTIME_TIMEOUT = 5
    RUNTIME_OTHER = 8

    AOT_FAIL = 3
    AOT_TIMEOUT = 6
    AOT_OTHER = 9

    QUICK_FAIL = 10
    QUICK_TIMEOUT = 11
    QUICK_OTHER = 12

    VERIFIER_FAIL = 13
    VERIFIER_TIMEOUT = 14
    VERIFIER_OTHER = 15

    COMPARE_FAIL = 16
    COMPARE_OUTPUT_FAIL = 17

    TS_NODE_FAIL = 20
    TS_NODE_TIMEOUT = 21
    TS_NODE_OTHER = 22

    NODE_FAIL = 26
    NODE_TIMEOUT = 27
    NODE_OTHER = 28

    SEGFAULT_FAIL = 29
    ABORT_FAIL = 30
    IRTOC_ASSERT_FAIL = 31
    ES2PANDA_NEG_FAIL = 32

    SRC_DUMPER_FAIL = 33
    INVALID_JSON = 34

    DECLGEN_ETS2TS_FAIL = 35
    DECLGEN_ETS2TS_TIMEOUT = 37
    DECLGEN_ETS2TS_OTHER = 38

    TSC_FAIL = 39
    TSC_TIMEOUT = 40
    TSC_OTHER = 41

    DECLGEN_TS2ETS_FAIL = 42
    DECLGEN_TS2ETS_TIMEOUT = 43
    DECLGEN_TS2ETS_OTHER = 44
    