#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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

from __future__ import annotations
from dataclasses import dataclass
import typing

import cdp.util as util
import cdp.profiler as profiler


@dataclass
class ProfileArray:
    """
    Custom container for holding multiple profiling results.

    This is required for 1.2 profiler because:
    1. The 1.2 profiler may generate multiple profile segments that need to be
       processed together, while standard CDP typically works with single profiles.
    2. Provides structured handling of profile arrays which is more common in
       the 1.2 profiler's use cases.
    """

    profile: typing.List[profiler.Profile]

    @classmethod
    def from_json(cls, json: typing.List[util.T_JSON_DICT]) -> ProfileArray:
        profiles = [profiler.Profile.from_json(item) for item in json]
        return cls(profiles)


def profiler_stop() -> typing.Generator[util.T_JSON_DICT, util.T_JSON_DICT, ProfileArray]:
    """
    Custom implementation of profiler stop command.

    This differs from standard CDP implementation because:
    1. Maintains compatibility with CDP protocol while extending it for 1.2
    profiler's specific needs.
    2. Returns a ProfileArray instead of a single Profile object to accommodate
    the 1.2 profiler's capability of returning multiple profile segments.
    """
    cmd_dict: util.T_JSON_DICT = {
        "method": "Profiler.stop",
    }
    json = yield cmd_dict
    return ProfileArray.from_json(json["profile"])
