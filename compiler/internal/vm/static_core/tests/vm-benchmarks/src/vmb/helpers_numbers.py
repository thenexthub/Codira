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

import math


def __degre(val: float) -> int:
    return int(math.floor(math.log10(abs(val)) / 3.0) * 3)


def __suffix(level: int) -> str:
    return {
        15: 'P',
        12: 'T',
        9: 'G',
        6: 'M',
        3: 'K',
        0: '',
        -3: 'm',
        -6: '\N{MICRO SIGN}',
        -9: 'n',
        -12: 'p'
    }.get(level, '')


def nano_str(val: float) -> str:
    return f'{int(round(val * (10 ** 9))):>11}n'


def auto_str(val: float) -> str:
    if 0 == val:
        return f'{0:>8}'
    d = __degre(val)
    s = __suffix(d)
    if not s:
        return f'{int(round(val)):>8}'
    return f'{int(round(val / (10 ** d))):>7}{s}'


def format_number(val: float, fmt: str = 'expo') -> str:
    if 'nano' == fmt:
        return nano_str(val)
    if 'auto' == fmt:
        return auto_str(val)
    return f'{val:.2e}'
